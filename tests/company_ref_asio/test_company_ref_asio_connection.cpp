/**
  Copyright © 2024 COMPAN REF
  @file test_company_ref_asio_connection.cpp
  @brief Test Socket type Agnostic Connection class
*/

#include <company_ref_asio/company_ref_asio_connection.h>
#include <gmock/gmock.h>

#include <Compan_logger/Compan_logger_sink_buffered.h>
#include <boost/asio/local/stream_protocol.hpp>

using namespace Compan::Edge;

TEST(AsioConnection, CtorTest)
{
    CompanLoggerSinkBuffered coutWrapper_;

    boost::asio::io_context ctx;

    using ThisAsioUdsConnection = AsioConnection<boost::asio::local::stream_protocol>;

    ThisAsioUdsConnection::Ptr uds2(new ThisAsioUdsConnection(ctx, 1, nullptr));
    EXPECT_EQ(coutWrapper_.pop(), "asio.connection: Error: missing MsgHandler - 1\n");

    boost::asio::basic_stream_socket<boost::asio::local::stream_protocol> socket(ctx);
    ThisAsioUdsConnection::Ptr uds1(new ThisAsioUdsConnection(ctx, 2, nullptr, std::move(socket)));

    EXPECT_EQ(coutWrapper_.pop(), "asio.connection: Error: missing MsgHandler - 2\n");
}
