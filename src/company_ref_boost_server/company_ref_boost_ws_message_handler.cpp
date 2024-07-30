/**
 Copyright © 2023 COMPAN REF
 @file company_ref_boost_ws_message_handler.cpp
 @brief VariantValueStore based message handler
 */
#include "company_ref_boost_ws_message_handler.h"

using namespace Compan::Edge;

#include <company_ref_dmo/company_ref_dmo_helper.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_protocol_utils/protobuf.h>
#include <company_ref_utils/company_ref_regex_utils.h>

#include <company_ref_variant_valuestore/company_ref_variant_container_util.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_visitor.h>

#include <Compan_logger/Compan_logger.h>

#include <exception>
#include <memory>
#include <regex>
#include <sstream>

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

namespace Compan{
namespace Edge {
CompanLogger AebMessageHandlerLog("server.message.handler", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

CompanEdgeBoostWsMessageHandler::CompanEdgeBoostWsMessageHandler(
        boost::asio::io_context& ctx,
        VariantValueStore& variantValueStore,
        DmoContainer& dmo,
        uint32_t connectionId)
    : CompanEdgeBoostMessageHandler(ctx, connectionId)
    , variantValueStore_(variantValueStore)
    , dmo_(dmo)
{
}

CompanEdgeBoostWsMessageHandler::~CompanEdgeBoostWsMessageHandler()
{
}

void CompanEdgeBoostWsMessageHandler::disconnect()
{
    CompanEdgeBoostMessageHandler::disconnect();

    wsSubscriberConnection_.clear();
    disconnectListeners();
}

void CompanEdgeBoostWsMessageHandler::handleMessage(CompanEdgeProtocol::ServerMessage const& serverMessage)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    // All onMessage() is using member variable responseMessagePtr
    ClientMessagePtr rspMsgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    // ValueChanged are VsSyncComplete are not needed because they are server to client msg only
    if (serverMessage.has_vssync()) onMessage(serverMessage.vssync(), rspMsgPtr);
    if (serverMessage.has_valuechanged()) onMessage(serverMessage.valuechanged(), rspMsgPtr);
    if (serverMessage.has_vssubscribe()) onMessage(serverMessage.vssubscribe(), rspMsgPtr);
    if (serverMessage.has_vsunsubscribe()) onMessage(serverMessage.vsunsubscribe(), rspMsgPtr);
    if (serverMessage.has_vsgetall()) onMessage(serverMessage.vsgetall(), rspMsgPtr);
    if (serverMessage.has_vsgetvalue()) onMessage(serverMessage.vsgetvalue(), rspMsgPtr);
    if (serverMessage.has_vssetvalue()) onMessage(serverMessage.vssetvalue(), rspMsgPtr);
    if (serverMessage.has_valueremoved()) onMessage(serverMessage.valueremoved(), rspMsgPtr);
    if (serverMessage.has_vsmultiget()) onMessage(serverMessage.vsmultiget(), rspMsgPtr);
    if (serverMessage.has_vsmultiset()) onMessage(serverMessage.vsmultiset(), rspMsgPtr);
    if (serverMessage.has_vsgetobject()) onMessage(serverMessage.vsgetobject(), rspMsgPtr);

    // quirk-around for unit-tests
    if (!rspMsgPtr) return;

    // only enqueue a response if a response is necessary
    if (rspMsgPtr->has_valuechanged() || rspMsgPtr->has_valueremoved() || rspMsgPtr->has_vsresult()
        || rspMsgPtr->has_vssynccompleted() || rspMsgPtr->has_vsmultigetresult() || rspMsgPtr->has_vsmultisetresult()) {

        onClientMessageSignal_(rspMsgPtr);
    }
}

void CompanEdgeBoostWsMessageHandler::onMessage(CompanEdgeProtocol::VsSync const& vsSyncValue, ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    disconnectListeners();

    // Populate response message (ClientMessage); it has ValuedChanged, ValueRemoved, VsResult and VsSyncCompleted
    // VsSyncCompleted service update
    rspMsgPtr->mutable_vssynccompleted()->set_services(CompanEdgeProtocol::ValueStore);

    // ValueChanged values update
    CompanEdgeProtocol::ValueChanged* pValueChanged = rspMsgPtr->mutable_valuechanged();

    if ((vsSyncValue.ids_size() <= 0)) {
        copyValueStoreToRepeated(pValueChanged->mutable_value());
        connectAllListeners();
        return;
    }

    for (auto& valueId : vsSyncValue.ids()) {

        VariantValue::Ptr valuePtr = variantValueStore_.get(valueId);
        if (valuePtr == nullptr) {
            CompanEdgeProtocol::Value notFoundValue;
            notFoundValue.set_id(valueId);

            DebugLog(AebMessageHandlerLog) << "Variant ValueStore doesnt have valueId:" << valueId << std::endl;

            continue;
        }

        copyValueChildrenToRepeated(valuePtr, pValueChanged->mutable_value(), true);
    }

    connectAddRemoveListeners();
}

void CompanEdgeBoostWsMessageHandler::onMessage(
        CompanEdgeProtocol::ValueChanged const& valueChangedValue,
        ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    // ValueChanged values update
    CompanEdgeProtocol::ValueChanged responseValueChanged;

    for (auto& valueIt : valueChangedValue.value()) {

        if (valueIt.has_addtocontainer()) {

            /// AddToContainer will send a response separately
            handleAddToContainer(valueIt.id(), valueIt.addtocontainer());
        } else if (valueIt.has_removefromcontainer()) {

            /// RemoveFromContainer will send a response separately
            handleRemoveFromContainer(valueIt.id(), valueIt.removefromcontainer());
        } else {
            // If value sets successfully, continue (sends a value changed via signal )
            // Failure ->
            // If the value is the same, continue
            // ELSE notify back the correct value

            VariantValue::Ptr valuePtr = variantValueStore_.get(valueIt.id());
            if (VariantValue::Ptr() == valuePtr) {
                ErrorLog(AebMessageHandlerLog)
                        << "Variant ValueStore doesnt have valueId: " << valueIt.id() << std::endl;
                continue;
            }

            ValueDataSet::Results setResult = valuePtr->set(valueIt);
            if (setResult == ValueDataSet::Success || setResult == ValueDataSet::SameValue) { continue; }

            // notify back to the client what the value really is
            ErrorLog(AebMessageHandlerLog) << "Variant ValueStore set failed on valueId: " << valueIt.id() << " - "
                                           << ValueDataSet::resultStr(setResult) << std::endl;

            *responseValueChanged.add_value() = valuePtr->get();
        }
    }

    if (responseValueChanged.value_size()) *rspMsgPtr->mutable_valuechanged() = std::move(responseValueChanged);
}

void CompanEdgeBoostWsMessageHandler::onMessage(CompanEdgeProtocol::VsSubscribe const& vsSubscribeValue, ClientMessagePtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] seq#:" << vsSubscribeValue.sequenceno()
                                         << std::endl;

