/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_connection.h
 @brief Agnostic Socket type connection class
 */
#ifndef __company_ref_ASIO_CONNECTION_H__
#define __company_ref_ASIO_CONNECTION_H__

#include <company_ref_utils/company_ref_signals.h>

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/io_context_strand.hpp>

#include <memory>

namespace Compan{
namespace Edge {

class AsioMsgHandler;
using AsioMsgHandlerPtr = std::shared_ptr<AsioMsgHandler>;

/*!
 * Agnostic Asio connection object, can be used to service either
 * a Server or Client connection
 */
class AsioConnectionBase {
public:
    using Ptr = std::shared_ptr<AsioConnectionBase>;

    enum ConnectionType { Client, Server };

    /// Signal for connection/disconnection notification
    using ConnectionSignal = SignalAsio<void(uint32_t const)>;

    AsioConnectionBase(boost::asio::io_context& ctx, uint32_t const connectionId, ConnectionType const connectionType);

    virtual ~AsioConnectionBase() = default;

    /// Connects
    virtual void connect() = 0;

    /// Stops servicing data
    virtual void close() = 0;

    /// Checks if connection is open
    virtual bool isConnected() = 0;

    /// Returns the connection id
    uint32_t connectionId() const;

    ConnectionType connectionType() const;

    /// Slot for OnConnection notifications
    SignalConnection connectOnConnection(ConnectionSignal::SlotType const&);

    /// Slot for OnDisconnection notifications
    SignalConnection connectOnDisconnection(ConnectionSignal::SlotType const&);

protected:
    /// Returns common identity information for the logger
    std::string loggerIdentity();

protected:
    static std::vector<std::string> const ConnectionTypeStr;

protected:
    boost::asio::io_context::strand readerStrand_; //!< used by doRead
    uint32_t const connectionId_;
    ConnectionType connectionType_;
    ConnectionSignal onConnected_;
    ConnectionSignal onDisconnected_;
};

template <typename T>
class AsioConnection : public AsioConnectionBase, public std::enable_shared_from_this<AsioConnection<T>> {
public:
    using Ptr = std::shared_ptr<AsioConnection<T>>;

    using BufferType = std::vector<uint8_t>;
    using SocketType = boost::asio::basic_stream_socket<T>;
    using EndPointType = typename SocketType::endpoint_type;

    /// Ctor for a client connection, socket is self-managed
    AsioConnection(boost::asio::io_context& ctx, uint32_t const connId, AsioMsgHandlerPtr msgHandler);

    /// Ctor for a server connection, created by a listen
    AsioConnection(
            boost::asio::io_context& ctx,
            uint32_t const connId,
            AsioMsgHandlerPtr msgHandler,
            SocketType socket);

    virtual ~AsioConnection();

    /// Connects
    virtual void connect();

    /// Stops servicing data
    virtual void close();

    /// Checks if connection is open
    virtual bool isConnected();

    /// Returns the connection id
    uint32_t connectionId() const;

    /// Returns the remote side endpoint
    EndPointType& endpoint();

    /// Starts servicing data
    void onConnect(boost::system::error_code const&);

protected:
    void doClose();

    void doRead();
    void onRead(boost::system::error_code const&, std::size_t);

    void doSend(BufferType&&);

protected:
    SocketType socket_;
    EndPointType endPoint_;

private:
    AsioMsgHandlerPtr dataHandler_;

    using SendFunction = std::function<void(BufferType&&)>;
    using SendConnection = std::shared_ptr<SendFunction>;

    std::mutex sendLock_;
    SendConnection onSend_; //!< Token for dataHandler's sending

    BufferType recvBuffer_;

    std::atomic<bool> doClose_;
};

inline uint32_t AsioConnectionBase::connectionId() const
{
    return connectionId_;
}

inline AsioConnectionBase::ConnectionType AsioConnectionBase::connectionType() const
{
    return connectionType_;
}

inline SignalConnection AsioConnectionBase::connectOnConnection(ConnectionSignal::SlotType const& cb)
{
    return onConnected_.connect(cb);
}

inline SignalConnection AsioConnectionBase::connectOnDisconnection(ConnectionSignal::SlotType const& cb)
{
    return onDisconnected_.connect(cb);
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_CONNECTION_H__
