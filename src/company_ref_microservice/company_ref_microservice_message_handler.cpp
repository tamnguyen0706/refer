/**
 Copyright © 2023 COMPAN REF
 @file company_ref_microservice_message_handler.cpp
 @brief Micro service Message Handler
 */
#include "company_ref_microservice_message_handler.h"

#include <company_ref_variant_valuestore/company_ref_variant_factory.h>

#include <company_ref_protocol_utils/company_ref_stream.h>
#include <Compan_logger/Compan_logger.h>

#include <sstream>

namespace Compan{
namespace Edge {
CompanLogger MicroServiceMessageHandlerLog("MsClient.MessageHandler", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

MicroServiceMessageHandler::MicroServiceMessageHandler(
        VariantValueStore& ws,
        std::string const& appName,
        uint32_t& seqNo)
    : ClientProtocolHandler(0)
    , ws_(ws)
    , appName_(appName)
    , seqNo_(seqNo)
    , addedListener_(ws_.connectValueAddedListener(
              std::bind(&MicroServiceMessageHandler::handleValueChanged, this, std::placeholders::_1)))
    , changedListener_(ws_.connectValueChangedListener(
              std::bind(&MicroServiceMessageHandler::handleValueChanged, this, std::placeholders::_1)))
    , removedListener_(ws_.connectValueRemovedListener(
              std::bind(&MicroServiceMessageHandler::handleValueRemoved, this, std::placeholders::_1)))
    , addToContainerListener_(ws_.connectValueAddToContainerListener(
              std::bind(&MicroServiceMessageHandler::handleValueChanged, this, std::placeholders::_1)))
    , removeFromContainerListener_(ws_.connectValueRemoveFromContainerListener(
              std::bind(&MicroServiceMessageHandler::handleValueChanged, this, std::placeholders::_1)))
{
    FunctionLog(MicroServiceMessageHandlerLog);
}

MicroServiceMessageHandler::~MicroServiceMessageHandler()
{
    FunctionLog(MicroServiceMessageHandlerLog);
}

void MicroServiceMessageHandler::disconnect()
{
    subscribeSignal_.disconnectAll();

    addedListener_.disconnect();
    changedListener_.disconnect();
    removedListener_.disconnect();

    addToContainerListener_.disconnect();
    removeFromContainerListener_.disconnect();
}

void MicroServiceMessageHandler::doMessage(CompanEdgeProtocol::ClientMessage const& responseMessage)
{
    FunctionArgLog(MicroServiceMessageHandlerLog) << std::endl;

    if (responseMessage.has_valuechanged()) onMessage(responseMessage.valuechanged());
    if (responseMessage.has_valueremoved()) onMessage(responseMessage.valueremoved());
    if (responseMessage.has_vssynccompleted()) onMessage(responseMessage.vssynccompleted());
    if (responseMessage.has_vsresult()) onMessage(responseMessage.vsresult());
}

void MicroServiceMessageHandler::onMessage(CompanEdgeProtocol::ValueChanged const& valueChanged)
{
    for (auto& value : valueChanged.value()) {
        if (valueUpdate(value)) continue;
    }
}

bool MicroServiceMessageHandler::valueUpdate(CompanEdgeProtocol::Value const& value)
{
    FunctionArgLog(MicroServiceMessageHandlerLog) << appName_ << "(" << value << ")" << std::endl;

    // Container add/remove needs to fake it through the microservice for code
    //  that depends on that notification
    if (value.has_addtocontainer()) {

        ws_.addToContainer(
                value.id(), value.addtocontainer().key(), value.addtocontainer().value(), VariantValue::Remote);
        return true;
    } else if (value.has_removefromcontainer()) {

        ws_.removeFromContainer(value.id(), value.removefromcontainer().key(), VariantValue::Remote);
        return true;
    }

    if (isAppNameFiltered(value.id())) return false;

    VariantValue::Ptr valuePtr = ws_.get(value.id());
    if (valuePtr == nullptr) { return ws_.set(value, VariantValue::Remote); }

    // container and struct updates don't really count when they're coming across the wire as an update
    //  Updates to the elements will all be dispatched locally
    if (valuePtr->type() == CompanEdgeProtocol::Container || valuePtr->type() == CompanEdgeProtocol::Struct) return true;

    ValueDataSet::Results setResult = valuePtr->set(value, VariantValue::Remote);
    if (setResult == ValueDataSet::Success || setResult == ValueDataSet::SameValue) { return true; }

    // notify back to the client what the value really is
    DebugLog(MicroServiceMessageHandlerLog) << "Variant ValueStore set failed on valueId: " << value.id() << " - "
                                            << ValueDataSet::resultStr(setResult) << std::endl;

    return false;
}

void MicroServiceMessageHandler::onMessage(CompanEdgeProtocol::ValueRemoved const& valueRemoved)
{
    FunctionArgLog(MicroServiceMessageHandlerLog) << std::endl;

    bool removed(false);
    for (auto& valueId : valueRemoved.id()) {

        // del value at Ws
        removed = ws_.del(valueId);
        if (!removed) {

            DebugLog(MicroServiceMessageHandlerLog)
                    << "Variant ValueStore removal failed on valueId:" << valueId << std::endl;
            continue;
        }
    }
}

void MicroServiceMessageHandler::onMessage(CompanEdgeProtocol::VsResult const& vsResult)
{
    if (vsResult.status() == CompanEdgeProtocol::VsResult::success) {
        for (auto& value : vsResult.values()) { valueUpdate(value); }
    } else if (vsResult.status() == CompanEdgeProtocol::VsResult::error_not_found) {
        for (auto& value : vsResult.values()) { ws_.del(value.id()); }
    }

    DebugLog(MicroServiceMessageHandlerLog) << __FUNCTION__ << ": " << vsResult << std::endl;

    subscribeSignal_(vsResult);
}

void MicroServiceMessageHandler::onMessage(CompanEdgeProtocol::VsSyncCompleted const&)
{
}

void MicroServiceMessageHandler::handleValueAdded(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(MicroServiceMessageHandlerLog) << " valueId: " << valuePtr->id() << std::endl;

    if (valuePtr->type() == CompanEdgeProtocol::Unknown) return;

    // We don't need to bounce back a value changed if it came from the remote side
    if (valuePtr->setUpdateType() == VariantValue::Remote) return;

    ServerMessagePtr requestMessage(std::make_shared<CompanEdgeProtocol::ServerMessage>());

    CompanEdgeProtocol::VsSubscribe* vsSubscribe = requestMessage->mutable_vssubscribe();
    vsSubscribe->set_sequenceno(++seqNo_);
    vsSubscribe->add_ids(valuePtr->id());

    DebugLog(MicroServiceMessageHandlerLog) << __FUNCTION__ << ": " << *vsSubscribe << std::endl;

    onSendCallback_(std::move(requestMessage));
}

void MicroServiceMessageHandler::handleValueChanged(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(MicroServiceMessageHandlerLog) << " valueId: " << valuePtr->id() << std::endl;

    if (valuePtr->type() == CompanEdgeProtocol::Unknown) return;

    // We don't need to bounce back a value changed if it came from the remote side
    if (valuePtr->setUpdateType() == VariantValue::Remote) return;

    ServerMessagePtr requestMessage(std::make_shared<CompanEdgeProtocol::ServerMessage>());
    CompanEdgeProtocol::ValueChanged* valueChanged = requestMessage->mutable_valuechanged();
    *valueChanged->add_value() = valuePtr->get();

    onSendCallback_(std::move(requestMessage));
}

void MicroServiceMessageHandler::handleValueRemoved(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(MicroServiceMessageHandlerLog) << " valueId: " << valuePtr->id() << std::endl;

    ServerMessagePtr requestMessage(std::make_shared<CompanEdgeProtocol::ServerMessage>());
    CompanEdgeProtocol::ValueRemoved* valueRemoved = requestMessage->mutable_valueremoved();
    valueRemoved->add_id(valuePtr->id());
    valueRemoved->add_hashtoken(valuePtr->hashToken());

    onSendCallback_(std::move(requestMessage));
}

void MicroServiceMessageHandler::write(CompanEdgeProtocol::ServerMessage const& reqMsg)
{
    FunctionArgLog(MicroServiceMessageHandlerLog) << reqMsg << std::endl;
    ServerMessagePtr requestMessage(std::make_shared<CompanEdgeProtocol::ServerMessage>(reqMsg));

    onSendCallback_(std::move(requestMessage));
}

void MicroServiceMessageHandler::insertAppNameFilter(ValueId const& valueId)
{
    VariantValue::Ptr valuePtr = ws_.get(valueId);
    if (valuePtr == nullptr) return;

    parentFilter_.insert(valuePtr->hashToken());
}

void MicroServiceMessageHandler::removeAppNameFilter(ValueId const& valueId)
{
    VariantValue::Ptr valuePtr = ws_.get(valueId);
    if (valuePtr == nullptr) return;

    parentFilter_.remove(valuePtr->hashToken());
}

bool MicroServiceMessageHandler::isAppNameMatch(ValueId const& valueId)
{
    ValueId parentId = valueId.parent();
    if (parentId.empty()) parentId = ValueId(valueId);

    parentId += appName_;

    if (parentId == valueId) return true;

    return false;
}

bool MicroServiceMessageHandler::isAppNameFiltered(ValueId const& valueId)
{
    FunctionArgLog(MicroServiceMessageHandlerLog) << ": (" << appName_ << ")" << valueId << std::endl;

    if (parentFilter_.empty()) return false;

    VariantValue::Ptr valuePtr = ws_.get(valueId);
    if (valuePtr == nullptr) {

        ValueId parentId(valueId.parent());
        while (!parentId.empty()) {
            valuePtr = ws_.get(parentId);
            if (valuePtr) break;

            parentId = parentId.parent();
        }

        if (valuePtr == nullptr) {
            DebugLog(MicroServiceMessageHandlerLog) << "No parent (" << appName_ << ")" << valueId << std::endl;
            return true;
        }

        if (parentFilter_.has(valuePtr->hashToken())) return !isAppNameMatch(valueId);
    }

    while (valuePtr != nullptr) {
        VariantValue::Ptr parentPtr = valuePtr->parent();

        if (parentPtr == nullptr) break;

        if (parentFilter_.has(parentPtr->hashToken())) return !isAppNameMatch(valuePtr->id());

        valuePtr = parentPtr->parent();
    }

    return false;
}
