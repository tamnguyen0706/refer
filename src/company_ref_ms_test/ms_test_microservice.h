/**
 Copyright © 2024 COMPAN REF
 @file ms_test_microservice.h
 @brief -*-*-*-*-*-
 */
#ifndef __MS_TEST_MICROSERVICE_H__
#define __MS_TEST_MICROSERVICE_H__

#include <company_ref_boost_client/company_ref_boost_uds_client.h>
#include <company_ref_microservice/company_ref_microservice_client.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_variant.h>

#include <boost/asio/deadline_timer.hpp>

namespace Compan{
namespace Edge {

class TestBase;
using TestBasePtr = std::shared_ptr<TestBase>;

class MsTestMicroSerivce {
public:
    using Ptr = std::unique_ptr<MsTestMicroSerivce>;

    MsTestMicroSerivce(
            boost::asio::io_context& ctx,
            std::string const& clientName,
            int const timeOut,
            std::string const& udsPath);

    MsTestMicroSerivce(
            boost::asio::io_context& ctx,
            std::string const& clientName,
            int const timeOut,
            std::string const& ipAddr,
            std::string const& port);

    virtual ~MsTestMicroSerivce();

    void doSubscribe(ValueId const& valueId);

    void doContainer(ValueId const& valueId);

protected:
    void doShutdownTimer(int const timeOutMs);
    void onShutdownTimer(boost::system::error_code const& error);

    void onLogger(VariantValue::Ptr const);

protected:
    boost::asio::io_context& ctx_;
    boost::asio::deadline_timer shutdownTimer_;
    MicroServiceClient msClient_;
    SignalScopedConnection onLogger_;
    bool forceShutdown_;

    TestBasePtr testPtr_;
};

} // namespace Edge
} // namespace Compan

#endif // __MS_TEST_MICROSERVICE_H__
