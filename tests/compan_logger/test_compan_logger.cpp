/**
  Copyright © 2023 COMPAN REF
  @file test_Compan_logger.cpp
  @brief Simple CompanLogger test
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Compan_logger.h"
#include "Compan_logger_sink_buffered.h"

using namespace Compan::Edge;

TEST(CompanLogger, Simple)
{
    CompanLoggerSinkBuffered sink;
    CompanLogger testLog("x", LogLevel::Debug);

    DebugLog(testLog) << "Something?" << std::endl;

    EXPECT_EQ(sink.pop(), "x: Debug: Something?\n");
}

TEST(CompanLogger, LogLevel)
{
    LogLevel const level(LogLevel::Debug);

    std::stringstream out;
    out << level;

    EXPECT_EQ(out.str(), "Debug");

    LogLevel chkLevel(LogLevel::None);

    {
        std::istringstream in(out.str());

        in >> chkLevel;

        EXPECT_EQ(chkLevel, level);
    }

    {
        std::istringstream in("bad");

        in >> chkLevel;

        EXPECT_EQ(chkLevel, LogLevel::None);
    }
}