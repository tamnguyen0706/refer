/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_server_protocol_serializer.h
 @brief Server Protocol Serializer MsgHandler
 */
#ifndef __company_ref_ASIO_SERVER_PROTOCOL_SERIALIZER_H__
#define __company_ref_ASIO_SERVER_PROTOCOL_SERIALIZER_H__

#include "company_ref_asio_server_protocol_handler.h"

#include <company_ref_asio/company_ref_asio_msg_handler.h>

#include <functional>
#include <memory>
#include <string>

#include <boost/asio/io_context_strand.hpp>

// namespace boost {
// namespace asio {
// class io_context;
// class strand;
// } // namespace asio
// } // namespace boost

namespace CompanEdgeProtocol {
class ServerMessage;
class ClientMessage;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {
template <typename>
struct AecCallbacks;

class VariantValueStore;
class DmoContainer;
class ServerProtocolHandler;

using ClientMessagePtr = std::shared_ptr<CompanEdgeProtocol::ClientMessage>;
using ServerMessagePtr = std::shared_ptr<CompanEdgeProtocol::ServerMessage>;

/*!
 *
 */
class ServerProtocolSerializer : public AsioMsgHandler, public std::enable_shared_from_this<ServerProtocolSerializer> {
public:
    using Ptr = std::shared_ptr<ServerProtocolSerializer>;

    ServerProtocolSerializer(
            boost::asio::io_context& ctx,
            VariantValueStore& variantValueStore,
            DmoContainer& dmo,
            std::mutex&,
            uint32_t connectionId);

    virtual ~ServerProtocolSerializer();

    virtual void start();

    virtual void stop();

    /// Called when data has been received by a connection
    virtual void recvData(BufferType&&);

    void onDecodeServerMessage(char const* data, size_t const len);
    void onEncodeClientMessage(ClientMessagePtr rspMsg);

private:
    boost::asio::io_context& ctx_;
    uint32_t connectionId_;

    ServerProtocolHandler::Ptr serverProtocolHandler_;
    SignalScopedConnection onEncodeClientMessage_;

    std::mutex bufferLock_;
    std::string buffer_;

    bool bNewFraming_;
    std::unique_ptr<AecCallbacks<std::string>> pAecCallbacks_; //!< Parser callbacks
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_SERVER_PROTOCOL_SERIALIZER_H__
