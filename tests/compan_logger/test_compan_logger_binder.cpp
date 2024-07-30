/**
  Copyright © 2023 COMPAN REF
  @file test_Compan_logger_binder.cpp
  @brief Simple CompanLogger Binder test
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Compan_logger.h"
#include "Compan_logger_binder.h"
#include "Compan_logger_controller.h"
#include "Compan_logger_sink_buffered.h"

using namespace Compan::Edge;

class CompanLoggerBinderTest : public CompanLoggerBinder {
public:
    CompanLoggerBinderTest(CompanLogger& logger)
        : CompanLoggerBinder("bindTest", logger)
        , expectLogLevel_(LogLevel::Information)
    {
    }

    virtual void onLogLevel(LogLevel const logLevel)
    {
        EXPECT_EQ(logLevel, expectLogLevel_);
    }

    LogLevel expectLogLevel_;
};

TEST(CompanLoggerBinder, Simple)
{
    CompanLogger testLog("CompanLoggerBinder", LogLevel::Debug);
    CompanLoggerBinderTest binder(testLog);

    testLog.logLevel(LogLevel::Information);
}

TEST(CompanLoggerBinder, AddController)
{
    CompanLogger testLog("CompanLoggerBinder", LogLevel::Debug);

    {
        CompanLoggerBinderTest::Ptr binder(std::make_unique<CompanLoggerBinderTest>(testLog));
        EXPECT_TRUE(CompanLoggerController::get()->addBinder(std::move(binder)));
    }

    testLog.logLevel(LogLevel::Information);
}
