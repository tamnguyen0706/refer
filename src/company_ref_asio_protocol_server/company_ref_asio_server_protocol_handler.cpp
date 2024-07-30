/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_server_protocol_handler.cpp
 @brief VariantValueStore base CompanEdgeProtocol message handler
 */

#include "company_ref_asio_server_protocol_handler.h"

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_variant_valuestore/company_ref_variant_container_util.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <company_ref_dmo/company_ref_dmo_helper.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_protocol_utils/protobuf.h>
#include <company_ref_utils/company_ref_regex_utils.h>
#include <company_ref_utils/company_ref_weak_bind.h>

#include <company_ref_variant_valuestore/company_ref_variant_valuestore_visitor.h>

#include <Compan_logger/Compan_logger.h>

#include <exception>
#include <memory>
#include <regex>
#include <sstream>

using namespace Compan::Edge;

namespace {
template <typename T>
CompanEdgeProtocol::VsResult* addVsResult(
        CompanEdgeProtocol::ClientMessage& responseMessage,
        T& msg,
        CompanEdgeProtocol::VsResult::Status status)
{
    CompanEdgeProtocol::VsResult* pVsResult = responseMessage.mutable_vsresult();
    pVsResult->set_sequenceno(msg.sequenceno());
    pVsResult->set_status(status);
    return pVsResult;
}
} // namespace

CompanLogger ServerProtocolHandlerLog("server.protocol.handler", LogLevel::Information);

ServerProtocolHandler::ServerProtocolHandler(
        VariantValueStore& variantValueStore,
        DmoContainer& dmo,
        std::mutex& wsContainerMutex,
        uint32_t connectionId)
    : ws_(variantValueStore)
    , dmo_(dmo)
    , wsContainerMutex_(wsContainerMutex)
    , connectionId_(connectionId)
{
    FunctionArgLog(ServerProtocolHandlerLog) << __FUNCTION__ << " [" << connectionId_ << "]" << std::endl;
}

ServerProtocolHandler::~ServerProtocolHandler()
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;
    disconnect();
}

void ServerProtocolHandler::disconnect()
{
    wsSubscriberConnection_.clear();
    disconnectListeners();
}

void ServerProtocolHandler::doHandleMessage(ServerMessagePtr msgPtr)
{
    if (msgPtr) doMessage(*msgPtr);
}

