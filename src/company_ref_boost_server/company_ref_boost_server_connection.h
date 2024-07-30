/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_server_connection.h
  @brief CompanEdge Boost Server - connection
*/

#ifndef __company_ref_BOOST_SERVER_CONNECTION_H__
#define __company_ref_BOOST_SERVER_CONNECTION_H__

#include "company_ref_boost_handler_factory.h"
#include "company_ref_boost_message_handler.h"

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_utils/company_ref_signals.h>

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>

#include <functional>
#include <memory>
#include <queue>
#include <vector>

namespace Compan{
namespace Edge {

template <typename T>
class CompanEdgeBoostServerConnectionManager;
class AECFrameId;

template <typename>
struct AecCallbacks;

class CompanEdgeBoostMessageHandler;
class VariantValueStore;

// Represents a single connection from a client.
// A connection holds an instance of message handler and reference to variant valuestore
template <typename T>
class CompanEdgeBoostServerConnection : public std::enable_shared_from_this<CompanEdgeBoostServerConnection<T>> {
public:
    using FnWantRead = std::function<bool()>;

    using SocketType = boost::asio::basic_stream_socket<T>;
    using EndPointType = typename SocketType::endpoint_type;

    using ClientMessagePtr = std::shared_ptr<CompanEdgeProtocol::ClientMessage>;

    using Ptr = std::shared_ptr<CompanEdgeBoostServerConnection<T>>;
    using DisconnectSignal = SignalAsio<void(CompanEdgeBoostServerConnection<T>::Ptr)>;

public:
    // class cannot be copied; delete copy constructor and copy assignment
    CompanEdgeBoostServerConnection(CompanEdgeBoostServerConnection const&) = delete;
    CompanEdgeBoostServerConnection& operator=(CompanEdgeBoostServerConnection const&) = delete;

    // Construct a connection with the given socket.
    explicit CompanEdgeBoostServerConnection(
            uint32_t connectionId,
            boost::asio::io_context& ioContext,
            boost::asio::basic_stream_socket<T> socket,
            CompanEdgeBoostMessageHandlerFactory& handlerFactory);

    virtual ~CompanEdgeBoostServerConnection();

    // Start the first asynchronous operation for the connection.
    void start();

    // Stop all asynchronous operations associated with the connection.
    void stop();

    uint32_t getConnectionId();

    EndPointType endpoint();

    /// Sets the completion callback function
    SignalConnection connectDisconnectListener(typename DisconnectSignal::SlotType const&);

protected:
    // Perform an asynchronous read operation.
    void doRead();
    void onRead(boost::system::error_code const& ec, size_t const length);

    void parseRequestMessage(char const* data, size_t const len);
    void parseReadBuffer(std::string const& buffer);
    void doServerQueue();

    void sendClientMessage(ClientMessagePtr rspMsg);
    void sendUnknownFrameId(COMPAN::REF::AECFrameId const& frameId);

    void doWrite();

    void doClose();

private:
    boost::asio::io_context& ioContext_;
    DisconnectSignal onDisconnectSignal_;

    // Socket for the connection.
    SocketType socket_;

    uint32_t connectionId_;

    std::queue<ClientMessagePtr> rspMsgQueue_;
    std::mutex rspMsgMutex_;

    std::mutex serverMutex_;
    std::queue<CompanEdgeProtocol::ServerMessage> serverQueue_;

    // The handler used to process the incoming request and outgoing response
    // ServerMessage is RequestMessage from client to server
    // ClientMessage is ResponseMessage from server to client
    CompanEdgeBoostMessageHandler::Ptr messageHandlerPtr_;

    SignalScopedConnection messageHandlerConnection_;

    std::string readBuffer_; //!< Incoming buffer

    bool bNewFraming_;
    std::unique_ptr<AecCallbacks<std::string>> pAecCallbacks_; //!< Parser callbacks
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_BOOST_SERVER_CONNECTION_H__
