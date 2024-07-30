/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_invoke.cpp
  @brief CompanEdge - CLI Invoke command
*/

#include "company_ref_cli_invoke.h"
#include <Compan_logger/Compan_logger.h>

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_utils/company_ref_process_monitor_factory.h>

#include <company_ref_protocol_utils/company_ref_stream.h>

using namespace Compan::Edge;

namespace Compan{
namespace Edge {
CompanLogger CompanEdgeInvokeLog("cli.invoke", LogLevel::Information);
} // namespace Edge
} // namespace Compan

CompanEdgeCliInvoke::CompanEdgeCliInvoke(
        boost::asio::io_context& ioCtx,
        std::string const& id,
        std::string const& exec,
        const bool onChangedOnly,
        ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , id_(id)
    , execBinary_(exec)
    , ctx_(ioCtx)
    , onChangedOnly_(onChangedOnly)
    , processMonitorPtr_()
{
    InfoLog(CompanEdgeInvokeLog) << "Adding invoke slot for id: " << id_ << ". Will execute binary: " << execBinary_
                               << std::endl;
}

CompanEdgeCliInvoke::~CompanEdgeCliInvoke() = default;

bool CompanEdgeCliInvoke::process(CompanEdgeProtocol::ServerMessage& msg)
{
    auto subscribeMsg = msg.mutable_vssubscribe();
    setSequenceNo(*subscribeMsg);
    subscribeMsg->add_ids(id_);

    return true;
}

CommandStatus CompanEdgeCliInvoke::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    // initial request is sent back as vsresult
    if (msg.has_vsresult()) {
        auto& vsResult = msg.vsresult();
        if (vsResult.status() != CompanEdgeProtocol::VsResult::success) {
            DebugLog(CompanEdgeInvokeLog) << CompanEdgeProtocol::VsResult::Status_Name(vsResult.status()) << std::endl;
            return CMD_STATUS_FAIL;
        }

        // ignore processing values from vsResult
        if (!onChangedOnly_) {
            for (auto const& val : vsResult.values()) {
                if (isInvokableType(val.type())) { valueChangedqueue_.push(val); }
            }
            boost::asio::post(ctx_, std::bind(&CompanEdgeCliInvoke::processQueue, this));
        }
    } else if (msg.has_valuechanged()) {
        for (int i = 0; i < msg.valuechanged().value_size(); ++i) {
            if (isInvokableType(msg.valuechanged().value(i).type())) {
                valueChangedqueue_.push(msg.valuechanged().value(i));
            }
        }
        boost::asio::post(ctx_, std::bind(&CompanEdgeCliInvoke::processQueue, this));
    }

    return CMD_STATUS_IN_PROGRESS;
}

void CompanEdgeCliInvoke::processFinished(CompanEdgeProcessMonitorBase* processMonitor)
{
    if (processMonitor) {
        InfoLog(CompanEdgeInvokeLog) << "Is process still running: " << (processMonitor->isRunning() ? " yes" : " no")
                                   << std::endl;
        InfoLog(CompanEdgeInvokeLog) << "Process return code: " << processMonitor->returnCode() << std::endl;
    }

    boost::asio::post(ctx_, std::bind(&CompanEdgeCliInvoke::processQueue, this));
}

void CompanEdgeCliInvoke::processQueue()
{
    if (processMonitorPtr_ && processMonitorPtr_->isRunning()) {
        InfoLog(CompanEdgeInvokeLog) << "Process currently running" << std::endl;
        return;
    }

    if (valueChangedqueue_.empty()) {
        InfoLog(CompanEdgeInvokeLog) << "Value changed queue empty, waiting for Value changes..." << std::endl;
        return;
    }

    InfoLog(CompanEdgeInvokeLog) << "Processing Value changed queue, size: " << valueChangedqueue_.size() << std::endl;

    auto& value = valueChangedqueue_.front();

    InfoLog(CompanEdgeInvokeLog) << "Invoking for value: " << value << std::endl;

    std::vector<std::string> params;
    params.push_back(value.id().c_str());
    params.push_back(to_string(value));

    valueChangedqueue_.pop();

    processMonitorPtr_ = CompanEdgeProcessMonitoryFactory::make(
            ctx_, execBinary_, params, std::bind(&CompanEdgeCliInvoke::processFinished, this, std::placeholders::_1));

    processMonitorPtr_->start();
}

bool CompanEdgeCliInvoke::isInvokableType(CompanEdgeProtocol::Value_Type const valueType)
{
    switch (valueType) {
    case CompanEdgeProtocol::Unset:
    case CompanEdgeProtocol::Unknown:
    case CompanEdgeProtocol::Struct:
    case CompanEdgeProtocol::Container: return false; break;
    default: return true; break;
    }
    return true;
}