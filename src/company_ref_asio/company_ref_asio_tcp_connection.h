/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_tcp_connection.h
 @brief Derived Connection class for TCP sockets
 */
#ifndef __company_ref_ASIO_TCP_CONNECTION_H__
#define __company_ref_ASIO_TCP_CONNECTION_H__

#include "company_ref_asio_connection.h"

#include <boost/asio/ip/tcp.hpp>

namespace Compan{
namespace Edge {

class AsioTcpConnection : public AsioConnection<boost::asio::ip::tcp> {
public:
    using Ptr = std::shared_ptr<AsioTcpConnection>;

    using BaseClass = AsioConnection<boost::asio::ip::tcp>;

    AsioTcpConnection(
            boost::asio::io_context& ctx,
            uint32_t const connId,
            AsioMsgHandlerPtr msgHandler,
            BaseClass::SocketType socket);

    AsioTcpConnection(
            boost::asio::io_context& ctx,
            uint32_t const connId,
            AsioMsgHandlerPtr msgHandler,
            std::string const& host,
            std::string const& port);

    virtual ~AsioTcpConnection();

    virtual void connect();

private:
    void doResolve(boost::asio::ip::tcp::resolver::query const&);
    void onResolve(boost::system::error_code const&, boost::asio::ip::tcp::resolver::iterator);
    void doResolveConnect(boost::asio::ip::tcp::resolver::iterator);
    void onResolveConnect(boost::system::error_code const&, boost::asio::ip::tcp::resolver::iterator);

protected:
    std::shared_ptr<AsioTcpConnection> shared_from_this()
    {
        return std::dynamic_pointer_cast<AsioTcpConnection>(BaseClass::shared_from_this());
    }
    std::shared_ptr<AsioTcpConnection const> shared_from_this() const
    {
        return std::dynamic_pointer_cast<AsioTcpConnection const>(BaseClass::shared_from_this());
    }

private:
    boost::asio::ip::tcp::resolver resolver_;

    std::string const host_;
    std::string const port_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_TCP_CONNECTION_H__
