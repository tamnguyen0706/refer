/**
 Copyright © 2023 COMPAN REF
 @file company_ref_tcp_handler_factory.cpp
 @brief Factory creator for Message bridge
 */
#include "company_ref_tcp_handler_factory.h"
#include "company_ref_tcp_message_handler.h"
#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger CompanEdgeTcpMessageHandlerFactoryLog("tcp.msg.factory", LogLevel::Information);

CompanEdgeTcpMessageHandlerFactory::CompanEdgeTcpMessageHandlerFactory(
        boost::asio::io_context& ctx,
        std::string const& udsPath)
    : ctx_(ctx)
    , udsPath_(udsPath)
{
    FunctionLog(CompanEdgeTcpMessageHandlerFactoryLog);
}

AsioMsgHandlerPtr CompanEdgeTcpMessageHandlerFactory::make(uint32_t const connectionId)
{
    FunctionLog(CompanEdgeTcpMessageHandlerFactoryLog);
    return std::make_shared<CompanEdgeTcpMessageHandler>(ctx_, udsPath_, connectionId);
}
