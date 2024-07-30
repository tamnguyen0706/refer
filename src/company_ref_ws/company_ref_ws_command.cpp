/**
  Copyright © 2023 COMPAN REF
  @file company_ref_ws_command.cpp
  @brief CompanEdge - WS command
*/

#include "company_ref_ws_command.h"

using namespace Compan::Edge;

CompanEdgeWsCommand::CompanEdgeWsCommand()
{
}

CompanEdgeWsCommand::~CompanEdgeWsCommand() = default;

void CompanEdgeWsCommand::setCommandCb(CommandCompleteCb const& cb)
{
    cb_ = cb;
};
