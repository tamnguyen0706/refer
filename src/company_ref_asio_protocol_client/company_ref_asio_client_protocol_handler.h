/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_client_protocol_handler.h
 @brief CompanEdgeProtocol message handler base object
 */
#ifndef __company_ref_ASIO_CLIENT_PROTOCOL_HANDLER_H__
#define __company_ref_ASIO_CLIENT_PROTOCOL_HANDLER_H__

#include <company_ref_utils/company_ref_callbacks.h>
#include <company_ref_utils/company_ref_signals.h>
#include <mutex>

namespace CompanEdgeProtocol {
class ServerMessage;
class ClientMessage;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

using ClientMessagePtr = std::shared_ptr<CompanEdgeProtocol::ClientMessage>;
using ServerMessagePtr = std::shared_ptr<CompanEdgeProtocol::ServerMessage>;

/// Message handler that interfaces with VariantValueStore
class ClientProtocolHandler {
public:
    using Ptr = std::shared_ptr<ClientProtocolHandler>;

    /// Callback function to signal message processing has completed
    using SendCallback = Signal<void(ServerMessagePtr)>;

    /*!
     * @param   Unique connection identifier
     */
    ClientProtocolHandler(uint32_t);
    virtual ~ClientProtocolHandler();

    virtual void disconnect();

    /// Request message handler
    virtual void doHandleMessage(ClientMessagePtr requestMessage);

    /// Request message handler
    virtual void doMessage(CompanEdgeProtocol::ClientMessage const& responseMessage) = 0;

    SignalConnection connectSendCallback(SendCallback::SlotType const& cb);

protected:
    ClientProtocolHandler(ClientProtocolHandler const&) = delete;
    ClientProtocolHandler& operator=(ClientProtocolHandler const&) = delete;

protected:
    uint32_t const connectionId_;
    SendCallback onSendCallback_;
};

inline SignalConnection ClientProtocolHandler::connectSendCallback(SendCallback::SlotType const& cb)
{
    return onSendCallback_.connect(cb);
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_CLIENT_PROTOCOL_HANDLER_H__
