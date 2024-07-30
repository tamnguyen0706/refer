/**
  Copyright © 2024 COMPAN REF
  @file mock_asio_client.h
  @brief -*-*-*-*-*-
*/
#ifndef TESTS_company_ref_MICROSERVICE_MOCK_ASIO_CLIENT_H_
#define TESTS_company_ref_MICROSERVICE_MOCK_ASIO_CLIENT_H_

#include <company_ref_asio_protocol_server/company_ref_asio_server_protocol_handler.h>
#include <company_ref_microservice/company_ref_microservice_message_handler.h>

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_utils/company_ref_weak_bind.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {

CompanLogger MockAsioClientLog("mockClient", LogLevel::Information);

/*!
 *   We use ServerProtocolHandler to emulate connection to a remote WS server
 */
class MockAsioClient : public AsioConnectionBase, public std::enable_shared_from_this<MockAsioClient> {
public:
    using Ptr = std::shared_ptr<MockAsioClient>;

    MockAsioClient(
            boost::asio::io_context& ctx,
            MicroServiceMessageHandler::Ptr clientHandler,
            ServerProtocolHandler::Ptr serverHandler)
        : AsioConnectionBase(ctx, 0, AsioConnectionBase::Client)
        , ctx_(ctx)
        , clientHandler_(clientHandler)
        , serverHandler_(serverHandler)
        , connectState_(false)
    {
        FunctionLog(MockAsioClientLog);

        boost::asio::post(ctx, std::bind(&MockAsioClient::doConnect, this));
    }

    virtual ~MockAsioClient()
    {
        FunctionLog(MockAsioClientLog);
        doClose();
    }

    /// Connects
    virtual void connect()
    {
        FunctionLog(MockAsioClientLog);
        doConnect();
    }

    /// Stops servicing data
    virtual void close()
    {
        FunctionLog(MockAsioClientLog);
        doClose();
    }

    virtual bool isConnected()
    {
        return connectState_;
    }

    virtual void write(CompanEdgeProtocol::ServerMessage const& msg)
    {
        FunctionLog(MockAsioClientLog);
        writeQueue_.push(msg);

        if (writeQueue_.size() == 1) doWrite();
    }

protected:
    virtual void doConnect()
    {
        FunctionLog(MockAsioClientLog);

        onClientMessage_ = serverHandler_->connectSendCallback(
                WeakBind(&MockAsioClient::onClientMessage, shared_from_this(), std::placeholders::_1));

        onServerMessage_ = clientHandler_->connectSendCallback(
                WeakBind(&MockAsioClient::onServerMessage, shared_from_this(), std::placeholders::_1));

        connectState_ = true;
        onConnected_(connectionId_);

        doWrite();
    }

    virtual void doCancel()
    {
        FunctionLog(MockAsioClientLog);
        doClose();
    }

    virtual void doClose()
    {
        FunctionLog(MockAsioClientLog);

        onClientMessage_.disconnect();
        if (serverHandler_) { serverHandler_->disconnect(); }

        onServerMessage_.disconnect();
        if (clientHandler_) { clientHandler_->disconnect(); }

        connectState_ = false;
        onDisconnected_(connectionId_);
    }

    void onClientMessage(ClientMessagePtr rspMsg)
    {
        FunctionLog(MockAsioClientLog);

        if (rspMsg == nullptr) return;

        if (rspMsg->has_valuechanged())
            DebugLog(MockAsioClientLog) << rspMsg->GetTypeName() << "::" << rspMsg->valuechanged() << std::endl;
        if (rspMsg->has_valueremoved())
            DebugLog(MockAsioClientLog) << rspMsg->GetTypeName() << "::" << rspMsg->valueremoved() << std::endl;
        if (rspMsg->has_vsresult())
            DebugLog(MockAsioClientLog) << rspMsg->GetTypeName() << "::" << rspMsg->vsresult() << std::endl;
        if (rspMsg->has_vsmultigetresult())
            DebugLog(MockAsioClientLog) << rspMsg->GetTypeName() << "::" << rspMsg->vsmultigetresult() << std::endl;
        if (rspMsg->has_vsmultisetresult())
            DebugLog(MockAsioClientLog) << rspMsg->GetTypeName() << "::" << rspMsg->vsmultisetresult() << std::endl;
        if (rspMsg->has_vssynccompleted())
            DebugLog(MockAsioClientLog) << rspMsg->GetTypeName() << "::" << rspMsg->vssynccompleted().GetTypeName()
                                        << std::endl;

        if (clientHandler_) clientHandler_->doHandleMessage(rspMsg);
    }

    void onServerMessage(ServerMessagePtr reqMsg)
    {
        FunctionLog(MockAsioClientLog);

        if (reqMsg == nullptr) return;

        if (serverHandler_) serverHandler_->doHandleMessage(reqMsg);
    }

    virtual void doWrite()
    {
        FunctionLog(MockAsioClientLog);

        if (!connectState_) return;

        CompanEdgeProtocol::ServerMessage msg;

        if (writeQueue_.empty()) { return; }

        msg = writeQueue_.front();
        writeQueue_.pop();

        if (msg.has_vssync())
            DebugLog(MockAsioClientLog) << msg.GetTypeName() << "::" << msg.vssync().GetTypeName() << std::endl;
        if (msg.has_valuechanged())
            DebugLog(MockAsioClientLog) << msg.GetTypeName() << "::" << msg.valuechanged() << std::endl;
        if (msg.has_valueremoved())
            DebugLog(MockAsioClientLog) << msg.GetTypeName() << "::" << msg.valueremoved() << std::endl;
        if (msg.has_vssubscribe())
            DebugLog(MockAsioClientLog) << msg.GetTypeName() << "::" << msg.vssubscribe() << std::endl;
        if (msg.has_vsunsubscribe())
            DebugLog(MockAsioClientLog) << msg.GetTypeName() << "::" << msg.vsunsubscribe().GetTypeName() << std::endl;
        if (msg.has_vsgetall())
            DebugLog(MockAsioClientLog) << msg.GetTypeName() << "::" << msg.vsgetall().GetTypeName() << std::endl;
        if (msg.has_vsgetvalue())
            DebugLog(MockAsioClientLog) << msg.GetTypeName() << "::" << msg.vsgetvalue().GetTypeName() << "\t"
                                        << msg.vsgetvalue().id() << std::endl;
        if (msg.has_vssetvalue())
            DebugLog(MockAsioClientLog) << msg.GetTypeName() << "::" << msg.vssetvalue().GetTypeName() << "\t"
                                        << msg.vssetvalue().id() << std::endl;

        serverHandler_->doMessage(msg);

        if (!writeQueue_.empty()) { boost::asio::post(ctx_, WeakBind(&MockAsioClient::doWrite, shared_from_this())); }
    }

private:
    boost::asio::io_context& ctx_;
    MicroServiceMessageHandler::Ptr clientHandler_;

    // This fakes the connection to the WS Server
    ServerProtocolHandler::Ptr serverHandler_;

    SignalScopedConnection onClientMessage_;
    SignalScopedConnection onServerMessage_;

    std::queue<CompanEdgeProtocol::ServerMessage> writeQueue_;

    bool connectState_;
};

} // namespace Edge
} // namespace Compan

#endif /* TESTS_company_ref_MICROSERVICE_MOCK_ASIO_CLIENT_H_ */
