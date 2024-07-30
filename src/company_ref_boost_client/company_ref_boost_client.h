/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_client.h
  @brief CompanEdge Boost - client
*/

#ifndef __company_ref_BOOST_CLIENT_company_ref_BOOST_CLIENT_H__
#define __company_ref_BOOST_CLIENT_company_ref_BOOST_CLIENT_H__

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_utils/company_ref_signals.h>

#include <functional>
#include <queue>
#include <string>
#include <vector>

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/io_context.hpp>

#include <memory>

namespace Compan{
namespace Edge {

template <typename>
struct AecCallbacks;

class CompanEdgeBoostClientBase : public std::enable_shared_from_this<CompanEdgeBoostClientBase> {
public:
    using Ptr = std::shared_ptr<CompanEdgeBoostClientBase>;

    /*!
     * @brief Users of this class must provide a fnWantRead function.
     *
     * This function should return true if it is interested in receiving data.
     */
    using fnWantRead = std::function<bool()>;

    /// Signal for when the connection is established
    using ClientConnectedSignal = SignalAsio<void()>;

    /// Signal for when the client has received a CompanEdgeProtocol::ClientMessage
    using ClientMessageSignal = SignalAsio<void(CompanEdgeProtocol::ClientMessage const&)>;

public:
    CompanEdgeBoostClientBase(boost::asio::io_context&);
    virtual ~CompanEdgeBoostClientBase();

    /// Opens a connection
    void connect();

    /// Closes the connection
    void close();

    /// Checks if the connection is valid
    virtual bool isConnected() = 0;

    /// Sets the want read callback function
    void setWantRead(fnWantRead);

    /// Transmits a CompanEdgeProtocol::ServerMessage to the server
    virtual void write(CompanEdgeProtocol::ServerMessage const&);

    boost::asio::io_context& getIoContext();

public:
    /// Connects a listener to receive a Connection notification
    SignalConnection connectClientConnectedListener(ClientConnectedSignal::SlotType const&);

    /// Connects a listener to receive a Connection notification
    SignalConnection connectClientDisconnectedListener(ClientConnectedSignal::SlotType const&);

    /// Connects a listener to receive a CompanEdgeProtocol::ClientMessage notifications
    SignalConnection connectClientMessageListener(ClientMessageSignal::SlotType const&);

protected:
    /// posts a doWrite
    void asyncDoWrite();

    virtual void doConnect() = 0;
    virtual void doCancel() = 0;
    virtual void doClose() = 0;
    virtual void doRead() = 0;
    virtual void doWrite() = 0;

protected:
    void onConnect(boost::system::error_code const&);

    void onRead(boost::system::error_code const&, std::size_t);

    void parseResponseMessage(char const* data, size_t const len);

    void doClientQueue();

protected:
    boost::asio::io_context& ioContext_;
    boost::asio::io_context::strand readerStrand_; //!< used by doRead

    fnWantRead fnWantRead_;
    std::string readBuffer_;
    ClientConnectedSignal onClientConnectedSignal_;
    ClientConnectedSignal onClientDisconnectedSignal_;
    ClientMessageSignal onClientMessageSignal_;

    std::mutex serverMutex_;
    std::queue<CompanEdgeProtocol::ServerMessage> serverQueue_;

    std::mutex clientMutex_;
    std::queue<CompanEdgeProtocol::ClientMessage> clientQueue_;

    bool bNewFraming_;
    std::unique_ptr<AecCallbacks<std::string>> pAecCallbacks_; //!< Parser callbacks
};

template <typename T>
class CompanEdgeBoostClient : public CompanEdgeBoostClientBase {
public:
    using SocketType = boost::asio::basic_stream_socket<T>;
    using EndPointType = typename SocketType::endpoint_type;

public:
    CompanEdgeBoostClient(boost::asio::io_context&);
    virtual ~CompanEdgeBoostClient();

    virtual bool isConnected();

protected:
    virtual void doCancel();
    virtual void doClose();
    virtual void doClientConnected();
    virtual void doRead();
    virtual void doWrite();

    virtual void doConnect();

protected:
    SocketType socket_;
    EndPointType endPoint_;
};
inline boost::asio::io_context& CompanEdgeBoostClientBase::getIoContext()
{
    return ioContext_;
}
inline SignalConnection CompanEdgeBoostClientBase::connectClientConnectedListener(
        ClientConnectedSignal::SlotType const& cb)
{
    return onClientConnectedSignal_.connect(cb);
}
inline SignalConnection CompanEdgeBoostClientBase::connectClientDisconnectedListener(
        ClientConnectedSignal::SlotType const& cb)
{
    return onClientDisconnectedSignal_.connect(cb);
}
inline SignalConnection CompanEdgeBoostClientBase::connectClientMessageListener(ClientMessageSignal::SlotType const& cb)
{
    return onClientMessageSignal_.connect(cb);
}

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_BOOST_CLIENT_company_ref_BOOST_CLIENT_H__*/
