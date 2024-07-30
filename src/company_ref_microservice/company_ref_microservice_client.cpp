/**
 Copyright © 2023 COMPAN REF
 @file company_ref_microservice_client.cpp
 @brief MicroService Client
 */

#include "company_ref_microservice_client.h"
#include <company_ref_microservice/company_ref_microservice_dynamic_dmo.h>

#include <company_ref_sdk_value_ids/dynamic_dmo_meta_data.h>
#include <company_ref_sdk_value_ids/dynamic_dmo_value_ids.h>
#include <company_ref_sdk_value_ids/logger_meta_data.h>
#include <company_ref_sdk_value_ids/microservices_meta_data.h>
#include <company_ref_sdk_value_ids/microservices_value_ids.h>

#include <company_ref_asio_protocol_client/company_ref_asio_tcp_protocol_client.h>
#include <company_ref_asio_protocol_client/company_ref_asio_uds_protocol_client.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>
#include <company_ref_protocol_utils/company_ref_stream.h>

#include <company_ref_main_apps/Compan_logger_factory.h>

#include <company_ref_variant_valuestore/company_ref_variant_bool_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_container_util.h>
#include <company_ref_variant_valuestore/company_ref_variant_enum_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <Compan_logger/Compan_logger.h>
#include <Compan_logger/Compan_logger_controller.h>

#include <unistd.h>

using namespace Compan::Edge;

CompanLogger MicroServiceClientLog("MsClient.Client", LogLevel::Information);

MicroServiceClient::MicroServiceClient(
        boost::asio::io_context& ctx,
        std::string const& appName,
        std::string const& udsPath)
    : MicroServiceClient(ctx, appName)
{
    clientConnection(std::make_shared<AsioUdsProtocolClient>(ctx, messageHandler_, udsPath));
}

MicroServiceClient::MicroServiceClient(
        boost::asio::io_context& ctx,
        std::string const& appName,
        std::string const& address,
        std::string const& port)
    : MicroServiceClient(ctx, appName)
{
    clientConnection(std::make_shared<AsioTcpProtocolClient>(ctx, messageHandler_, address, port));
}

MicroServiceClient::MicroServiceClient(boost::asio::io_context& ctx, std::string const& appName)
    : ctx_(ctx)
    , appName_(appName)
    , ws_(ctx_)
    , autoReconnect_(true)
    , seqNo_(0)
    , messageHandler_(std::make_shared<MicroServiceMessageHandler>(ws_, appName_, seqNo_))
    , connected_(false)
    , subscribeCompleteMapCb_()
    , dynamicDmo_(std::make_shared<MicroserviceDynamicDmo>(ws_, appName_))
{
    FunctionLog(MicroServiceClientLog);

    LoggerMetaData::create(ws_);
    DynamicDmoMetaData::create(ws_);
    MicroservicesMetaData::create(ws_);

    messageHandler_->insertAppNameFilter("logger");
    messageHandler_->insertAppNameFilter(ValueId(DynamicDmoValueIds::BaseValueId, "dmo"));

    LoggerFactory::createBinders(appName_, ws_, true);

    {
        // subscribe to the appname's logger elements
        //  first start - subscription is automatic, because the elements are added
        //  second start- they exist already, so, subscription notify changes
        CompanEdgeProtocol::ServerMessage msg;

        CompanEdgeProtocol::VsSubscribe* pVsSubscribe = msg.mutable_vssubscribe();
        pVsSubscribe->set_sequenceno(++seqNo_);
        pVsSubscribe->add_ids(ValueId(LoggerFactory::LoggerId, appName_));

        insertWriteQueue(std::move(msg));
    }

    // try to subscribe to the appname's microservice elements
    //  These may NOT exist if you're running the microservice outside
    //  of the microservice's manager
    subscribeWithCompletion(
            ValueId(MicroservicesValueIds::BaseValueId, appName_),
            std::bind(&MicroServiceClient::onMicroserviceSubcriptionComplete, this, std::placeholders::_1));
}

MicroServiceClient::~MicroServiceClient()
{
    FunctionLog(MicroServiceClientLog);
}

MicroserviceDynamicDmo::Ptr MicroServiceClient::getDynamicDmo()
{
    return dynamicDmo_;
}

