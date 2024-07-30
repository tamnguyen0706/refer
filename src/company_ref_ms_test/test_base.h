/**
 Copyright © 2024 COMPAN REF
 @file test_base.h
 @brief -*-*-*-*-*-
 */
#ifndef __TEST_BASE_H__
#define __TEST_BASE_H__

#include <company_ref_microservice/company_ref_microservice_client.h>

#include <memory>

namespace Compan{
namespace Edge {

class TestBase {
public:
    using Ptr = std::shared_ptr<TestBase>;

    TestBase(boost::asio::io_context& ctx, MicroServiceClient& msClient);
    virtual ~TestBase();

    virtual void start() = 0;
    virtual void stop() = 0;

protected:
    boost::asio::io_context& ctx_;
    MicroServiceClient& msClient_;
};

} // namespace Edge
} // namespace Compan

#endif // __TEST_BASE_H__
