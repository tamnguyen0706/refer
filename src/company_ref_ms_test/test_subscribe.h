/**
 Copyright © 2024 COMPAN REF
 @file test_subscribe.h
 @brief -*-*-*-*-*-
 */
#ifndef __TEST_SUBSCRIBE_H__
#define __TEST_SUBSCRIBE_H__

#include "test_base.h"

#include <company_ref_utils/company_ref_function_time_profiler.h>

#include <sstream>

namespace Compan{
namespace Edge {

class TestSubscribe : public TestBase, public std::enable_shared_from_this<TestSubscribe> {
public:
    using Ptr = std::shared_ptr<TestSubscribe>;

    TestSubscribe(boost::asio::io_context& ctx, MicroServiceClient& msClient, ValueId const& valueId);

    virtual ~TestSubscribe();

    virtual void start();
    virtual void stop();

protected:
    void onSubscribeComplete();

private:
    ValueId const& valueId_;

    bool subscribeCompleted_;
    int subscription_;

    std::stringstream profileStrm_;
    std::shared_ptr<FunctionTimeProfiler> timeProfile_;
};

} // namespace Edge
} // namespace Compan

#endif // __TEST_SUBSCRIBE_H__
