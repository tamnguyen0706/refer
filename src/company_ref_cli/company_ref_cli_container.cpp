/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_container.cpp
  @brief CompanEdge - CLI AddTo RemoveFrom Container command
*/

#include "company_ref_cli_container.h"
#include <Compan_logger/Compan_logger.h>

#include <company_ref_protocol/company_ref_protocol.pb.h>

#include <company_ref_protocol_utils/company_ref_stream.h>

using namespace Compan::Edge;

namespace Compan{
namespace Edge {
CompanLogger CompanEdgeContainerLog("cli.container", LogLevel::Information);
} // namespace Edge
} // namespace Compan

CompanEdgeCliContainer::CompanEdgeCliContainer(
        boost::asio::io_context& ioCtx,
        std::string const& id,
        std::string const& containerKey,
        const bool condition, // addToContainer:true, removeFromContainer:false
        ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , id_(id)
    , containerKey_(containerKey)
    , ctx_(ioCtx)
    , condition_(condition)
    , fqvid_(id_ + "." + containerKey_)
{
    DebugLog(CompanEdgeContainerLog) << "Listening container slot for id: " << id_
                                   << (condition_ ? ", add: " : ", remove: ") << containerKey_ << std::endl;
}

CompanEdgeCliContainer::~CompanEdgeCliContainer() = default;

bool CompanEdgeCliContainer::process(CompanEdgeProtocol::ServerMessage& msg)
{
    auto subscribeMsg = msg.mutable_vssubscribe();
    setSequenceNo(*subscribeMsg);
    subscribeMsg->add_ids(id_);

    CompanEdgeProtocol::Value* val(msg.mutable_valuechanged()->add_value());
    val->set_id(id_);
    if (condition_) {
        val->set_type(CompanEdgeProtocol::Value_Type::ContainerAddTo);
        val->mutable_addtocontainer()->set_key(containerKey_);
    } else {
        val->set_type(CompanEdgeProtocol::Value_Type::ContainerRemoveFrom);
        val->mutable_removefromcontainer()->set_key(containerKey_);
    }
    return true;
}

CommandStatus CompanEdgeCliContainer::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    // Subscription response by vsresult
    if (msg.has_vsresult()) {
        auto& vsResult = msg.vsresult();
        if (vsResult.status() != CompanEdgeProtocol::VsResult::success) {
            DebugLog(CompanEdgeContainerLog) << CompanEdgeProtocol::VsResult::Status_Name(vsResult.status()) << std::endl;
            return CMD_STATUS_FAIL;
        }
        // verify the subscription value id is a container type
        for (auto& value : vsResult.values()) {
            DebugLog(CompanEdgeContainerLog) << "Value: " << value << ", id_: " << id_ << std::endl;
            if (value.id() == id_) {
                if (value.type() != CompanEdgeProtocol::Value_Type::Container) { return CMD_STATUS_FAIL; }
                break;
            }
        }
    }
    // listening for on value changed or removed
    else if (msg.has_valuechanged()) {
        size_t count(0);
        for (auto& value : msg.valuechanged().value()) {
            DebugLog(CompanEdgeContainerLog) << ++count << " Receiving add: " << value << std::endl;
            if (CompanEdgeProtocol::Value_Type::ContainerAddTo == value.type()
                && containerKey_ == value.addtocontainer().key()) {
                return CMD_STATUS_SUCCESS;
            }
        }
    } else if (msg.has_valueremoved()) {
        size_t count(0);
        for (auto& id : msg.valueremoved().id()) {
            DebugLog(CompanEdgeContainerLog) << ++count << " Receiving remove: " << id << std::endl;
            if (fqvid_ == id) { return CMD_STATUS_SUCCESS; }
        }
    }

    return CMD_STATUS_IN_PROGRESS;
}
