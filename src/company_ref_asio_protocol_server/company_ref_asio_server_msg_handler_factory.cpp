/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_server_msg_handler_factory.cpp
 @brief ServerProtocolSerializer MsgHandler factory
 */

#include "company_ref_asio_server_msg_handler_factory.h"
#include "company_ref_asio_server_protocol_serializer.h"

using namespace Compan::Edge;

ServerMsgHandlerFactory::ServerMsgHandlerFactory(boost::asio::io_context& ctx, VariantValueStore& ws, DmoContainer& dmo)
    : AsioMsgHandlerFactory()
    , ctx_(ctx)
    , ws_(ws)
    , dmo_(dmo)
{
}

ServerMsgHandlerFactory::~ServerMsgHandlerFactory() = default;

AsioMsgHandlerPtr ServerMsgHandlerFactory::make(uint32_t const connectionId)
{
    return std::make_shared<ServerProtocolSerializer>(ctx_, ws_, dmo_, wsContainerMutex_, connectionId);
}
