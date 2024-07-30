/**
  Copyright © 2024 COMPAN REF
  @file test_company_ref_asio_msg_handler.cpp
  @brief -*-*-*-*-*-
*/

#include <gmock/gmock.h>

#include <company_ref_asio/company_ref_asio_msg_handler.h>

#include <boost/asio/buffer.hpp>

using namespace Compan::Edge;

class MockAsioMsgHandler : public AsioMsgHandler {
public:
    MockAsioMsgHandler(AsioMsgHandler::BufferType const& sendBuffer)
        : rspBuffer_(sendBuffer)
    {
    }

    virtual void recvData(BufferType&&)
    {
        BufferType sendBuffer = rspBuffer_;
        sendData(std::move(sendBuffer));
    }

    virtual void start()
    {
    }

    virtual void stop()
    {
    }

    BufferType rspBuffer_;
};

class MockConnection {
public:
    MockConnection(AsioMsgHandler::BufferType const& sendBuffer)
        : sendBuffer_(sendBuffer)
    {
    }

    void sendData(AsioMsgHandler::BufferType&& sendBuffer)
    {
        EXPECT_EQ(sendBuffer_, sendBuffer);
    }

    AsioMsgHandler::BufferType sendBuffer_;
};

using BufferType = std::vector<uint8_t>;
using SendFunction = std::function<void(BufferType&&)>;
using SendConnection = std::shared_ptr<SendFunction>;

TEST(AsioMsgHandlerTest, CheckConnection)
{
    AsioMsgHandler::BufferType buffer;

    MockAsioMsgHandler handler(buffer);
    MockConnection connection(buffer);

    SendConnection sendConnection;

    sendConnection = handler.connectSendData(std::bind(&MockConnection::sendData, connection, std::placeholders::_1));
    EXPECT_TRUE(sendConnection);

    sendConnection.reset();
    EXPECT_FALSE(sendConnection);
}

TEST(AsioMsgHandlerTest, CheckSend)
{
    AsioMsgHandler::BufferType rspBuffer({'r', 's', 'p'});

    MockAsioMsgHandler handler(rspBuffer);
    MockConnection connection(rspBuffer);

    SendConnection sendConnection;

    sendConnection = handler.connectSendData(std::bind(&MockConnection::sendData, connection, std::placeholders::_1));

    AsioMsgHandler::BufferType buffer;
    handler.recvData(std::move(buffer));
}

TEST(AsioMsgHandlerTest, CheckSendDisconnect)
{
    AsioMsgHandler::BufferType rspBuffer({'r', 's', 'p'});

    MockAsioMsgHandler handler(rspBuffer);
    MockConnection connection(rspBuffer);

    SendConnection sendConnection;

    sendConnection = handler.connectSendData(std::bind(&MockConnection::sendData, connection, std::placeholders::_1));

    AsioMsgHandler::BufferType buffer;
    handler.recvData(std::move(buffer));

    sendConnection.reset();

    connection = AsioMsgHandler::BufferType();
    buffer = AsioMsgHandler::BufferType();

    handler.recvData(std::move(buffer));
}
