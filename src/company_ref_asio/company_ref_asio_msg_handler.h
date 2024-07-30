/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_msg_handler.h
 @brief Message Handler class
 */
#ifndef __company_ref_ASIO_MSG_HANDLER_H__
#define __company_ref_ASIO_MSG_HANDLER_H__

#include <functional>
#include <memory>
#include <vector>

#include <boost/asio/buffer.hpp>

namespace Compan{
namespace Edge {

/*!
 * Send/Recv message handler for a connection
 *
 * For this handler to be able to transmit data,
 * you must std::bind a function using connectSendData
 * and hold onto that object (SendConnection).
 *
 * This behaves similarly to a Signal, except this
 * does not async::post the call, it is a direct
 * call back.
 *
 * If the SendConnection object goes out of scope,
 * the handler's sendData will not transmit data
 *
 * isConnected can be used by the recvData function
 * to know if it should consume the data or not.
 *
 */
class AsioMsgHandler {
public:
    using BufferType = std::vector<uint8_t>;
    using SendFunction = std::function<void(BufferType&&)>;
    using SendConnection = std::shared_ptr<SendFunction>;

    using Ptr = std::shared_ptr<AsioMsgHandler>;

    virtual ~AsioMsgHandler();

    /// Setup required objects for the handler
    virtual void start() = 0;

    /// Tear down required objects for the handler
    virtual void stop() = 0;

    /// Called when data has been received by a connection
    virtual void recvData(BufferType&&) = 0;

    /// Used to send data to the connection
    bool sendData(BufferType&&);

    /// "slot" for sending data to the Connection object
    SendConnection connectSendData(SendFunction const& f);

    /// Returns true if the "slot" is connected
    bool isConnected() const;

private:
    std::weak_ptr<SendFunction> sendFunction_;
};

inline bool AsioMsgHandler::isConnected() const
{
    return (bool)(sendFunction_.lock());
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_MSG_HANDLER_H__
