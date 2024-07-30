/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_bl.h
  @brief CompanEdge Connector - CLI business logic
*/

#ifndef __company_ref_CLI_company_ref_CLI_BL_H__
#define __company_ref_CLI_company_ref_CLI_BL_H__

#include "company_ref_cli_command_status.h"
#include <company_ref_boost_client/company_ref_boost_client.h>

#include <boost/asio.hpp>

#include <memory>
#include <vector>

namespace CompanEdgeProtocol {
class ServerMessage;
class ClientMessage;
} // namespace CompanEdgeProtocol
namespace Compan{
namespace Edge {
class CompanEdgeCliCommand;

class CompanEdgeCliBl {
public:
    using CommandPtr = std::unique_ptr<CompanEdgeCliCommand>;
    using CommandQueue = std::vector<CommandPtr>;

public:
    CompanEdgeCliBl(boost::asio::io_context& ioCtx, CompanEdgeBoostClientBase::Ptr, CommandQueue&&);
    virtual ~CompanEdgeCliBl();
    void forceRead(bool);
    void onClientConnected();
    void onMessageReceived(CompanEdgeProtocol::ClientMessage const&);

    CommandStatus exitStatus() const;

private:
    void processNextCommand();
    bool wantRead();
    void checkForNextCommand();

private:
    boost::asio::io_context& ctx_;
    CompanEdgeBoostClientBase::Ptr client_;
    CommandQueue commandQueue_;
    SignalScopedConnection clientConnectedConnection_;
    SignalScopedConnection clientMessageConnection_;
    bool forceRead_;
    CommandStatus cmdStatus_;
};
inline void CompanEdgeCliBl::forceRead(bool forceRead)
{
    forceRead_ = forceRead;
}
} // namespace Edge
} // namespace Compan

#endif /*__company_ref_CLI_company_ref_CLI_BL_H__*/
