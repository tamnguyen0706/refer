/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_container.h
  @brief CompanEdge - CLI AddTo RemoveFrom Container command
*/

#ifndef __company_ref_CLI_CONTAINER_H__
#define __company_ref_CLI_CONTAINER_H__

#include "company_ref_cli_command.h"
#include "company_ref_cli_command_status.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <boost/asio.hpp>

namespace Compan{
namespace Edge {

class CompanEdgeProcessMonitorBase;

class CompanEdgeCliContainer : public CompanEdgeCliCommand {
public:
    CompanEdgeCliContainer(
            boost::asio::io_context& ioCtx,
            const std::string& id,
            const std::string& containerKey,
            const bool condition, // addToContainer:true, removeFromContainer:false
            ProtocolValueMap& savedValues);
    virtual ~CompanEdgeCliContainer();

    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    void processQueue();

private:
    const std::string id_;
    const std::string containerKey_;
    boost::asio::io_context& ctx_;
    const bool condition_;
    const std::string fqvid_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_CLI_CONTAINER_H__