/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_dispatcher_asio.cpp
 @brief Log Message Sink Boost.ASIO Dispatcher
 */
#include "Compan_logger_dispatcher_asio.h"

#include <boost/asio/post.hpp>

using namespace Compan::Edge;

CompanLoggerDispatcherAsio::CompanLoggerDispatcherAsio(boost::asio::io_context& ctx)
    : CompanLoggerDispatcher()
    , ctx_(ctx)
{
}

CompanLoggerDispatcherAsio::~CompanLoggerDispatcherAsio()
{
}

void CompanLoggerDispatcherAsio::push(LogMessage&& msg)
{
    // In case we are running the io context in multiple threads
    boost::asio::post(ctx_, [this, msg = std::move(msg)]() {
        std::lock_guard<std::mutex> lock(mutex_);
        msgQueue_.emplace_back(msg);
        if (msgQueue_.size() == 1) boost::asio::post(ctx_, std::bind(&CompanLoggerDispatcherAsio::processQueue, this));
    });
}

void CompanLoggerDispatcherAsio::processQueue()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (msgQueue_.empty()) return;

    LogMessage msg = msgQueue_.front();
    msgQueue_.pop_front();

    dispatchToSinks(msg);

    boost::asio::post(ctx_, std::bind(&CompanLoggerDispatcherAsio::processQueue, this));
}
