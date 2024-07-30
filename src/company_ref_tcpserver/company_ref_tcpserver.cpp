/**
 Copyright © 2023 COMPAN REF
 @file company_ref_tcpserver.cpp
 @brief Tcp Server interface object
 */
#include "company_ref_tcpserver.h"

#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {
CompanLogger TcpServerLog("tcp.server", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

TcpServer::TcpServer(
        boost::asio::io_context& ioContext,
        std::string const& udsPath,
        std::string const& host,
        std::string const& port)
    : ioContext_(ioContext)
    , client_(ioContext_, "TcpServer", udsPath)
    , host_(host)
    , port_(port)
    , handlerFactory_(ioContext, udsPath)
{
    FunctionLog(TcpServerLog);

    client_.clientConnection()->connectOnConnection(std::bind(&TcpServer::startServer, this));
    client_.clientConnection()->connectOnDisconnection(std::bind(&TcpServer::stopServer, this));
}

TcpServer::~TcpServer()
{
    FunctionLog(TcpServerLog);

    if (server_) { server_->close(); }
}

void TcpServer::startServer()
{
    FunctionLog(TcpServerLog);

    // Start Tcp connection
    boost::system::error_code ec;
    boost::asio::ip::address::from_string(host_, ec);
    if (ec.value() != 0) {
        // Provided IP address is invalid. Breaking execution.
        ErrorLog(TcpServerLog) << "Failed to parse the IP address " << host_ << ":" << port_ << ", ec=" << ec.value()
                               << " : " << ec.message() << std::endl;
        return;
    }

    server_ = std::make_unique<AsioTcpServer>(ioContext_, handlerFactory_, host_, port_);

    if (!server_) {
        ErrorLog(TcpServerLog) << "TcpServer failed to start" << std::endl;
        return;
    }

    DebugLog(TcpServerLog) << "Server is running, Ip: " << host_ << ":" << port_ << std::endl;
}

void TcpServer::stopServer()
{
    FunctionLog(TcpServerLog);

    if (server_) { server_->close(); }
}
