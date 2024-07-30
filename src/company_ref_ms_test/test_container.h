/**
 Copyright © 2024 COMPAN REF
 @file test_container.h
 @brief -*-*-*-*-*-
 */
#ifndef __TEST_CONTAINER_H__
#define __TEST_CONTAINER_H__

#include "test_base.h"

#include <company_ref_utils/company_ref_function_time_profiler.h>
#include <random>
#include <time.h>

namespace Compan{
namespace Edge {

struct TestContainerStats {
    TestContainerStats(std::string const& key);

    TestContainerStats(std::string const& key, CompanEdgeProtocol::Value_Type const& action);

    void completed();

    void print(std::ostream&) const;

    std::string key_;
    CompanEdgeProtocol::Value_Type action_;
    struct timespec start_;
    struct timespec ended_;
};

struct TestContainerStatsCompare {
    bool operator()(TestContainerStats const& lhs, TestContainerStats const& rhs) const
    {
        return lhs.key_ < rhs.key_;
    }
};

class TestContainer : public TestBase, public std::enable_shared_from_this<TestContainer> {
public:
    using Ptr = std::shared_ptr<TestContainer>;

    TestContainer(boost::asio::io_context& ctx, MicroServiceClient& msClient, ValueId const& valueId);

    virtual ~TestContainer();

    virtual void start();
    virtual void stop();

protected:
    void onSubscribeComplete();

    void onValueId(VariantValue::Ptr valuePtr);

    void doAddToTimer(bool const wait = true);
    void onAddToTimer(boost::system::error_code const& error);

    void doRemoveFromTimer(bool const wait = true);
    void onRemoveFromTimer(boost::system::error_code const& error);

private:
    ValueId const& valueId_;
    boost::asio::deadline_timer addToTimer_;
    bool addToActive_;
    boost::asio::deadline_timer removeFromTimer_;
    bool removeFromActive_;

    bool stopTest_;

    bool subscribeCompleted_;
    int subscription_;
    std::random_device rd_;
    std::mt19937 mt_;

    std::stringstream profileStrm_;
    std::shared_ptr<FunctionTimeProfiler> timeProfile_;

    SignalScopedConnection onValueId_;

    std::mutex mutex_;

    std::set<TestContainerStats, TestContainerStatsCompare> addStats_;
    std::set<TestContainerStats, TestContainerStatsCompare> delStats_;

    std::set<std::string> added_;
};

} // namespace Edge
} // namespace Compan

#endif // __TEST_CONTAINER_H__
