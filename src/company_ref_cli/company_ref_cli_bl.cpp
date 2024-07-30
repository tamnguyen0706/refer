/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_bl.cpp
  @brief CompanEdge - CLI business logic
*/

#include "company_ref_cli_bl.h"

#include "company_ref_cli_command.h"
#include <company_ref_boost_client/company_ref_boost_client.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

namespace Compan{
namespace Edge {
CompanLogger CompanEdgeCliBlLog("cli.bl", LogLevel::Information);
} // namespace Edge
} // namespace Compan

CompanEdgeCliBl::CompanEdgeCliBl(
        boost::asio::io_context& ioCtx,
        CompanEdgeBoostClientBase::Ptr client,
        CommandQueue&& commandQueue)
    : ctx_(ioCtx)
    , client_(client)
    , commandQueue_(std::move(commandQueue))
    , clientConnectedConnection_(
              client_->connectClientConnectedListener(std::bind(&CompanEdgeCliBl::onClientConnected, this)))
    , clientMessageConnection_(client_->connectClientMessageListener(
              std::bind(&CompanEdgeCliBl::onMessageReceived, this, std::placeholders::_1)))
    , forceRead_(false)
    , cmdStatus_(CMD_STATUS_FAIL)
{
    client_->setWantRead(std::bind(&CompanEdgeCliBl::wantRead, this));
}

CompanEdgeCliBl::~CompanEdgeCliBl()
{
}

void CompanEdgeCliBl::onClientConnected()
{
    FunctionLog(CompanEdgeCliBlLog);

    if (commandQueue_.empty()) return;
    processNextCommand();
}

void CompanEdgeCliBl::onMessageReceived(CompanEdgeProtocol::ClientMessage const& msg)
{
    FunctionLog(CompanEdgeCliBlLog);

    if (commandQueue_.empty()) return;

    cmdStatus_ = commandQueue_.front()->process(msg);

    if (CMD_STATUS_IN_PROGRESS == cmdStatus_) { return; }

    if (CMD_STATUS_SUCCESS == cmdStatus_) {
        commandQueue_.erase(commandQueue_.begin());
        return processNextCommand();
    }

    if (CMD_STATUS_FAIL == cmdStatus_) {
        commandQueue_.clear();
        client_->close();
    }
}

CommandStatus CompanEdgeCliBl::exitStatus() const
{
    FunctionLog(CompanEdgeCliBlLog);

    return cmdStatus_;
}

void CompanEdgeCliBl::processNextCommand()
{
    FunctionLog(CompanEdgeCliBlLog);

    if (commandQueue_.empty()) {
        client_->close();
        return;
    }

    CompanEdgeProtocol::ServerMessage serverMessage;
    if (commandQueue_.front()->process(serverMessage)) { client_->write(serverMessage); }

    if (commandQueue_.front()->expectNoResponseMsg()) {
        boost::asio::post(ctx_, std::bind(&CompanEdgeCliBl::checkForNextCommand, this));
    }
}

bool CompanEdgeCliBl::wantRead()
{
    return (forceRead_ || !commandQueue_.empty());
}

void CompanEdgeCliBl::checkForNextCommand()
{
    FunctionLog(CompanEdgeCliBlLog);

    if (commandQueue_.empty()) return;

    // process next command when expects no response message
    if (commandQueue_.front()->expectNoResponseMsg()) {
        commandQueue_.erase(commandQueue_.begin());
        boost::asio::post(ctx_, std::bind(&CompanEdgeCliBl::processNextCommand, this));
    }
}
