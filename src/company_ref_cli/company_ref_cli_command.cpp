/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_command.cpp
  @brief CompanEdge - CLI command
*/

#include "company_ref_cli_command.h"
#include "company_ref_value_io_helper.h"

#include <company_ref_boost_client/company_ref_boost_client.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <Compan_logger/Compan_logger.h>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>

using namespace Compan::Edge;

namespace Compan{
namespace Edge {
CompanLogger CompanEdgeCliCommandLog("cli.command", LogLevel::Information);
} // namespace Edge
} // namespace Compan

unsigned int CompanEdgeCliCommand::sequenceNo_ = 0;

CompanEdgeCliCommand::CompanEdgeCliCommand(ProtocolValueMap& savedValues)
    : queueDelayMs_(300)
    , savedValues_(savedValues)
    , expectNoResponseMsg_(false)
    , format_(FORMAT_Compan)
{
}

CompanEdgeCliCommand::~CompanEdgeCliCommand() = default;

void CompanEdgeCliCommand::setPrinter(std::shared_ptr<CompanEdgeCliOutput> cliOutput)
{
    cliOutput_ = cliOutput;
}

void CompanEdgeCliCommand::setFormatOption(FormatOption format)
{
    format_ = format;
    cliOutput_->setFormatOption(format_);
}

void CompanEdgeCliCommand::setExpectNoResponseMessage(const bool expectNoResponseMsg)
{
    expectNoResponseMsg_ = expectNoResponseMsg;
}

bool CompanEdgeCliCommand::expectNoResponseMsg()
{
    return expectNoResponseMsg_;
}

FormatOption CompanEdgeCliCommand::getFormatOption()
{
    return format_;
}

std::string CompanEdgeCliCommand::convert2SetValue(
        const ProtocolValueMap& savedValues,
        const std::string& valueId,
        const std::string& currentValue)
{
    if (FORMAT_TR069 == getFormatOption()) {
        CompanEdgeProtocol::Value_Type type(CompanEdgeProtocol::Unset);
        auto it = savedValues.find(valueId);
        if (it != savedValues.end()) { type = it->second.type(); }

        switch (type) {
        case CompanEdgeProtocol::TimeVal: {
            // convert input set string to CompanTimeVal format: "second:microsecond"
            timeval tv = ValueIOHelper::timeFromIsoExtendedStrToTimeVal(currentValue);
            return ValueIOHelper::toValueInputStr(tv);
        } break;
        case CompanEdgeProtocol::TimeSpec: {
            // convert input set string to CompanTimeSpec format: "second:nanosecond"
            timespec ts = ValueIOHelper::timeFromIsoExtendedStrToTimeSpec(currentValue);
            return ValueIOHelper::toValueInputStr(ts);
        } break;
        default: break;
        }
    }
    return currentValue;
}

Get::Get(std::string const& id, ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , id_(id)
{
}

Get::~Get()
{
}

bool Get::process(CompanEdgeProtocol::ServerMessage& msg)
{
    CompanEdgeProtocol::VsGetValue* vsGetValueMsg = msg.mutable_vsgetvalue();
    setSequenceNo(*vsGetValueMsg);
    vsGetValueMsg->set_id(id_);
    return true;
}

CommandStatus Get::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    if (msg.has_vsresult()) {
        auto& vsResult = msg.vsresult();
        if (vsResult.status() != CompanEdgeProtocol::VsResult::success) {
            cliOutput_->printError(CompanEdgeProtocol::VsResult::Status_Name(vsResult.status()));
            return CMD_STATUS_FAIL;
        }

        if (msg.vsresult().sequenceno() != sequenceNo()) {
            cliOutput_->printError("Msg sequence number mismatch");
            return CMD_STATUS_FAIL;
        }

        auto it = savedValues_.find(Compan_SYSTEM_CONFIG_TIMEZONE);
        if (it != savedValues_.end()) { cliOutput_->setAttribute(PRINT_TIMEZONE, to_string(it->second)); }

        // cliOutput_->setAttribute(PRINT_VALUES_DELIMITER, std::string());
        cliOutput_->setAttribute(PRINT_VALUE_CATEGORY_SHOWN, "false");
        cliOutput_->setAttribute(PRINT_VALUEID_VALUE_SEPARATOR, "|");
        cliOutput_->setAttribute(PRINT_VALUE_ONLY, "true");

        cliOutput_->printValue(vsResult);
        return CMD_STATUS_SUCCESS;
    }
    return CMD_STATUS_FAIL;
}

