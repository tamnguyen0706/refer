/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_msg_handler.cpp
 @brief Message Handler class
 */
#include "company_ref_asio_msg_handler.h"

#include <iostream>
#include <utility>
using namespace Compan::Edge;

AsioMsgHandler::~AsioMsgHandler() = default;

bool AsioMsgHandler::sendData(BufferType&& buffer)
{
    SendConnection sendFunction = sendFunction_.lock();
    if (!sendFunction) return false;

    try {
        (*sendFunction)(std::move(buffer));
        return true;
    } catch (std::bad_function_call& ec) {
        std::cerr << "sendData: " << ec.what() << std::endl;
    }

    return false;
}

AsioMsgHandler::SendConnection AsioMsgHandler::connectSendData(SendFunction const& cb)
{
    SendConnection sendFunction = std::make_shared<SendFunction>(cb);
    sendFunction_ = sendFunction;
    return sendFunction;
}
