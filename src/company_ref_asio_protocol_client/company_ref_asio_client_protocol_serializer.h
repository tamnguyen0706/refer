/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_client_protocol_serializer.h
 @brief Client Protocol Serializer MsgHandler
 */
#ifndef __company_ref_ASIO_CLIENT_PROTOCOL_SERIALIZER_H__
#define __company_ref_ASIO_CLIENT_PROTOCOL_SERIALIZER_H__

#include <company_ref_asio/company_ref_asio_msg_handler.h>

#include <company_ref_utils/company_ref_signals.h>

#include <functional>
#include <memory>
#include <string>

#include <boost/asio/io_context_strand.hpp>

namespace CompanEdgeProtocol {
class ServerMessage;
class ClientMessage;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {
template <typename>
struct AecCallbacks;

class ClientProtocolHandler;
using ClientProtocolHandlerPtr = std::shared_ptr<ClientProtocolHandler>;

using ClientMessagePtr = std::shared_ptr<CompanEdgeProtocol::ClientMessage>;
using ServerMessagePtr = std::shared_ptr<CompanEdgeProtocol::ServerMessage>;

/*!
 *
 */
class ClientProtocolSerializer : public AsioMsgHandler, public std::enable_shared_from_this<ClientProtocolSerializer> {
public:
    using Ptr = std::shared_ptr<ClientProtocolSerializer>;

    ClientProtocolSerializer(
            boost::asio::io_context& ctx,
            ClientProtocolHandlerPtr clientProtocolHandler,
            uint32_t connectionId);

    virtual ~ClientProtocolSerializer();

    virtual void start();

    virtual void stop();

    /// Called when data has been received by a connection
    virtual void recvData(BufferType&&);

    void onDecodeClientMessage(char const* data, size_t const len);
    void onEncodeServerMessage(ServerMessagePtr rspMsg);

private:
    boost::asio::io_context& ctx_;
    uint32_t connectionId_;

    ClientProtocolHandlerPtr clientProtocolHandler_;
    SignalScopedConnection onEncodeServerMessage_;

    std::mutex bufferLock_;
    std::string buffer_;

    bool bNewFraming_;
    std::unique_ptr<AecCallbacks<std::string>> pAecCallbacks_; //!< Parser callbacks
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_CLIENT_PROTOCOL_SERIALIZER_H__