GetValues::GetValues(std::vector<std::pair<std::string, std::string>> const& ids, ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , ids_(ids)
{
}

GetValues::~GetValues()
{
}

bool GetValues::process(CompanEdgeProtocol::ServerMessage& msg)
{
    CompanEdgeProtocol::VsMultiGet* vsMultiGetMsg = msg.mutable_vsmultiget();
    setSequenceNo(*vsMultiGetMsg);
    for (auto& id : ids_) { vsMultiGetMsg->add_ids(id.first); }
    return true;
}

CommandStatus GetValues::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    if (msg.has_vsmultigetresult()) {
        auto& vsResult = msg.vsmultigetresult();
        if (vsResult.sequenceno() != sequenceNo()) {
            cliOutput_->printError("Msg sequence number mismatch");
            return CMD_STATUS_FAIL;
        }

        for (auto& res : vsResult.results()) {
            if (res.error() != CompanEdgeProtocol::VsMultiGetResult::Success) {
                cliOutput_->printError(res.description());
                continue;
            }

            for (auto& val : res.values()) {
                CompanEdgeProtocol::Value value(val);
                auto const result = savedValues_.insert(
                        std::make_pair<std::string, CompanEdgeProtocol::Value>(val.id().c_str(), std::move(value)));
                if (!result.second) { result.first->second = val; }
            }
        }
        return CMD_STATUS_SUCCESS;
    }
    return CMD_STATUS_FAIL;
}

GetAll::GetAll(ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
{
}

GetAll::~GetAll()
{
}

bool GetAll::process(CompanEdgeProtocol::ServerMessage& serverMessage)
{
    CompanEdgeProtocol::VsGetAll* vsGetAllMsg = serverMessage.mutable_vsgetall();
    setSequenceNo(*vsGetAllMsg);
    return true;
}

CommandStatus GetAll::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    if (msg.has_vsresult()) {
        auto& vsResult = msg.vsresult();
        if (vsResult.status() != CompanEdgeProtocol::VsResult::success) {
            cliOutput_->printError(CompanEdgeProtocol::VsResult::Status_Name(vsResult.status()));
            return CMD_STATUS_FAIL;
        }

        if (vsResult.sequenceno() != sequenceNo()) {
            cliOutput_->printError("Msg sequence number mismatch");
            return CMD_STATUS_FAIL;
        }

        auto it = savedValues_.find(Compan_SYSTEM_CONFIG_TIMEZONE);
        if (it != savedValues_.end()) { cliOutput_->setAttribute(PRINT_TIMEZONE, to_string(it->second)); }

        // cliOutput_->setAttribute(PRINT_VALUES_DELIMITER, std::string());
        cliOutput_->setAttribute(PRINT_VALUE_CATEGORY_SHOWN, "false");
        cliOutput_->setAttribute(PRINT_VALUEID_VALUE_SEPARATOR, "|");
        cliOutput_->setAttribute(PRINT_VALUE_ONLY, "true");

        cliOutput_->print(msg);
        return CMD_STATUS_SUCCESS;
    }
    return CMD_STATUS_FAIL;
}

GetObject::GetObject(const std::string& id, ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , id_(id)
{
}

GetObject::~GetObject()
{
}

bool GetObject::process(CompanEdgeProtocol::ServerMessage& serverMessage)
{
    CompanEdgeProtocol::VsGetObject* vsGetObjectMsg = serverMessage.mutable_vsgetobject();
    setSequenceNo(*vsGetObjectMsg);
    vsGetObjectMsg->set_id(id_);
    return true;
}

CommandStatus GetObject::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    if (msg.has_vsresult()) {
        auto& vsResult = msg.vsresult();
        if (vsResult.status() != CompanEdgeProtocol::VsResult::success) {
            cliOutput_->printError(CompanEdgeProtocol::VsResult::Status_Name(vsResult.status()));
            return CMD_STATUS_FAIL;
        }

        if (vsResult.sequenceno() != sequenceNo()) {
            cliOutput_->printError("Msg sequence number mismatch");
            return CMD_STATUS_FAIL;
        }

        auto it = savedValues_.find(Compan_SYSTEM_CONFIG_TIMEZONE);
        if (it != savedValues_.end()) { cliOutput_->setAttribute(PRINT_TIMEZONE, to_string(it->second)); }

        cliOutput_->setAttribute(PRINT_VALUE_CATEGORY_SHOWN, "false");
        cliOutput_->setAttribute(PRINT_VALUEID_VALUE_SEPARATOR, "|");
        cliOutput_->setAttribute(PRINT_VALUE_ONLY, "true");

        cliOutput_->print(msg);
        return CMD_STATUS_SUCCESS;
    }
    return CMD_STATUS_FAIL;
}

