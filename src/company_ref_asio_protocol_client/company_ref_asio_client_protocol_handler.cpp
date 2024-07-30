/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_client_protocol_handler.cpp
 @brief CompanEdgeProtocol message handler base object
 */

#include "company_ref_asio_client_protocol_handler.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger ClientProtocolHandlerLog("client.protocol.handler", LogLevel::Information);

ClientProtocolHandler::ClientProtocolHandler(uint32_t connectionId)
    : connectionId_(connectionId)
{
    FunctionArgLog(ClientProtocolHandlerLog) << __FUNCTION__ << " [" << connectionId_ << "]" << std::endl;
}

ClientProtocolHandler::~ClientProtocolHandler()
{
    FunctionArgLog(ClientProtocolHandlerLog) << "[" << connectionId_ << "]" << std::endl;
    disconnect();
}

void ClientProtocolHandler::disconnect()
{
}

void ClientProtocolHandler::doHandleMessage(ClientMessagePtr msgPtr)
{
    if (msgPtr) doMessage(*msgPtr);
}
