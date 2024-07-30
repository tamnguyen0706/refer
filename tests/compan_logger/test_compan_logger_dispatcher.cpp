/**
  Copyright © 2023 COMPAN REF
  @file test_Compan_logger_dispatcher.cpp
  @brief Test Logger Dispatcher
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Compan_logger.h"
#include "Compan_logger_dispatcher.h"
#include "Compan_logger_dispatcher_asio.h"
#include "Compan_logger_sink_buffered.h"

#include <thread>

using namespace Compan::Edge;

TEST(CompanLoggerDispatcher, Buffered)
{
    CompanLoggerSinkBuffered sink;
    CompanLoggerDispatcher dispatcher;

    CompanLogger testLog("testLog", LogLevel::Debug);

    CompanLoggerDispatcher::LogMessage msg = {testLog, "Something?", testLog.logLevel()};

    dispatcher.dispatchToSinks(std::move(msg));

    EXPECT_EQ(sink.pop(), "testLog: Debug: Something?\n");
}

TEST(CompanLoggerDispatcher, Asio)
{
    boost::asio::io_context ctx;
    CompanLoggerSinkBuffered sink;
    CompanLoggerDispatcherAsio dispatcher(ctx);

    CompanLogger testLog("testLog", LogLevel::Debug);

    dispatcher.push((CompanLoggerDispatcher::LogMessage({testLog, "Message 1", testLog.logLevel()})));
    dispatcher.push((CompanLoggerDispatcher::LogMessage({testLog, "Message 2", testLog.logLevel()})));

    // push thru 5 async posts
    EXPECT_EQ(ctx.poll_one(), 1);
    EXPECT_EQ(ctx.poll_one(), 1);
    EXPECT_EQ(ctx.poll_one(), 1);
    EXPECT_EQ(ctx.poll_one(), 1);
    EXPECT_EQ(ctx.poll_one(), 0);

    EXPECT_EQ(
            sink.pop(),
            "testLog: Debug: Message 1\n"
            "testLog: Debug: Message 2\n");
}

TEST(CompanLoggerDispatcher, AsioThreaded)
{
    boost::asio::io_context ctx;
    CompanLoggerSinkBuffered sink;

    CompanLoggerDispatcherAsio dispatcher(ctx);

    CompanLogger testLog("testLog", LogLevel::Debug);

    std::string baseResult("testLog: Debug: Message ");
    std::string baseMessage("Message ");
    std::stringstream sinkResults;
    for (int count = 0; count < 20; ++count) {
        sinkResults << baseResult << count << std::endl;

        std::stringstream logMessage;

        logMessage << "Message " << count;

        dispatcher.push((CompanLoggerDispatcher::LogMessage({testLog, logMessage.str(), testLog.logLevel()})));
    }

    // push thru two async posts
    std::thread thread1{[&ctx]() { ctx.run(); }};
    std::thread thread2{[&ctx]() { ctx.run(); }};
    thread1.join();
    thread2.join();

    EXPECT_EQ(sink.pop(), sinkResults.str());
}
