/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_dispatcher_asio.h
 @brief Log Message Sink Boost.ASIO Dispatcher
 */
#ifndef __Compan_LOGGER_DISPATCHER_ASIO_H__
#define __Compan_LOGGER_DISPATCHER_ASIO_H__

#include "Compan_logger_dispatcher.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>

namespace Compan{
namespace Edge {
/*!
 * @brief Boost.ASIO dispatcher
 *
 * Uses asio post operation to consume a queue.
 *
 * Wraps the incoming io_context in a strand to guarantee
 * serialization of messages are in order
 */
class CompanLoggerDispatcherAsio : public CompanLoggerDispatcher {
public:
    CompanLoggerDispatcherAsio(boost::asio::io_context& ctx);
    virtual ~CompanLoggerDispatcherAsio();
    /// Pushes a LogMessage into the dispatch queue
    virtual void push(LogMessage&& msg);

protected:
    void processQueue();

private:
    boost::asio::io_context::strand ctx_;
    std::deque<LogMessage> msgQueue_;
};
} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_DISPATCHER_ASIO_H__