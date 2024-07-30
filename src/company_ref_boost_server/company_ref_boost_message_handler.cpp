/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_message_handler.cpp
  @brief CompanEdge Boost - message handler
*/

#include "company_ref_boost_message_handler.h"

using namespace Compan::Edge;

CompanEdgeBoostMessageHandler::CompanEdgeBoostMessageHandler(boost::asio::io_context& ctx, uint32_t const connectionId)
    : connectionId_(connectionId)
    , onClientMessageSignal_(ctx)
{
}

void CompanEdgeBoostMessageHandler::disconnect()
{
    onClientMessageSignal_.disconnectAll();
}
