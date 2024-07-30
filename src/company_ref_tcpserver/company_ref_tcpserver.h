/**
 Copyright © 2023 COMPAN REF
 @file company_ref_tcpserver.h
 @brief Tcp Server interface object
 */
#ifndef __company_ref_TCPSERVER_H__
#define __company_ref_TCPSERVER_H__

#include <company_ref_asio/company_ref_asio_tcp_server.h>
#include <company_ref_microservice/company_ref_microservice_client.h>
#include <boost/asio/io_context.hpp>

#include "company_ref_tcp_handler_factory.h"

namespace Compan{
namespace Edge {

/*!
 * Main interface for controller the TCP Server for CompanController
 *
 */
class TcpServer {
public:
    TcpServer(
            boost::asio::io_context& ioContext,
            std::string const& udsPath,
            std::string const& host,
            std::string const& port);
    virtual ~TcpServer();

    void stopServer();

protected:
    void startServer();

private:
    boost::asio::io_context& ioContext_; //!< Main execution context
    MicroServiceClient client_;          //!< Microservices client
    std::string host_;
    std::string port_;

    CompanEdgeTcpMessageHandlerFactory handlerFactory_;

    std::unique_ptr<AsioTcpServer> server_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_TCPSERVER_H__