void ServerProtocolHandler::doMessage(CompanEdgeProtocol::ServerMessage const& serverMessage)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    // ValueChanged are VsSyncComplete are not needed because they are server to client msg only
    if (serverMessage.has_vssync()) onSendCallback_(doMessage(serverMessage.vssync()));
    if (serverMessage.has_vssubscribe()) onSendCallback_(doMessage(serverMessage.vssubscribe()));
    if (serverMessage.has_vsunsubscribe()) onSendCallback_(doMessage(serverMessage.vsunsubscribe()));
    if (serverMessage.has_vssetvalue()) onSendCallback_(doMessage(serverMessage.vssetvalue()));
    if (serverMessage.has_vsmultiset()) onSendCallback_(doMessage(serverMessage.vsmultiset()));
    if (serverMessage.has_valuechanged()) onSendCallback_(doMessage(serverMessage.valuechanged()));
    if (serverMessage.has_valueremoved()) onSendCallback_(doMessage(serverMessage.valueremoved()));
    if (serverMessage.has_vsgetvalue()) onSendCallback_(doMessage(serverMessage.vsgetvalue()));
    if (serverMessage.has_vsmultiget()) onSendCallback_(doMessage(serverMessage.vsmultiget()));
    if (serverMessage.has_vsgetobject()) onSendCallback_(doMessage(serverMessage.vsgetobject()));
    if (serverMessage.has_vsgetall()) onSendCallback_(doMessage(serverMessage.vsgetall()));
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsSync const& vsSyncValue)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    disconnectListeners();

    // Populate response message (ClientMessage); it has ValuedChanged, ValueRemoved, VsResult and VsSyncCompleted
    // VsSyncCompleted service update
    msgPtr->mutable_vssynccompleted()->set_services(CompanEdgeProtocol::ValueStore);

    // ValueChanged values update
    CompanEdgeProtocol::ValueChanged* pValueChanged = msgPtr->mutable_valuechanged();

    if ((vsSyncValue.ids_size() <= 0)) {
        copyValueStoreToRepeated(pValueChanged->mutable_value());
        connectAllListeners();

        return msgPtr;
    }

    for (auto& valueId : vsSyncValue.ids()) {

        VariantValue::Ptr valuePtr = ws_.get(valueId);
        if (valuePtr == nullptr) {
            CompanEdgeProtocol::Value notFoundValue;
            notFoundValue.set_id(valueId);

            DebugLog(ServerProtocolHandlerLog) << "Variant ValueStore doesnt have valueId:" << valueId << std::endl;

            continue;
        }

        copyValueChildrenToRepeated(valuePtr, pValueChanged->mutable_value(), true);
    }

    connectAddRemoveListeners();

    return msgPtr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::ValueChanged const& valueChangedValue)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    // ValueChanged values update
    CompanEdgeProtocol::ValueChanged responseValueChanged;

    for (auto& valueIt : valueChangedValue.value()) {

        if (valueIt.has_addtocontainer()) {

            /// AddToContainer will send a response separately
            doAddToContainer(valueIt.id(), valueIt.addtocontainer());
        } else if (valueIt.has_removefromcontainer()) {

            /// RemoveFromContainer will send a response separately
            doRemoveFromContainer(valueIt.id(), valueIt.removefromcontainer());
        } else {
            // If value sets successfully, continue (sends a value changed via signal )
            // Failure ->
            // If the value is the same, continue
            // ELSE notify back the correct value

            VariantValue::Ptr valuePtr = ws_.get(valueIt.id());
            if (VariantValue::Ptr() == valuePtr) {
                DebugLog(ServerProtocolHandlerLog)
                        << "Variant ValueStore doesnt have valueId: " << valueIt.id() << std::endl;
                continue;
            }

            ValueDataSet::Results setResult = valuePtr->set(valueIt);
            if (setResult == ValueDataSet::Success || setResult == ValueDataSet::SameValue) { continue; }

            // notify back to the client what the value really is
            DebugLog(ServerProtocolHandlerLog) << "Variant ValueStore set failed on valueId: " << valueIt.id() << " - "
                                               << ValueDataSet::resultStr(setResult) << std::endl;

            *responseValueChanged.add_value() = valuePtr->get();
        }
    }

    // this gets called when a value update FAILED, so, we send back what it currently is
    if (responseValueChanged.value_size()) { *msgPtr->mutable_valuechanged() = std::move(responseValueChanged); }

    return msgPtr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsSubscribe const& vsSubscribeValue)
{
    FunctionArgLog(ServerProtocolHandlerLog)
            << "[" << connectionId_ << "] seq#:" << vsSubscribeValue.sequenceno() << std::endl;

    if ((vsSubscribeValue.ids_size() <= 0)) {

        ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

        // Populate response message (ClientMessage); it has ValuedChanged, ValueRemoved, VsResult and vsSyncCompleted
        // VsResult 'sequenceNo', 'status' and 'values' update
        CompanEdgeProtocol::VsResult* pVsResult =
                addVsResult(*msgPtr, vsSubscribeValue, CompanEdgeProtocol::VsResult::success);

        copyValueStoreToRepeated(pVsResult->mutable_values());
        connectAllListeners();

        return msgPtr;
    }

    // we return two different results based on a good/bad subscription
    ClientMessagePtr rspMsgSuccessPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();
    ClientMessagePtr rspMsgNotFoundPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    // send discreete messages about the subscription status
    CompanEdgeProtocol::VsResult* vsResultSuccess =
            addVsResult(*rspMsgSuccessPtr, vsSubscribeValue, CompanEdgeProtocol::VsResult::success);

    CompanEdgeProtocol::VsResult* vsResultNotFound =
            addVsResult(*rspMsgNotFoundPtr, vsSubscribeValue, CompanEdgeProtocol::VsResult::error_not_found);

    // VsResult values update
    for (auto& valueId : vsSubscribeValue.ids()) {

        VariantValue::Ptr valuePtr = ws_.get(valueId);
        if (valuePtr == nullptr) {
            CompanEdgeProtocol::Value notFoundValue;
            notFoundValue.set_id(valueId);

            *vsResultNotFound->add_values() = notFoundValue;

            DebugLog(ServerProtocolHandlerLog)
                    << "Variant ValueStore doesnt have valueId: (" << valueId << ")" << std::endl;

            continue;
        }

        copyValueChildrenToRepeated(valuePtr, vsResultSuccess->mutable_values(), true);
    }

    if (vsResultSuccess->values_size()) {
        connectAddRemoveListeners();
        onSendCallback_(rspMsgSuccessPtr);
    }

    if (vsResultNotFound->values_size()) onSendCallback_(rspMsgNotFoundPtr);

    return nullptr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsUnsubscribe const& vsUnsubscribeValue)
{
    FunctionArgLog(ServerProtocolHandlerLog)
            << "[" << connectionId_ << "] seq#:" << vsUnsubscribeValue.sequenceno() << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    addVsResult(*msgPtr, vsUnsubscribeValue, CompanEdgeProtocol::VsResult::success);

    wsSubscriberConnection_.clear();
    disconnectListeners();

    return msgPtr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsGetAll const& vsGetAllValue)
{
    FunctionArgLog(ServerProtocolHandlerLog)
            << "[" << connectionId_ << "] seq#:" << vsGetAllValue.sequenceno() << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::VsResult* pVsResult = addVsResult(*msgPtr, vsGetAllValue, CompanEdgeProtocol::VsResult::success);

    if (ws_.size() <= 0) {
        DebugLog(ServerProtocolHandlerLog) << "WsMap is empty, size:" << ws_.size() << std::endl;
        return msgPtr;
    }

    copyValueStoreToRepeated(pVsResult->mutable_values());
    return msgPtr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsGetValue const& vsGetValue)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "] valueId:" << vsGetValue.id() << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::VsResult* pVsResult =
            addVsResult(*msgPtr, vsGetValue, CompanEdgeProtocol::VsResult::error_not_found);

    if (ws_.size() <= 0 || vsGetValue.id().empty()) {
        DebugLog(ServerProtocolHandlerLog)
                << "WsMap or valueId is empty, size:" << ws_.size() << ", valueId:" << vsGetValue.id() << std::endl;

        return msgPtr;
    }

    VariantValue::Ptr valuePtr = ws_.get(vsGetValue.id());
    if (VariantValue::Ptr() == valuePtr) {
        DebugLog(ServerProtocolHandlerLog) << "Not found valueId: " << vsGetValue.id() << std::endl;
        return msgPtr;
    }

    // complete response message (ClientMessage)
    pVsResult->set_status(CompanEdgeProtocol::VsResult::success);
    *pVsResult->add_values() = valuePtr->get();

    /// THIS NEEDS TO BE DEPRECATED - use GetObject instead
    //    if (valuePtr->type() == CompanEdgeProtocol::Container) {
    //        VariantValueVisitor::visitChildren(valuePtr, [&pVsResult](VariantValue::Ptr const& visitPtr) {
    //            *pVsResult->add_values() = visitPtr->get();
    //        });
    //    }

    return msgPtr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsSetValue const& vsSetValue)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "] valueId:" << vsSetValue.id() << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::VsResult* pVsResult =
            addVsResult(*msgPtr, vsSetValue, CompanEdgeProtocol::VsResult::error_not_found);

    if (ws_.size() <= 0 || vsSetValue.id().empty()) {
        DebugLog(ServerProtocolHandlerLog)
                << "WsMap or valueId is empty, size:" << ws_.size() << ", valueId: " << vsSetValue.id() << std::endl;

        return msgPtr;
    }

    VariantValue::Ptr valuePtr = ws_.get(vsSetValue.id());
    if (VariantValue::Ptr() == valuePtr) {
        DebugLog(ServerProtocolHandlerLog) << "Not found valueId: " << vsSetValue.id() << std::endl;

        return msgPtr;
    }

    ValueDataSet::Results setResult = valuePtr->set(vsSetValue.value());

    switch (setResult) {
    case ValueDataSet::Success:
    case ValueDataSet::SameValue: pVsResult->set_status(CompanEdgeProtocol::VsResult::success); break;

    case ValueDataSet::InvalidType: pVsResult->set_status(CompanEdgeProtocol::VsResult::wrong_value_type); break;
    case ValueDataSet::RangeError: pVsResult->set_status(CompanEdgeProtocol::VsResult::range_error); break;
    case ValueDataSet::EnumError: pVsResult->set_status(CompanEdgeProtocol::VsResult::enum_error); break;
    case ValueDataSet::AccessError: pVsResult->set_status(CompanEdgeProtocol::VsResult::access_error); break;
    }

    *pVsResult->add_values() = valuePtr->get();

    return msgPtr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::ValueRemoved const& valueRemovedValue)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    bool removed(false);
    for (auto& valueId : valueRemovedValue.id()) {
        // del value at Ws
        removed = ws_.del(valueId);
        if (!removed) {
            DebugLog(ServerProtocolHandlerLog)
                    << "Variant ValueStore removal failed on valueId:" << valueId << std::endl;
            continue;
        }
    }

    return nullptr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsMultiGet const& msg)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::VsMultiGetResult* vsMultiGetResult = msgPtr->mutable_vsmultigetresult();
    vsMultiGetResult->set_sequenceno(msg.sequenceno());

    for (auto& id : msg.ids()) {

        CompanEdgeProtocol::VsMultiGetResult_Result* result = vsMultiGetResult->add_results();

        VariantValue::Ptr valuePtr = ws_.get(id);
        if (valuePtr == nullptr) {
            result->set_error(CompanEdgeProtocol::VsMultiGetResult::ValueNotFound);
            result->set_description("Value not found");
            continue;
        }

        result->set_error(CompanEdgeProtocol::VsMultiGetResult::Success);
        *result->add_values() = valuePtr->get();

        if (valuePtr->type() == CompanEdgeProtocol::Container) {
            VariantValueVisitor::visitChildren(valuePtr, [&result](VariantValue::Ptr const& visitPtr) {
                *result->add_values() = visitPtr->get();
            });
        }
    }

    return msgPtr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsMultiSet const& msg)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::VsMultiSetResult* vsMultiSetResult = msgPtr->mutable_vsmultisetresult();
    vsMultiSetResult->set_sequenceno(msg.sequenceno());

    for (auto& setValue : msg.values()) {
        CompanEdgeProtocol::VsMultiSetResult_Result* result = vsMultiSetResult->add_results();

        VariantValue::Ptr valuePtr = ws_.get(setValue.id());

        result->set_id(setValue.id());

        if (valuePtr == nullptr) {
            result->set_error(CompanEdgeProtocol::VsMultiSetResult::ValueNotFound);
            result->set_description("Value not found");
            continue;
        }

        ValueDataSet::Results setResult = valuePtr->set(setValue.value());
        if (setResult == ValueDataSet::Success || setResult == ValueDataSet::SameValue) {
            result->set_error(CompanEdgeProtocol::VsMultiSetResult::Success);
            continue;
        }

        result->set_error(CompanEdgeProtocol::VsMultiSetResult::UnknownError);
        std::stringstream ss;
        ss << "Failed to set value: " << ValueDataSet::resultStr(setResult);
        result->set_description(ss.str());
    }

    return msgPtr;
}

