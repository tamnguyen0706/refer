/**
  Copyright © 2023 COMPAN REF
  @file test_Compan_logger_sink.cpp
  @brief Test Logger Sink
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Compan_logger_sink_buffered.h"
#include "Compan_logger_sink_cout.h"

#include "Compan_logger.h"

#include <iostream>
#include <sstream>

class GtestOstreamWrapper {
public:
    GtestOstreamWrapper(std::ostream& os)
        : os_(os)
        , tmpStreamBuf_(os.rdbuf())
    {
        os_.rdbuf(tmpStream_.rdbuf());
    }

    virtual ~GtestOstreamWrapper()
    {
        os_.rdbuf(tmpStreamBuf_);

        if (!empty()) os_ << pop() << std::endl;
    }

    bool empty() const
    {
        return tmpStream_.str().empty();
    }

    std::string pop()
    {
        std::string str(tmpStream_.str());

        std::string clearStrm;
        tmpStream_.str(clearStrm);

        return str;
    }

private:
    std::ostream& os_;
    std::streambuf* tmpStreamBuf_;
    std::stringstream tmpStream_;
};

using namespace Compan::Edge;

TEST(CompanLoggerSink, Cout)
{
    CompanLoggerSinkCout sink;
    GtestOstreamWrapper coutWrapper(std::cout);

    CompanLogger testLog("testLog", LogLevel::Debug);

    sink(testLog, "Something?", testLog.logLevel());

    EXPECT_EQ(coutWrapper.pop(), "testLog: Dbg : Something?\n");
}

TEST(CompanLoggerSink, Buffered)
{
    CompanLoggerSinkBuffered sink;

    CompanLogger testLog("testLog", LogLevel::Debug);

    sink(testLog, "Something?", testLog.logLevel());

    EXPECT_EQ(sink.pop(), "testLog: Debug: Something?\n");
}