int MicroServiceClient::subscribeWithCompletion(std::string const& valueId, SubscribeCompleteCb const& cb)
{
    FunctionLog(MicroServiceClientLog);

    if (!onSubscribeComplete_.connected()) {
        onSubscribeComplete_ = messageHandler_->connectSubscribeListener(
                std::bind(&MicroServiceClient::subscribeComplete, this, std::placeholders::_1));
    }

    CompanEdgeProtocol::ServerMessage msg;

    CompanEdgeProtocol::VsSubscribe* pVsSubscribe = msg.mutable_vssubscribe();
    pVsSubscribe->set_sequenceno(++seqNo_);
    pVsSubscribe->add_ids(valueId);

    subscribeCompleteMapCb_.emplace(std::pair<uint32_t, SubscribeCompleteCb>(pVsSubscribe->sequenceno(), cb));

    DebugLog(MicroServiceClientLog) << __FUNCTION__ << ": " << *pVsSubscribe << std::endl;

    insertWriteQueue(std::move(msg));

    return pVsSubscribe->sequenceno();
}

int MicroServiceClient::subscribeWithCompletion(std::vector<std::string> const& valueIds, SubscribeCompleteCb const& cb)
{
    FunctionLog(MicroServiceClientLog);

    if (!onSubscribeComplete_.connected()) {
        onSubscribeComplete_ = messageHandler_->connectSubscribeListener(
                std::bind(&MicroServiceClient::subscribeComplete, this, std::placeholders::_1));
    }

    CompanEdgeProtocol::ServerMessage msg;

    CompanEdgeProtocol::VsSubscribe* pVsSubscribe = msg.mutable_vssubscribe();
    pVsSubscribe->set_sequenceno(++seqNo_);

    for (auto& valueId : valueIds) pVsSubscribe->add_ids(valueId);

    subscribeCompleteMapCb_.emplace(std::pair<uint32_t, SubscribeCompleteCb>(pVsSubscribe->sequenceno(), cb));

    DebugLog(MicroServiceClientLog) << __FUNCTION__ << ": " << *pVsSubscribe << std::endl;

    insertWriteQueue(std::move(msg));

    return pVsSubscribe->sequenceno();
}

int MicroServiceClient::subscribeAll(SubscribeCompleteCb const& cb)
{
    FunctionLog(MicroServiceClientLog);

    if (!onSubscribeComplete_.connected()) {
        onSubscribeComplete_ = messageHandler_->connectSubscribeListener(
                std::bind(&MicroServiceClient::subscribeComplete, this, std::placeholders::_1));
    }

    messageHandler_->removeAppNameFilter("logger");
    messageHandler_->removeAppNameFilter(DynamicDmoValueIds::BaseValueId);

    CompanEdgeProtocol::ServerMessage msg;

    CompanEdgeProtocol::VsSubscribe* pVsSubscribe = msg.mutable_vssubscribe();
    pVsSubscribe->set_sequenceno(++seqNo_);

    subscribeCompleteMapCb_.emplace(std::pair<uint32_t, SubscribeCompleteCb>(pVsSubscribe->sequenceno(), cb));

    DebugLog(MicroServiceClientLog) << __FUNCTION__ << ": " << *pVsSubscribe << std::endl;

    insertWriteQueue(std::move(msg));

    return pVsSubscribe->sequenceno();
}

void MicroServiceClient::cancelSubscibeCompletion(int const subscription)
{
    FunctionLog(MicroServiceClientLog);

    auto it = subscribeCompleteMapCb_.find(subscription);
    if (it == subscribeCompleteMapCb_.end()) { return; }

    subscribeCompleteMapCb_.erase(it);

    if (subscribeCompleteMapCb_.empty()) onSubscribeComplete_.disconnect();
}

void MicroServiceClient::unsubscribeAll()
{
    FunctionLog(MicroServiceClientLog);

    CompanEdgeProtocol::ServerMessage msg;

    CompanEdgeProtocol::VsUnsubscribe* pVsUnsubscribe = msg.mutable_vsunsubscribe();
    pVsUnsubscribe->set_sequenceno(++seqNo_);

    insertWriteQueue(std::move(msg));
}

void MicroServiceClient::onClientConnected()
{
    FunctionLog(MicroServiceClientLog);

    connected_ = true;

    boost::asio::post(ctx_, std::bind(&MicroServiceClient::writeFromQueue, this));
}