ClientMessagePtr ServerProtocolHandler::doMessage(CompanEdgeProtocol::VsGetObject const& msg)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::VsResult* pVsResult = addVsResult(*msgPtr, msg, CompanEdgeProtocol::VsResult::error_not_found);

    if (ws_.size() <= 0 || msg.id().empty()) {
        DebugLog(ServerProtocolHandlerLog)
                << "WsMap or valueId is empty, size:" << ws_.size() << ", valueId:" << msg.id() << std::endl;

        return msgPtr;
    }

    size_t firstWildcard = msg.id().find_first_of('*');
    size_t lastWildcard = msg.id().find_last_of('*');

    // if we're passing a wild card, get the parent
    ValueId valueId(msg.id().substr(0, firstWildcard));

    VariantValue::Ptr valuePtr = ws_.get(valueId);
    if (valuePtr == nullptr) {
        DebugLog(ServerProtocolHandlerLog) << "Not found valueId: " << msg.id() << std::endl;
        return msgPtr;
    }

    if (firstWildcard == lastWildcard || firstWildcard == std::string::npos) {
        *pVsResult->add_values() = valuePtr->get();
    }

    std::regex regexSearch = RegexUtils::makeWildCardRegex(msg.id());
    VariantValueVisitor::visitChildren(valuePtr, [&regexSearch, &pVsResult](VariantValue::Ptr const& visitPtr) {
        if (!std::regex_match(visitPtr->id().name(), regexSearch)) return;
        *pVsResult->add_values() = visitPtr->get();
    });

    // complete response message (ClientMessage)
    if (pVsResult->values_size()) pVsResult->set_status(CompanEdgeProtocol::VsResult::success);

    return msgPtr;
}