Set::Set(std::string const& id, std::string const& value, ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , id_(id)
    , value_(value)
{
    DebugLog(CompanEdgeCliCommandLog) << "Input valueId:" << id_ << ", valueStr:" << value_
                                    << ", size:" << savedValues.size() << std::endl;
}

Set::~Set()
{
}

bool Set::process(CompanEdgeProtocol::ServerMessage& msg)
{
    CompanEdgeProtocol::VsSetValue* vsSetValueMsg = msg.mutable_vssetvalue();
    setSequenceNo(*vsSetValueMsg);
    vsSetValueMsg->set_id(id_);
    vsSetValueMsg->set_value(convert2SetValue(savedValues_, id_, value_));
    return true;
}

CommandStatus Set::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    if (msg.has_vsresult()) {
        auto& vsResult = msg.vsresult();
        if (vsResult.status() != CompanEdgeProtocol::VsResult::success) {
            cliOutput_->printError(CompanEdgeProtocol::VsResult::Status_Name(vsResult.status()));
            return CMD_STATUS_FAIL;
        }

        if (msg.vsresult().sequenceno() != sequenceNo()) {
            cliOutput_->printError("Msg sequence number mismatch");
            return CMD_STATUS_FAIL;
        }

        auto it = savedValues_.find(Compan_SYSTEM_CONFIG_TIMEZONE);
        if (it != savedValues_.end()) { cliOutput_->setAttribute(PRINT_TIMEZONE, to_string(it->second)); }
        cliOutput_->setAttribute(PRINT_VALUE_CATEGORY_SHOWN, "false");
        cliOutput_->setAttribute(PRINT_VALUEID_VALUE_SEPARATOR, "|");
        cliOutput_->setAttribute(PRINT_VALUE_ONLY, "true");

        cliOutput_->print(vsResult);
        return CMD_STATUS_SUCCESS;
    }
    return CMD_STATUS_FAIL;
}

EnableMonitor::EnableMonitor(ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
{
}

EnableMonitor::~EnableMonitor()
{
}

bool EnableMonitor::process(CompanEdgeProtocol::ServerMessage& msg)
{
    msg.mutable_vssync();
    return true;
}

CommandStatus EnableMonitor::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    auto it = savedValues_.find(Compan_SYSTEM_CONFIG_TIMEZONE);
    if (it != savedValues_.end()) { cliOutput_->setAttribute(PRINT_TIMEZONE, to_string(it->second)); }

    cliOutput_->setAttribute(PRINT_VALUE_CATEGORY_SHOWN, "false");
    cliOutput_->setAttribute(PRINT_VALUEID_VALUE_SEPARATOR, "|");
    cliOutput_->setAttribute(PRINT_VALUE_ONLY, "true");

    cliOutput_->print(msg);
    return CMD_STATUS_IN_PROGRESS;
}