    if ((vsSubscribeValue.ids_size() <= 0)) {

        ClientMessagePtr rspMsgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

        // Populate response message (ClientMessage); it has ValuedChanged, ValueRemoved, VsResult and vsSyncCompleted
        // VsResult 'sequenceNo', 'status' and 'values' update
        CompanEdgeProtocol::VsResult* pVsResult =
                addVsResult(*rspMsgPtr, vsSubscribeValue, CompanEdgeProtocol::VsResult::success);

        copyValueStoreToRepeated(pVsResult->mutable_values());
        connectAllListeners();

        onClientMessageSignal_(rspMsgPtr);

        return;
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

        VariantValue::Ptr valuePtr = variantValueStore_.get(valueId);
        if (valuePtr == nullptr) {
            CompanEdgeProtocol::Value notFoundValue;
            notFoundValue.set_id(valueId);

            *vsResultNotFound->add_values() = notFoundValue;

            DebugLog(AebMessageHandlerLog)
                    << "Variant ValueStore doesnt have valueId: (" << valueId << ")" << std::endl;

            continue;
        }

        copyValueChildrenToRepeated(valuePtr, vsResultSuccess->mutable_values(), true);
    }

    if (vsResultSuccess->values_size()) {
        connectAddRemoveListeners();
        onClientMessageSignal_(rspMsgSuccessPtr);
    }