bool ServerProtocolHandler::isParentSubscribed(VariantValue::Ptr const valuePtr)
{
    if (valuePtr == nullptr) return false;

    if (wsSubscriberConnection_.empty()) return true;

    if (wsSubscriberConnection_.has(valuePtr->hashToken())) return true;

    ValueId valuePath(valuePtr->id());

    while (!valuePath.empty()) {
        VariantValue::Ptr parentPtr = ws_.get(valuePath.parent().name());
        if (parentPtr == nullptr) return false;

        valuePath = valuePath.parent();

        if (!wsSubscriberConnection_.has(parentPtr->hashToken())) continue;

        wsSubscriberConnection_.insert(
                valuePtr->hashToken(),
                valuePtr->connectChangedListener(
                        WeakBind(&ServerProtocolHandler::onValueChanged, shared_from_this(), std::placeholders::_1)));

        return true;
    }

    return false;
}

void ServerProtocolHandler::onValueAdded(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    if (valuePtr == nullptr) return;

    // check if any of the parent's are subscribed
    if (!isParentSubscribed(valuePtr)) return;

    // AddToContainer functionality set's to VariantValue::Remote so that the
    //  AddToContainer handler can send the values in a single pass
    if (valuePtr->setUpdateType() == VariantValue::Remote) return;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::ValueChanged* valueChanged = msgPtr->mutable_valuechanged();
    *valueChanged->add_value() = valuePtr->get();

    onSendCallback_(msgPtr);
}

