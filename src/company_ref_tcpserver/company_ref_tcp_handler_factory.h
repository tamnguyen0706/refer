/**
 Copyright © 2023 COMPAN REF
 @file company_ref_tcp_handler_factory.h
 @brief Factory creator for Message bridge
 */
#ifndef __company_ref_TCP_HANDLER_FACTORY_H__
#define __company_ref_TCP_HANDLER_FACTORY_H__

#include <company_ref_asio/company_ref_asio_msg_handler_factory.h>

#include <boost/asio/io_context.hpp>

namespace Compan{
namespace Edge {

// class AsioMsgHandler;
// using AsioMsgHandlerPtr = std::shared_ptr<AsioMsgHandler>;

/// Factory for Tcp -> Uds client Message bridge
class CompanEdgeTcpMessageHandlerFactory : public AsioMsgHandlerFactory {
public:
    CompanEdgeTcpMessageHandlerFactory(boost::asio::io_context& ctx, std::string const& udsPath);
    virtual ~CompanEdgeTcpMessageHandlerFactory() = default;

    virtual AsioMsgHandlerPtr make(uint32_t const connectionId);

private:
    boost::asio::io_context& ctx_;
    std::string udsPath_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_TCP_HANDLER_FACTORY_H__
