/**
 Copyright © 2024 COMPAN REF
 @file company_ref_uds_message_handler.cpp
 @brief -*-*-*-*-*-
 */
#include "company_ref_uds_message_handler.h"
#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger CompanEdgeUdsMessageHandlerLog("uds.msg.handler", LogLevel::Information);

CompanEdgeUdsMessageHandler::CompanEdgeUdsMessageHandler()
{
    FunctionLog(CompanEdgeUdsMessageHandlerLog);
}

void CompanEdgeUdsMessageHandler::start()
{
    FunctionLog(CompanEdgeUdsMessageHandlerLog);
}

void CompanEdgeUdsMessageHandler::stop()
{
    FunctionLog(CompanEdgeUdsMessageHandlerLog);
}

void CompanEdgeUdsMessageHandler::recvData(BufferType&& buffer)
{
    FunctionLog(CompanEdgeUdsMessageHandlerLog);
    SendConnection sendFunction = sendPassThruFunction_.lock();
    if (!sendFunction) {
        WarnLog(CompanEdgeUdsMessageHandlerLog) << "Unbound send function" << std::endl;
        return;
    }

    try {
        (*sendFunction)(std::move(buffer));
    } catch (std::bad_function_call& ec) {
        ErrorLog(CompanEdgeUdsMessageHandlerLog) << "sendData: " << ec.what() << std::endl;
    }
}

AsioMsgHandler::SendConnection CompanEdgeUdsMessageHandler::connectSendPassThruData(SendFunction const& cb)
{
    SendConnection sendFunction = std::make_shared<SendFunction>(cb);
    sendPassThruFunction_ = sendFunction;
    return sendFunction;
}