void ServerProtocolHandler::onValueChanged(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    if (valuePtr == nullptr) return;

    // containers are handled else where
    if (valuePtr->type() == CompanEdgeProtocol::ContainerAddTo
        || valuePtr->type() == CompanEdgeProtocol::ContainerRemoveFrom)
        return;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();
    CompanEdgeProtocol::ValueChanged* valueChanged = msgPtr->mutable_valuechanged();
    *valueChanged->add_value() = valuePtr->get();

    onSendCallback_(msgPtr);
}

void ServerProtocolHandler::onValueRemoved(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    if (valuePtr == nullptr) return;

    // make sure we aren't sending out a remove notification for something we arent' subscribed to
    if (!wsSubscriberConnection_.empty()) {

        if (!wsSubscriberConnection_.has(valuePtr->hashToken())) { return; }

        wsSubscriberConnection_.remove(valuePtr->hashToken());

        if (wsSubscriberConnection_.empty()) {
            if (removedListener_.connected()) { removedListener_.disconnect(); }
        }
    }

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();
    CompanEdgeProtocol::ValueRemoved* valueRemoved = msgPtr->mutable_valueremoved();
    valueRemoved->add_id(valuePtr->id());
    valueRemoved->add_hashtoken(valuePtr->hashToken());

    onSendCallback_(msgPtr);
}