    if (vsResultNotFound->values_size()) onClientMessageSignal_(rspMsgNotFoundPtr);
}

void CompanEdgeBoostWsMessageHandler::onMessage(
        CompanEdgeProtocol::VsUnsubscribe const& vsUnsubscribeValue,
        ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] seq#:" << vsUnsubscribeValue.sequenceno()
                                         << std::endl;

    addVsResult(*rspMsgPtr, vsUnsubscribeValue, CompanEdgeProtocol::VsResult::success);

    wsSubscriberConnection_.clear();
    disconnectListeners();
}

void CompanEdgeBoostWsMessageHandler::onMessage(
        CompanEdgeProtocol::VsGetAll const& vsGetAllValue,
        ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] seq#:" << vsGetAllValue.sequenceno()
                                         << std::endl;

    CompanEdgeProtocol::VsResult* pVsResult = addVsResult(*rspMsgPtr, vsGetAllValue, CompanEdgeProtocol::VsResult::success);

    if (variantValueStore_.size() <= 0) {
        DebugLog(AebMessageHandlerLog) << "WsMap is empty, size:" << variantValueStore_.size() << std::endl;
        return;
    }

    copyValueStoreToRepeated(pVsResult->mutable_values());
}

void CompanEdgeBoostWsMessageHandler::onMessage(
        CompanEdgeProtocol::VsGetValue const& vsGetValue,
        ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] valueId:" << vsGetValue.id() << std::endl;

    CompanEdgeProtocol::VsResult* pVsResult =
            addVsResult(*rspMsgPtr, vsGetValue, CompanEdgeProtocol::VsResult::error_not_found);

    if (variantValueStore_.size() <= 0 || vsGetValue.id().empty()) {
        DebugLog(AebMessageHandlerLog) << "WsMap or valueId is empty, size:" << variantValueStore_.size()
                                       << ", valueId:" << vsGetValue.id() << std::endl;
        return;
    }

    VariantValue::Ptr valuePtr = variantValueStore_.get(vsGetValue.id());
    if (VariantValue::Ptr() == valuePtr) {
        DebugLog(AebMessageHandlerLog) << "Not found valueId: " << vsGetValue.id() << std::endl;
        return;
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
}

void CompanEdgeBoostWsMessageHandler::onMessage(
        CompanEdgeProtocol::VsSetValue const& vsSetValue,
        ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] valueId:" << vsSetValue.id() << std::endl;

    CompanEdgeProtocol::VsResult* pVsResult =
            addVsResult(*rspMsgPtr, vsSetValue, CompanEdgeProtocol::VsResult::error_not_found);

    if (variantValueStore_.size() <= 0 || vsSetValue.id().empty()) {
        DebugLog(AebMessageHandlerLog) << "WsMap or valueId is empty, size:" << variantValueStore_.size()
                                       << ", valueId: " << vsSetValue.id() << std::endl;
        return;
    }

    VariantValue::Ptr valuePtr = variantValueStore_.get(vsSetValue.id());
    if (VariantValue::Ptr() == valuePtr) {
        DebugLog(AebMessageHandlerLog) << "Not found valueId: " << vsSetValue.id() << std::endl;
        return;
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
}

