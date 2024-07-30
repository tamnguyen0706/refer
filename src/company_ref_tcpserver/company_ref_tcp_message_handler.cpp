/**
 Copyright © 2023 COMPAN REF
 @file company_ref_tcp_message_handler.cpp
 @brief Tcp -> Uds client Message bridge
 */
#include "company_ref_tcp_message_handler.h"
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger CompanEdgeTcpMessageHandlerLog("tcp.msg.handler", LogLevel::Information);

CompanEdgeTcpMessageHandler::CompanEdgeTcpMessageHandler(
        boost::asio::io_context& ctx,
        std::string const& udsPath,
        uint32_t const connectonId)
    : udsHandler_(std::make_shared<CompanEdgeUdsMessageHandler>())
    , udsClient_(std::make_shared<AsioUdsConnection>(ctx, connectonId, udsHandler_, udsPath))
{
    FunctionLog(CompanEdgeTcpMessageHandlerLog);
    udsClient_->connect();
}

CompanEdgeTcpMessageHandler::~CompanEdgeTcpMessageHandler()
{
    FunctionLog(CompanEdgeTcpMessageHandlerLog);
}

void CompanEdgeTcpMessageHandler::start()
{
    FunctionLog(CompanEdgeTcpMessageHandlerLog);
    udsHandler_->start();

    udsSendConnection_ = udsHandler_->connectSendPassThruData(
            std::bind(&CompanEdgeTcpMessageHandler::passthruData, shared_from_this(), std::placeholders::_1));
}

void CompanEdgeTcpMessageHandler::stop()
{
    FunctionLog(CompanEdgeTcpMessageHandlerLog);

    udsSendConnection_.reset();

    udsHandler_->stop();
    udsClient_->close();
}

void CompanEdgeTcpMessageHandler::recvData(BufferType&& buffer)
{
    FunctionLog(CompanEdgeTcpMessageHandlerLog);
    if (!udsHandler_->sendData(std::move(buffer))) ErrorLog(CompanEdgeTcpMessageHandlerLog) << "Whoops" << std::endl;
}

void CompanEdgeTcpMessageHandler::passthruData(BufferType&& buffer)
{
    FunctionLog(CompanEdgeTcpMessageHandlerLog);
    if (!sendData(std::move(buffer))) ErrorLog(CompanEdgeTcpMessageHandlerLog) << "Whoops" << std::endl;
}