void ServerProtocolHandler::onValueAddToContainer(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    if (valuePtr == nullptr) return;

    if (valuePtr->type() != CompanEdgeProtocol::ContainerAddTo) {
        DebugLog(ServerProtocolHandlerLog)
                << __FUNCTION__ << ": " << valuePtr->id() << " not ContainerAddTo" << std::endl;
        return;
    }

    // check if any of the parent's are subscribed
    if (!isParentSubscribed(valuePtr)) {
        DebugLog(ServerProtocolHandlerLog) << __FUNCTION__ << ": " << valuePtr->id() << " not subscribed" << std::endl;
        return;
    }

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::ValueChanged* valueChanged = msgPtr->mutable_valuechanged();

    VariantValue::Ptr containerPtr = ws_.get(ValueId(valuePtr->id(), valuePtr->get().addtocontainer().key()));
    if (containerPtr == nullptr) return;

    *valueChanged->add_value() = containerPtr->get();

    VariantValueVisitor::visitChildren(containerPtr, [&valueChanged](VariantValue::Ptr const& visitPtr) {
        *valueChanged->add_value() = visitPtr->get();
    });

    // send the AddToContainer
    *valueChanged->add_value() = valuePtr->get();

    VariantValue::Ptr selfPtr = ws_.get(valuePtr->id());
    if (selfPtr == nullptr) return;

    // send a notification for the container itself
    *valueChanged->add_value() = selfPtr->get();

    onSendCallback_(msgPtr);
}

void ServerProtocolHandler::onValueRemoveFromContainer(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    if (valuePtr == nullptr) return;

    // make sure we aren't sending out a remove notification for something we arent' subscribed to
    if (!wsSubscriberConnection_.empty()) {

        if (!wsSubscriberConnection_.has(valuePtr->hashToken())) { return; }

        wsSubscriberConnection_.remove(valuePtr->hashToken());

        if (wsSubscriberConnection_.empty()) {
            if (removedListener_.connected()) { removedListener_.disconnect(); }
        }
    }

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::ValueChanged* valueChanged = msgPtr->mutable_valuechanged();

    // send the RemoveFromContainer
    *valueChanged->add_value() = valuePtr->get();

    onSendCallback_(msgPtr);
}

void ServerProtocolHandler::connectAllListeners()
{
    connectAddRemoveListeners();

    if (!changedListener_.connected()) {
        changedListener_ = ws_.connectValueChangedListener(
                WeakBind(&ServerProtocolHandler::onValueChanged, shared_from_this(), std::placeholders::_1));
    }
}

void ServerProtocolHandler::connectAddRemoveListeners()
{
    if (!removedListener_.connected()) {
        removedListener_ = ws_.connectValueRemovedListener(
                WeakBind(&ServerProtocolHandler::onValueRemoved, shared_from_this(), std::placeholders::_1));
    }
    if (!addedListener_.connected()) {
        addedListener_ = ws_.connectValueAddedListener(
                WeakBind(&ServerProtocolHandler::onValueAdded, shared_from_this(), std::placeholders::_1));
    }
    if (!addToContainerListener_.connected()) {
        addToContainerListener_ = ws_.connectValueAddToContainerListener(
                WeakBind(&ServerProtocolHandler::onValueAddToContainer, shared_from_this(), std::placeholders::_1));
    }

    if (!removeFromContainerListener_.connected()) {
        removeFromContainerListener_ = ws_.connectValueRemoveFromContainerListener(WeakBind(
                &ServerProtocolHandler::onValueRemoveFromContainer, shared_from_this(), std::placeholders::_1));
    }
}

void ServerProtocolHandler::disconnectListeners()
{
    changedListener_.disconnect();
    addToContainerListener_.disconnect();
    removeFromContainerListener_.disconnect();
    addedListener_.disconnect();
    removedListener_.disconnect();
}