void CompanEdgeBoostWsMessageHandler::onMessage(CompanEdgeProtocol::ValueRemoved const& valueRemovedValue, ClientMessagePtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    bool removed(false);
    for (auto& valueId : valueRemovedValue.id()) {
        // del value at Ws
        removed = variantValueStore_.del(valueId);
        if (!removed) {
            ErrorLog(AebMessageHandlerLog) << "Variant ValueStore removal failed on valueId:" << valueId << std::endl;
            continue;
        }
    }
}

void CompanEdgeBoostWsMessageHandler::onMessage(CompanEdgeProtocol::VsMultiGet const& msg, ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    CompanEdgeProtocol::VsMultiGetResult* vsMultiGetResult = rspMsgPtr->mutable_vsmultigetresult();
    vsMultiGetResult->set_sequenceno(msg.sequenceno());

    for (auto& id : msg.ids()) {

        CompanEdgeProtocol::VsMultiGetResult_Result* result = vsMultiGetResult->add_results();

        VariantValue::Ptr valuePtr = variantValueStore_.get(id);
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
}

void CompanEdgeBoostWsMessageHandler::onMessage(CompanEdgeProtocol::VsMultiSet const& msg, ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    CompanEdgeProtocol::VsMultiSetResult* vsMultiSetResult = rspMsgPtr->mutable_vsmultisetresult();
    vsMultiSetResult->set_sequenceno(msg.sequenceno());

    for (auto& setValue : msg.values()) {
        CompanEdgeProtocol::VsMultiSetResult_Result* result = vsMultiSetResult->add_results();

        VariantValue::Ptr valuePtr = variantValueStore_.get(setValue.id());

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
}

void CompanEdgeBoostWsMessageHandler::onMessage(CompanEdgeProtocol::VsGetObject const& msg, ClientMessagePtr rspMsgPtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "]" << std::endl;

    CompanEdgeProtocol::VsResult* pVsResult = addVsResult(*rspMsgPtr, msg, CompanEdgeProtocol::VsResult::error_not_found);

    if (variantValueStore_.size() <= 0 || msg.id().empty()) {
        DebugLog(AebMessageHandlerLog) << "WsMap or valueId is empty, size:" << variantValueStore_.size()
                                       << ", valueId:" << msg.id() << std::endl;
        return;
    }

    size_t firstWildcard = msg.id().find_first_of('*');
    size_t lastWildcard = msg.id().find_last_of('*');

    // if we're passing a wild card, get the parent
    ValueId valueId(msg.id().substr(0, firstWildcard));

    VariantValue::Ptr valuePtr = variantValueStore_.get(valueId);
    if (valuePtr == nullptr) {
        DebugLog(AebMessageHandlerLog) << "Not found valueId: " << msg.id() << std::endl;
        return;
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
}

bool CompanEdgeBoostWsMessageHandler::isParentSubscribed(VariantValue::Ptr const valuePtr)
{
    if (wsSubscriberConnection_.empty()) return true;

    if (wsSubscriberConnection_.has(valuePtr->hashToken())) return true;

    ValueId valuePath(valuePtr->id());

    while (!valuePath.empty()) {
        VariantValue::Ptr parentPtr = variantValueStore_.get(valuePath.parent().name());
        if (parentPtr == nullptr) return false;

        valuePath = valuePath.parent();

        if (!wsSubscriberConnection_.has(parentPtr->hashToken())) continue;

        wsSubscriberConnection_.insert(
                valuePtr->hashToken(),
                valuePtr->connectChangedListener(std::bind(
                        &CompanEdgeBoostWsMessageHandler::handleValueChanged,
                        shared_from_this(),
                        std::placeholders::_1)));
        return true;
    }

    return false;
}

void CompanEdgeBoostWsMessageHandler::handleValueAdded(VariantValue::Ptr const value)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] valueId:" << value->id()
                                         << ", hash:" << value->hashToken() << std::endl;

    // check if any of the parent's are subscribed
    if (!isParentSubscribed(value)) return;

    // AddToContainer functionality set's to VariantValue::Remote so that the
    //  AddToContainer handler can send the values in a single pass
    if (value->setUpdateType() == VariantValue::Remote) return;

    ClientMessagePtr rspMsgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::ValueChanged* valueChanged = rspMsgPtr->mutable_valuechanged();
    *valueChanged->add_value() = value->get();

    onClientMessageSignal_(rspMsgPtr);
}

void CompanEdgeBoostWsMessageHandler::handleValueChanged(VariantValue::Ptr const value)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] valueId:" << value->id()
                                         << ", hash:" << value->hashToken() << std::endl;

    // containers are handled else where
    if (value->type() == CompanEdgeProtocol::ContainerAddTo || value->type() == CompanEdgeProtocol::ContainerRemoveFrom)
        return;

    ClientMessagePtr rspMsgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();
    CompanEdgeProtocol::ValueChanged* valueChanged = rspMsgPtr->mutable_valuechanged();
    *valueChanged->add_value() = value->get();

    onClientMessageSignal_(rspMsgPtr);
}

