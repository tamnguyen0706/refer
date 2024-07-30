/**
 Copyright © 2023 COMPAN REF
 @file company_ref_boost_ws_handler_factory.cpp
 @brief VariantValueStore message handler factory
 */

#include "company_ref_boost_ws_handler_factory.h"
#include "company_ref_boost_ws_message_handler.h"

using namespace Compan::Edge;

CompanEdgeBoostWsMessageHandlerFactory::CompanEdgeBoostWsMessageHandlerFactory(
        boost::asio::io_context& ctx,
        VariantValueStore& ws,
        DmoContainer& dmo)
    : ctx_(ctx)
    , ws_(ws)
    , dmo_(dmo)
{
}

CompanEdgeBoostMessageHandler::Ptr CompanEdgeBoostWsMessageHandlerFactory::makeHandler(uint32_t const connectionId)
{
    return std::make_shared<CompanEdgeBoostWsMessageHandler>(ctx_, ws_, dmo_, connectionId);
}