bool ServerProtocolHandler::doAddToContainer(std::string const& id, AddToContainer const& msg)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "] valueId:" << id << std::endl;

    ValueId containerId(id);

    VariantValue::Ptr containerPtr = ws_.get(containerId);
    if (containerPtr == nullptr || containerPtr->type() != CompanEdgeProtocol::Container) {
        ErrorLog(ServerProtocolHandlerLog) << __FUNCTION__ << " - Not a container: " << containerId << std::endl;
        return false;
    }

    /// Make sure that another process doesn't attempt to add/remove from a container in parallel
    std::lock_guard<std::mutex> lock(wsContainerMutex_);

    /// In the event we aren't subscribed to this, we need to signal back completion
    ///  eg - CLI needs notification
    SignalScopedConnection tempConnection;

    insertSubscriberFilter(containerPtr);

    ValueId keyId(containerId, msg.key());
    VariantValue::Ptr valuePtr = ws_.get(keyId);
    if (valuePtr != nullptr) {
        DebugLog(ServerProtocolHandlerLog) << __FUNCTION__ << " - Key exists: " << keyId << std::endl;

        VariantValue::Ptr addToPtr = ws_.addToContainer(containerId, msg.key(), msg.value(), VariantValue::Local);

        if (!addToContainerListener_.connected()) onValueAddToContainer(addToPtr);
        return false;
    }

    if (DmoValueStoreHelper(dmo_, ws_).insertChild(containerId, msg.key(), msg.value(), VariantValue::Remote)) {

        DebugLog(ServerProtocolHandlerLog) << __FUNCTION__ << " - Container added: " << keyId << std::endl;

        VariantValueVisitor::visitChildren(
                containerPtr, [this](VariantValue::Ptr const& visitPtr) { insertSubscriberFilter(visitPtr); });

        if (!addToContainerListener_.connected()) {
            VariantValue::Ptr addValuePtr =
                    VariantContainerUtil::makeAddToContainer(containerPtr, msg.key(), VariantValue::Local);
            if (addValuePtr) onValueAddToContainer(addValuePtr);
        }

        return true;
    }

    return false;
}

bool ServerProtocolHandler::doRemoveFromContainer(std::string const& id, RemoveFromContainer const& msg)
{
    ValueId containerId(id);

    VariantValue::Ptr containerPtr = ws_.get(containerId);
    if (containerPtr == nullptr || containerPtr->type() != CompanEdgeProtocol::Container) {
        DebugLog(ServerProtocolHandlerLog) << __FUNCTION__ << " - Not a container: " << containerId << std::endl;
        return false;
    }

    /// Make sure that another process doesn't attempt to add/remove from a container in parallel
    std::lock_guard<std::mutex> lock(wsContainerMutex_);

    ValueId keyId(containerId, msg.key());
    VariantValue::Ptr valuePtr = ws_.get(keyId);
    if (valuePtr == nullptr) {
        DebugLog(ServerProtocolHandlerLog) << __FUNCTION__ << " - Key doesn't exist: " << keyId << std::endl;
        return false;
    }

    /// In the event we aren't subscribed to this, we need to signal back completion
    ///  eg - CLI needs notification
    ws_.del(ValueId(id, msg.key()), VariantValue::Remote);

    return true;
}

void ServerProtocolHandler::copyValueStoreToRepeated(
        google::protobuf::RepeatedPtrField<CompanEdgeProtocol::Value>* repeatedValues)
{
    ws_.visitValues([&repeatedValues](VariantValue::Ptr const& value) { *repeatedValues->Add() = value->get(); });
}

void ServerProtocolHandler::insertSubscriberFilter(VariantValue::Ptr valuePtr)
{
    FunctionArgLog(ServerProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    if (valuePtr == nullptr) return;

    if (wsSubscriberConnection_.has(valuePtr->hashToken())) return;

    SignalScopedConnection connection = valuePtr->connectChangedListener(
            WeakBind(&ServerProtocolHandler::onValueChanged, shared_from_this(), std::placeholders::_1));
    wsSubscriberConnection_.insert(valuePtr->hashToken(), std::move(connection));
}

void ServerProtocolHandler::copyValueChildrenToRepeated(
        VariantValue::Ptr valuePtr,
        google::protobuf::RepeatedPtrField<CompanEdgeProtocol::Value>* repeatedValues,
        bool const subscribe)
{
    if (valuePtr == nullptr) return;

    if (subscribe) insertSubscriberFilter(valuePtr);

    *repeatedValues->Add() = valuePtr->get();

    VariantValueVisitor::visitChildren(
            valuePtr, [this, &repeatedValues, &subscribe](VariantValue::Ptr const& visitPtr) {
                if (subscribe) insertSubscriberFilter(visitPtr);

                *repeatedValues->Add() = visitPtr->get();
            });
}