void CompanEdgeBoostWsMessageHandler::handleValueRemoved(VariantValue::Ptr const value)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] valueId:" << value->id() << std::endl;

    // make sure we aren't sending out a remove notification for something we arent' subscribed to
    if (!wsSubscriberConnection_.empty()) {

        if (!wsSubscriberConnection_.has(value->hashToken())) { return; }

        wsSubscriberConnection_.remove(value->hashToken());

        if (wsSubscriberConnection_.empty()) {
            if (removedListener_.connected()) { removedListener_.disconnect(); }
        }
    }

    ClientMessagePtr rspMsgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();
    CompanEdgeProtocol::ValueRemoved* valueRemoved = rspMsgPtr->mutable_valueremoved();
    valueRemoved->add_id(value->id());
    valueRemoved->add_hashtoken(value->hashToken());

    onClientMessageSignal_(rspMsgPtr);
}

void CompanEdgeBoostWsMessageHandler::handleValueAddToContainer(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] valueId:" << valuePtr->id()
                                         << ", hash:" << valuePtr->hashToken() << std::endl;

    if (valuePtr->type() != CompanEdgeProtocol::ContainerAddTo) {
        DebugLog(AebMessageHandlerLog) << __FUNCTION__ << ": " << valuePtr->id() << " not ContainerAddTo" << std::endl;
        return;
    }

    // check if any of the parent's are subscribed
    if (!isParentSubscribed(valuePtr)) {
        DebugLog(AebMessageHandlerLog) << __FUNCTION__ << ": " << valuePtr->id() << " not subscribed" << std::endl;
        return;
    }

    ClientMessagePtr rspMsgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::ValueChanged* valueChanged = rspMsgPtr->mutable_valuechanged();

    VariantValue::Ptr containerPtr =
            variantValueStore_.get(ValueId(valuePtr->id(), valuePtr->get().addtocontainer().key()));

    *valueChanged->add_value() = containerPtr->get();

    VariantValueVisitor::visitChildren(containerPtr, [&valueChanged](VariantValue::Ptr const& visitPtr) {
        *valueChanged->add_value() = visitPtr->get();
    });

    // send the AddToContainer
    *valueChanged->add_value() = valuePtr->get();

    // send a notification for the container itself
    *valueChanged->add_value() = variantValueStore_.get(valuePtr->id())->get();

    onClientMessageSignal_(rspMsgPtr);
}

