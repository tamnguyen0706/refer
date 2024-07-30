/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_invoke.h
  @brief CompanEdge - CLI Invoke command
*/

#ifndef __company_ref_CLI_company_ref_CLI_INVOKE_H__
#define __company_ref_CLI_company_ref_CLI_INVOKE_H__

#include "company_ref_cli_command.h"
#include "company_ref_cli_command_status.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <memory>
#include <queue>

namespace Compan{
namespace Edge {

class CompanEdgeProcessMonitorBase;

class CompanEdgeCliInvoke : public CompanEdgeCliCommand {
public:
    CompanEdgeCliInvoke(
            boost::asio::io_context& ioCtx,
            const std::string& id,
            const std::string& exec,
            const bool onChangedOnly,
            ProtocolValueMap& savedValues);
    virtual ~CompanEdgeCliInvoke();

    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    void processFinished(CompanEdgeProcessMonitorBase*);
    void processQueue();
    bool isInvokableType(CompanEdgeProtocol::Value_Type const);

private:
    const std::string id_;
    const std::string execBinary_;
    boost::asio::io_context& ctx_;
    const bool onChangedOnly_;

    std::shared_ptr<CompanEdgeProcessMonitorBase> processMonitorPtr_;
    std::queue<CompanEdgeProtocol::Value> valueChangedqueue_;
};

} // namespace Edge
} // namespace Compan

#endif