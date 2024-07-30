/**
 Copyright © 2024 COMPAN REF
 @file test_subscribe.cpp
 @brief -*-*-*-*-*-
 */
#include "test_subscribe.h"

#include <iostream>

using namespace Compan::Edge;

TestSubscribe::TestSubscribe(boost::asio::io_context& ctx, MicroServiceClient& msClient, ValueId const& valueId)
    : TestBase(ctx, msClient)
    , valueId_(valueId)
    , subscribeCompleted_(false)
    , subscription_(0)
{
}

TestSubscribe::~TestSubscribe() = default;

void TestSubscribe::start()
{
    timeProfile_ = std::make_shared<FunctionTimeProfiler>(profileStrm_, msClient_.appName());

    subscription_ = msClient_.subscribeWithCompletion(
            valueId_, std::bind(&TestSubscribe::onSubscribeComplete, shared_from_this()));
}

void TestSubscribe::stop()
{
    if (!subscribeCompleted_) msClient_.cancelSubscibeCompletion(subscription_);

    if (timeProfile_) {
        profileStrm_ << "Subscription completed: " << std::boolalpha << subscribeCompleted_ << "\t";
        timeProfile_.reset();
        std::cout << profileStrm_.str() << std::endl;
    }
}

void TestSubscribe::onSubscribeComplete()
{
    subscribeCompleted_ = true;

    if (timeProfile_) {
        profileStrm_ << "Subscription completed: " << std::boolalpha << subscribeCompleted_ << "\t";
        timeProfile_.reset();
        std::cout << profileStrm_.str() << std::endl;
    }
}