void CompanEdgeBoostWsMessageHandler::handleValueRemoveFromContainer(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] valueId:" << valuePtr->id()
                                         << ", hash:" << valuePtr->hashToken() << std::endl;

    // make sure we aren't sending out a remove notification for something we arent' subscribed to
    if (!wsSubscriberConnection_.empty()) {

        if (!wsSubscriberConnection_.has(valuePtr->hashToken())) { return; }

        wsSubscriberConnection_.remove(valuePtr->hashToken());

        if (wsSubscriberConnection_.empty()) {
            if (removedListener_.connected()) { removedListener_.disconnect(); }
        }
    }

    ClientMessagePtr rspMsgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();

    CompanEdgeProtocol::ValueChanged* valueChanged = rspMsgPtr->mutable_valuechanged();

    // send the RemoveFromContainer
    *valueChanged->add_value() = valuePtr->get();

    onClientMessageSignal_(rspMsgPtr);
}

void CompanEdgeBoostWsMessageHandler::connectAllListeners()
{
    connectAddRemoveListeners();

    if (!changedListener_.connected()) {
        changedListener_ = variantValueStore_.connectValueChangedListener(std::bind(
                &CompanEdgeBoostWsMessageHandler::handleValueChanged, shared_from_this(), std::placeholders::_1));
    }
}

void CompanEdgeBoostWsMessageHandler::connectAddRemoveListeners()
{
    if (!removedListener_.connected()) {
        removedListener_ = variantValueStore_.connectValueRemovedListener(std::bind(
                &CompanEdgeBoostWsMessageHandler::handleValueRemoved, shared_from_this(), std::placeholders::_1));
    }
    if (!addedListener_.connected()) {
        addedListener_ = variantValueStore_.connectValueAddedListener(
                std::bind(&CompanEdgeBoostWsMessageHandler::handleValueAdded, shared_from_this(), std::placeholders::_1));
    }
    if (!addToContainerListener_.connected()) {
        addToContainerListener_ = variantValueStore_.connectValueAddToContainerListener(std::bind(
                &CompanEdgeBoostWsMessageHandler::handleValueAddToContainer, shared_from_this(), std::placeholders::_1));
    }

    if (!removeFromContainerListener_.connected()) {
        removeFromContainerListener_ = variantValueStore_.connectValueRemoveFromContainerListener(std::bind(
                &CompanEdgeBoostWsMessageHandler::handleValueRemoveFromContainer,
                shared_from_this(),
                std::placeholders::_1));
    }
}

void CompanEdgeBoostWsMessageHandler::disconnectListeners()
{
    if (changedListener_.connected()) { changedListener_.disconnect(); }
    if (addToContainerListener_.connected()) { addToContainerListener_.disconnect(); }
    if (removeFromContainerListener_.connected()) { removeFromContainerListener_.disconnect(); }
    if (addedListener_.connected()) { addedListener_.disconnect(); }
    if (removedListener_.connected()) { removedListener_.disconnect(); }
}

bool CompanEdgeBoostWsMessageHandler::handleAddToContainer(std::string const& id, AddToContainer const& msg)
{
    FunctionArgLog(AebMessageHandlerLog) << "[" << connectionId_ << "] valueId:" << id << std::endl;

    ValueId containerId(id);

    VariantValue::Ptr containerPtr = variantValueStore_.get(containerId);
    if (containerPtr == nullptr || containerPtr->type() != CompanEdgeProtocol::Container) {
        ErrorLog(AebMessageHandlerLog) << __FUNCTION__ << " - Not a container: " << containerId << std::endl;
        return false;
    }

    /// In the event we aren't subscribed to this, we need to signal back completion
    ///  eg - CLI needs notification
    SignalScopedConnection tempConnection;

    if (!addToContainerListener_.connected())
        tempConnection = variantValueStore_.connectValueAddToContainerListener(std::bind(
                &CompanEdgeBoostWsMessageHandler::handleValueAddToContainer, shared_from_this(), std::placeholders::_1));

    insertSubscriberFilter(containerPtr);

    ValueId keyId(containerId, msg.key());
    VariantValue::Ptr valuePtr = variantValueStore_.get(keyId);
    if (valuePtr != nullptr) {
        DebugLog(AebMessageHandlerLog) << __FUNCTION__ << " - Key exists: " << keyId << std::endl;

        variantValueStore_.addToContainer(containerId, msg.key(), msg.value(), VariantValue::Local);
        return false;
    }

    if (DmoValueStoreHelper(dmo_, variantValueStore_)
                .insertChild(containerId, msg.key(), msg.value(), VariantValue::Remote)) {

        DebugLog(AebMessageHandlerLog) << __FUNCTION__ << " - Container added: " << keyId << std::endl;

        VariantValueVisitor::visitChildren(
                containerPtr, [this](VariantValue::Ptr const& visitPtr) { insertSubscriberFilter(visitPtr); });

        if (!addToContainerListener_.connected()) {
            VariantValue::Ptr addValuePtr =
                    VariantContainerUtil::makeAddToContainer(containerPtr, msg.key(), VariantValue::Local);
            if (addValuePtr) handleValueAddToContainer(addValuePtr);
        }

        return true;
    }

    return false;
}