AddToContainer::AddToContainer(std::string const& containerId, std::string const& keyId, ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , containerId_(containerId)
    , keyId_(keyId)
{
    setExpectNoResponseMessage(true);
}

AddToContainer::~AddToContainer()
{
}

bool AddToContainer::process(CompanEdgeProtocol::ServerMessage& msg)
{
    DebugLog(CompanEdgeCliCommandLog) << "Creating server valuechanged AddToContainer" << std::endl;

    CompanEdgeProtocol::Value* val(msg.mutable_valuechanged()->add_value());
    val->set_type(CompanEdgeProtocol::Value_Type::ContainerAddTo);
    val->set_id(containerId_);
    val->mutable_addtocontainer()->set_key(keyId_);

    return true;
}

CommandStatus AddToContainer::process(CompanEdgeProtocol::ClientMessage const&)
{
    return CMD_STATUS_SUCCESS;
}

RemoveFromContainer::RemoveFromContainer(
        std::string const& containerId,
        std::string const& keyId,
        ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , containerId_(containerId)
    , keyId_(keyId)
{
    setExpectNoResponseMessage(true);
}

RemoveFromContainer::~RemoveFromContainer()
{
}

bool RemoveFromContainer::process(CompanEdgeProtocol::ServerMessage& msg)
{
    DebugLog(CompanEdgeCliCommandLog) << "Creating server valuechanged RemoveFromContainer" << std::endl;

    CompanEdgeProtocol::Value* val(msg.mutable_valuechanged()->add_value());
    val->set_type(CompanEdgeProtocol::Value_Type::ContainerRemoveFrom);
    val->set_id(containerId_);
    val->mutable_removefromcontainer()->set_key(keyId_);
    return true;
}

CommandStatus RemoveFromContainer::process(CompanEdgeProtocol::ClientMessage const&)
{
    return CMD_STATUS_SUCCESS;
}

MultiGet::MultiGet(std::vector<std::pair<std::string, std::string>> const& ids, ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , ids_(ids)
{
    DebugLog(CompanEdgeCliCommandLog) << "Cmd size:" << ids_.size() << std::endl;
    uint32_t count(0);
    for (auto& id : ids) { DebugLog(CompanEdgeCliCommandLog) << "cmd " << ++count << " id:" << id.first << std::endl; }
}

MultiGet::~MultiGet()
{
}

bool MultiGet::process(CompanEdgeProtocol::ServerMessage& msg)
{
    CompanEdgeProtocol::VsMultiGet* vsMultiGetMsg = msg.mutable_vsmultiget();
    setSequenceNo(*vsMultiGetMsg);
    for (auto& id : ids_) { vsMultiGetMsg->add_ids(id.first); }
    return true;
}

CommandStatus MultiGet::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    if (msg.has_vsmultigetresult()) {
        auto& vsResult = msg.vsmultigetresult();
        if (vsResult.sequenceno() != sequenceNo()) {
            cliOutput_->printError("Msg sequence number mismatch");
            return CMD_STATUS_FAIL;
        }

        auto it = savedValues_.find(Compan_SYSTEM_CONFIG_TIMEZONE);
        if (it != savedValues_.end()) { cliOutput_->setAttribute(PRINT_TIMEZONE, to_string(it->second)); }

        cliOutput_->setAttribute(PRINT_VALUE_CATEGORY_SHOWN, "false");
        cliOutput_->setAttribute(PRINT_VALUEID_VALUE_SEPARATOR, "|");
        cliOutput_->setAttribute(PRINT_VALUE_ONLY, "true");

        cliOutput_->print(vsResult);
        return CMD_STATUS_SUCCESS;
    }
    return CMD_STATUS_FAIL;
}

MultiSet::MultiSet(std::vector<std::pair<std::string, std::string>> const& ids, ProtocolValueMap& savedValues)
    : CompanEdgeCliCommand(savedValues)
    , ids_(ids)
{
    DebugLog(CompanEdgeCliCommandLog) << "Cmd size:" << ids_.size() << ", cache size:" << savedValues.size() << std::endl;
    uint32_t count(0);
    for (auto& id : ids) {
        DebugLog(CompanEdgeCliCommandLog) << "cmd " << ++count << " id:" << id.first << ":" << id.second << std::endl;
    }
}

MultiSet::~MultiSet()
{
}

bool MultiSet::process(CompanEdgeProtocol::ServerMessage& msg)
{
    CompanEdgeProtocol::VsMultiSet* vsMultiSetMsg = msg.mutable_vsmultiset();
    setSequenceNo(*vsMultiSetMsg);

    for (auto& id : ids_) {
        CompanEdgeProtocol::VsMultiSet::Value* val(vsMultiSetMsg->add_values());
        val->set_id(id.first);
        val->set_value(convert2SetValue(savedValues_, id.first, id.second));
    }
    return true;
}

CommandStatus MultiSet::process(CompanEdgeProtocol::ClientMessage const& msg)
{
    if (msg.has_vsmultisetresult()) {
        auto& vsResult = msg.vsmultisetresult();
        if (vsResult.sequenceno() != sequenceNo()) {
            cliOutput_->printError("Msg sequence number mismatch");
            return CMD_STATUS_FAIL;
        }

        auto it = savedValues_.find(Compan_SYSTEM_CONFIG_TIMEZONE);
        if (it != savedValues_.end()) { cliOutput_->setAttribute(PRINT_TIMEZONE, to_string(it->second)); }

        cliOutput_->setAttribute(PRINT_VALUE_CATEGORY_SHOWN, "false");
        cliOutput_->setAttribute(PRINT_VALUEID_VALUE_SEPARATOR, "|");
        cliOutput_->setAttribute(PRINT_VALUE_ONLY, "true");

        cliOutput_->print(vsResult);
        return CMD_STATUS_SUCCESS;
    }
    return CMD_STATUS_FAIL;
}