void MicroServiceClient::onClientDisconnected()
{
    FunctionLog(MicroServiceClientLog);

    connected_ = false;

    if (!autoReconnect_) return;

    /// If the WS server dies and we do a restart anyways
    while (!requestQueue_.empty()) requestQueue_.pop();

    // re-subscribe to the logger
    {
        CompanEdgeProtocol::ServerMessage msg;
        msg.mutable_vsgetvalue()->set_sequenceno(++seqNo_);

        msg.mutable_vsgetvalue()->set_id(LoggerFactory::LoggerId);

        insertWriteQueue(std::move(msg));
    }

    // Walks the registered list of CompanLogger elements (sources)
    // to recreate their values on the ValueStoreServer
    {
        CompanEdgeProtocol::ServerMessage msg;
        CompanEdgeProtocol::ValueChanged* valueChanged = msg.mutable_valuechanged();

        for (auto& source : CompanLoggerController::get()->sources()) {

            // The logger's main key element
            ValueId loggerKey(appName_, source->name());

            VariantValue::Ptr valuePtr = ws_.get(ValueId(LoggerFactory::LoggerId, loggerKey));
            if (valuePtr == nullptr) continue;

            // make the fake value look like the real value
            CompanEdgeProtocol::Value addToValue;
            addToValue.set_id(LoggerFactory::LoggerId);
            addToValue.set_type(CompanEdgeProtocol::ContainerAddTo);
            addToValue.mutable_addtocontainer()->set_key(loggerKey);

            // add the entry
            *valueChanged->add_value() = std::move(addToValue);

            // follow up with a set
            *valueChanged->add_value() = valuePtr->get();
        }

        insertWriteQueue(std::move(msg));
    }

    {
        // Now, re-subscribe for all the values

        // first, we send what our current information is
        CompanEdgeProtocol::ServerMessage updateMsg;
        CompanEdgeProtocol::ValueChanged* valueChanged = updateMsg.mutable_valuechanged();

        // then, we re-subscribe for information
        CompanEdgeProtocol::ServerMessage subscribeMsg;
        CompanEdgeProtocol::VsSubscribe* vsSubscribe = subscribeMsg.mutable_vssubscribe();

        vsSubscribe->set_sequenceno(++seqNo_);

        // we might be reconnection soon - we'll need to resubscribe
        ws_.visitValues([&valueChanged, &vsSubscribe](VariantValue::Ptr const& valuePtr) {
            CompanEdgeProtocol::Value value = valuePtr->get();

            vsSubscribe->add_ids(value.id());

            if (!ValueTypeTraits::isValid(value)) return;

            *valueChanged->add_value() = std::move(value);
        });

        ++seqNo_;
        insertWriteQueue(std::move(updateMsg));
        insertWriteQueue(std::move(subscribeMsg));
    }
}

void MicroServiceClient::insertWriteQueue(CompanEdgeProtocol::ServerMessage&& msg)
{
    if (requestQueue_.empty()) boost::asio::post(ctx_, std::bind(&MicroServiceClient::writeFromQueue, this));

    requestQueue_.push(std::move(msg));
}

void MicroServiceClient::writeFromQueue()
{
    if (!connected_ || requestQueue_.empty()) return;

    messageHandler_->write(requestQueue_.front());

    requestQueue_.pop();

    boost::asio::post(ctx_, std::bind(&MicroServiceClient::writeFromQueue, this));
}

void MicroServiceClient::subscribeComplete(CompanEdgeProtocol::VsResult const& vsResult)
{
    FunctionLog(MicroServiceClientLog);

    auto it = subscribeCompleteMapCb_.find(vsResult.sequenceno());
    if (it == subscribeCompleteMapCb_.end()) { return; }

    // call the designated callback
    if (it->second) it->second(vsResult.status() == CompanEdgeProtocol::VsResult::success);
    subscribeCompleteMapCb_.erase(it);

    if (subscribeCompleteMapCb_.empty()) onSubscribeComplete_.disconnect();
}

void MicroServiceClient::setStartupCompleted()
{
    FunctionLog(MicroServiceClientLog);

    MicroservicesValueIds::Microservices microservice(ws_, MicroservicesValueIds::BaseValueId, appName_);

    if (!microservice.isValid()) {
        WarnLog(MicroServiceClientLog) << "Not subscribed to " << ValueId(MicroservicesValueIds::BaseValueId, appName_)
                                       << std::endl;
        return;
    }

    microservice.status->startupComplete->set(true);
}

void MicroServiceClient::onMicroserviceSubcriptionComplete(bool const completed)
{
    if (!completed) return;

    MicroservicesValueIds::Microservices microservice(ws_, MicroservicesValueIds::BaseValueId, appName_);

    if (!microservice.isValid()) {
        WarnLog(MicroServiceClientLog) << "Not subscribed to " << ValueId(MicroservicesValueIds::BaseValueId, appName_)
                                       << std::endl;
        return;
    }

    microservice.status->pid->set(getpid());
}

void MicroServiceClient::clientConnection(AsioConnectionBase::Ptr client)
{
    if (client == nullptr) return;

    client_ = client;

    connectionManager_ = std::make_shared<ClientConnectionManager>(ctx_, client_);
    connectionManager_->start();

    clientConnectedConnection_ = client_->connectOnConnection(std::bind(&MicroServiceClient::onClientConnected, this));
    clientDisconnectedConnection_ =
            client_->connectOnDisconnection(std::bind(&MicroServiceClient::onClientDisconnected, this));

    client_->connect();
}