bool CompanEdgeBoostWsMessageHandler::handleRemoveFromContainer(std::string const& id, RemoveFromContainer const& msg)
{
    ValueId containerId(id);

    VariantValue::Ptr containerPtr = variantValueStore_.get(containerId);
    if (containerPtr == nullptr || containerPtr->type() != CompanEdgeProtocol::Container) {
        ErrorLog(AebMessageHandlerLog) << __FUNCTION__ << " - Not a container: " << containerId << std::endl;
        return false;
    }

    ValueId keyId(containerId, msg.key());
    VariantValue::Ptr valuePtr = variantValueStore_.get(keyId);
    if (valuePtr == nullptr) {
        ErrorLog(AebMessageHandlerLog) << __FUNCTION__ << " - Key doesn't exist: " << keyId << std::endl;
        return false;
    }

    /// In the event we aren't subscribed to this, we need to signal back completion
    ///  eg - CLI needs notification
    SignalScopedConnection tempConnection;

    if (!removeFromContainerListener_.connected())
        tempConnection = variantValueStore_.connectValueRemoveFromContainerListener(std::bind(
                &CompanEdgeBoostWsMessageHandler::handleValueRemoveFromContainer,
                shared_from_this(),
                std::placeholders::_1));

    variantValueStore_.del(ValueId(id, msg.key()), VariantValue::Remote);

    return true;
}

void CompanEdgeBoostWsMessageHandler::copyValueStoreToRepeated(
        google::protobuf::RepeatedPtrField<CompanEdgeProtocol::Value>* repeatedValues)
{
    variantValueStore_.visitValues(
            [&repeatedValues](VariantValue::Ptr const& value) { *repeatedValues->Add() = value->get(); });
}

void CompanEdgeBoostWsMessageHandler::insertSubscriberFilter(VariantValue::Ptr valuePtr)
{
    if (wsSubscriberConnection_.has(valuePtr->hashToken())) return;

    wsSubscriberConnection_.insert(
            valuePtr->hashToken(),
            valuePtr->connectChangedListener(std::bind(
                    &CompanEdgeBoostWsMessageHandler::handleValueChanged, shared_from_this(), std::placeholders::_1)));
}

void CompanEdgeBoostWsMessageHandler::copyValueChildrenToRepeated(
        VariantValue::Ptr valuePtr,
        google::protobuf::RepeatedPtrField<CompanEdgeProtocol::Value>* repeatedValues,
        bool const subscribe)
{
    if (subscribe) insertSubscriberFilter(valuePtr);

    *repeatedValues->Add() = valuePtr->get();

    VariantValueVisitor::visitChildren(
            valuePtr, [this, &repeatedValues, &subscribe](VariantValue::Ptr const& visitPtr) {
                if (subscribe) insertSubscriberFilter(visitPtr);

                *repeatedValues->Add() = visitPtr->get();
            });
}
