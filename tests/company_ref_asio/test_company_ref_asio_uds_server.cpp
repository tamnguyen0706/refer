/**
  Copyright © 2024 COMPAN REF
  @file test_company_ref_asio_uds_server.cpp
  @brief Tests derived client/server for UDS sockets
*/

#include <gmock/gmock.h>

#include <company_ref_asio/company_ref_asio_msg_handler.h>
#include <company_ref_asio/company_ref_asio_msg_handler_factory.h>
#include <company_ref_asio/company_ref_asio_uds_connection.h>
#include <company_ref_asio/company_ref_asio_uds_server.h>

#include <Compan_logger/Compan_logger_sink_buffered.h>

using namespace Compan::Edge;

class ServerMsgHandler : public AsioMsgHandler {
public:
    ServerMsgHandler()
        : sent_(false)
    {
    }

    virtual ~ServerMsgHandler()
    {
        EXPECT_TRUE(sent_);
    }

    virtual void start()
    {
    }
    virtual void stop()
    {
    }

    virtual void recvData(BufferType&& buffer)
    {
        sendData(std::move(buffer));
        sent_ = true;
    }
    bool sent_;
};

class PingPongFactory : public AsioMsgHandlerFactory {
public:
    PingPongFactory()
    {
    }

    AsioMsgHandler::Ptr make(uint32_t const)
    {
        return std::make_shared<ServerMsgHandler>();
    }
};

class ClientMsgHandler : public AsioMsgHandler {
public:
    using Ptr = std::shared_ptr<ClientMsgHandler>;

    ClientMsgHandler(BufferType const& checkBuffer)
        : checkBuffer_(checkBuffer)
        , matched_(false)
    {
    }

    virtual ~ClientMsgHandler() = default;

    virtual void start()
    {
    }
    virtual void stop()
    {
    }

    virtual void recvData(BufferType&& buffer)
    {
        matched_ = (buffer == checkBuffer_);
    }

    void sendMessage(BufferType& buffer)
    {
        sendData(std::move(buffer));
    }

    BufferType checkBuffer_;
    bool matched_;
};

class UdsServerTest : public testing::Test {
public:
    UdsServerTest()
        : ctx_()
        , udsPath_("./integrated")
        , buffer_({'h', 'e', 'l', 'l', 'o'})
    {
    }

    virtual ~UdsServerTest()
    {
        EXPECT_TRUE(coutWrapper_.empty());
        if (!coutWrapper_.empty()) std::cout << coutWrapper_.pop() << std::endl;
    }

    boost::asio::io_context ctx_;
    std::string const udsPath_;
    AsioMsgHandler::BufferType buffer_;

    PingPongFactory factory_;

    CompanLoggerSinkBuffered coutWrapper_;
};

TEST_F(UdsServerTest, IntegratedClientDisconnect)
{
    PingPongFactory factory_;
    AsioUdsServer server(ctx_, factory_, udsPath_);

    EXPECT_TRUE(server.empty());

    ClientMsgHandler::Ptr clientHandler(std::make_shared<ClientMsgHandler>(buffer_));
    AsioUdsConnection::Ptr client = std::make_shared<AsioUdsConnection>(ctx_, 0, clientHandler, udsPath_);

    client->connect();

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    clientHandler->sendMessage(buffer_);

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    EXPECT_FALSE(server.empty());

    client->close();

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    EXPECT_TRUE(server.empty());
    EXPECT_TRUE(clientHandler->matched_);
}

TEST_F(UdsServerTest, IntegratedClientReconnect)
{
    AsioUdsServer server(ctx_, factory_, udsPath_);

    EXPECT_TRUE(server.empty());

    ClientMsgHandler::Ptr clientHandler(std::make_shared<ClientMsgHandler>(buffer_));
    AsioUdsConnection::Ptr client = std::make_shared<AsioUdsConnection>(ctx_, 0, clientHandler, udsPath_);
    client->connect();

    EXPECT_EQ(ctx_.poll_one(), 1);

    {
        AsioMsgHandler::BufferType buffer(buffer_);
        clientHandler->sendMessage(buffer);
    }

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    EXPECT_FALSE(server.empty());

    client->close();

    EXPECT_TRUE(clientHandler->matched_);

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    EXPECT_TRUE(server.empty());

    client->connect();
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    EXPECT_FALSE(server.empty());

    {
        AsioMsgHandler::BufferType buffer(buffer_);
        clientHandler->sendMessage(buffer);
    }
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    client->close();

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    EXPECT_TRUE(server.empty());
}

TEST_F(UdsServerTest, IntegratedServerDisconnect)
{
    AsioUdsServer server(ctx_, factory_, udsPath_);

    EXPECT_TRUE(server.empty());

    ClientMsgHandler::Ptr clientHandler(std::make_shared<ClientMsgHandler>(buffer_));
    AsioUdsConnection::Ptr client = std::make_shared<AsioUdsConnection>(ctx_, 0, clientHandler, udsPath_);
    client->connect();

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    clientHandler->sendMessage(buffer_);

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    EXPECT_FALSE(server.empty());

    server.close();

    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);
    EXPECT_EQ(ctx_.poll_one(), 1);

    EXPECT_FALSE(client->isConnected());

    EXPECT_TRUE(server.empty());
    EXPECT_TRUE(clientHandler->matched_);
}
