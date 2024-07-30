/**
 Copyright © 2024 COMPAN REF
 @file test_container.cpp
 @brief -*-*-*-*-*-
 */
#include "test_container.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_utils/company_ref_timespec_utils.h>

#include <iostream>
#include <time.h>

namespace {
std::string createRandomName()
{
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::string randomName;

    for (int i = 0; i < 8; ++i) {
        size_t index = rand() % (int)(sizeof(charset) - 1);
        randomName += charset[index];
    }

    return randomName;
}
} // namespace

using namespace Compan::Edge;

TestContainerStats::TestContainerStats(std::string const& key)
    : TestContainerStats(key, CompanEdgeProtocol::Unset)
{
}

TestContainerStats::TestContainerStats(std::string const& key, CompanEdgeProtocol::Value_Type const& action)
    : key_(key)
    , action_(action)
    , start_()
    , ended_()
{
    clock_gettime(CLOCK_MONOTONIC, &start_);
}

void TestContainerStats::completed()
{
    clock_gettime(CLOCK_MONOTONIC, &ended_);
}

void TestContainerStats::print(std::ostream& os) const
{
    TimeSpecWrapper tsw(TimeSpecUtil::getDiff(start_, ended_));

    os << key_ << " " << CompanEdgeProtocol::Value_Type_Name(action_) << " ";
    if (TimeSpecUtil::toNsec(ended_) == 0)
        os << "Incomplete";
    else
        os << tsw;
    os << std::endl;
}

TestContainer::TestContainer(boost::asio::io_context& ctx, MicroServiceClient& msClient, ValueId const& valueId)
    : TestBase(ctx, msClient)
    , valueId_(valueId)
    , addToTimer_(ctx)
    , addToActive_(false)
    , removeFromTimer_(ctx)
    , removeFromActive_(false)
    , stopTest_(false)
    , subscribeCompleted_(false)
    , subscription_(0)
    , mt_(rd_())
{
}

TestContainer::~TestContainer()
{
    std::cout << "Missing completions" << std::endl;
    for (auto& stats : addStats_) { stats.print(std::cout); }
    for (auto& stats : delStats_) { stats.print(std::cout); }
}

void TestContainer::start()
{
    timeProfile_ = std::make_shared<FunctionTimeProfiler>(profileStrm_, msClient_.appName());

    subscription_ = msClient_.subscribeWithCompletion(
            valueId_, std::bind(&TestContainer::onSubscribeComplete, shared_from_this()));
}

void TestContainer::stop()
{
    if (!subscribeCompleted_) msClient_.cancelSubscibeCompletion(subscription_);

    if (timeProfile_) {
        profileStrm_ << "Subscription completed: " << std::boolalpha << subscribeCompleted_ << "\t";
        timeProfile_.reset();
        std::cout << profileStrm_.str() << std::endl;
    }

    stopTest_ = true;

    addToTimer_.cancel();
    addToActive_ = true;

    doRemoveFromTimer(false);
}

void TestContainer::onSubscribeComplete()
{
    subscribeCompleted_ = true;

    if (timeProfile_) {
        profileStrm_ << "Subscription completed: " << std::boolalpha << subscribeCompleted_ << "\t";
        timeProfile_.reset();
        std::cout << profileStrm_.str() << std::endl;
    }

    VariantValue::Ptr valuePtr = msClient_.ws().get(valueId_);
    if (valuePtr == nullptr) {
        std::cerr << "No " << valueId_ << " found!" << std::endl;
        ctx_.stop();
        return;
    }

    if (valuePtr->type() != CompanEdgeProtocol::Container) {
        std::cerr << valueId_ << " not a container" << std::endl;
        ctx_.stop();
        return;
    }

    onValueId_ = valuePtr->connectChangedListener(
            std::bind(&TestContainer::onValueId, shared_from_this(), std::placeholders::_1));

    doAddToTimer(false);
}

void TestContainer::onValueId(VariantValue::Ptr valuePtr)
{
    if (valuePtr == nullptr) return;

    if (valuePtr->setUpdateType() == VariantValue::Local) return;

    if (valuePtr->type() == CompanEdgeProtocol::ContainerAddTo) {
        std::string key = valuePtr->get().addtocontainer().key();
        if (key.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);

        auto element = addStats_.find(TestContainerStats(key));
        if (element == addStats_.end()) return;

        TestContainerStats stats = *element;

        addStats_.erase(element);

        stats.completed();
        stats.print(std::cout);

        doRemoveFromTimer();
    } else if (valuePtr->type() == CompanEdgeProtocol::ContainerRemoveFrom) {
        std::string key = valuePtr->get().removefromcontainer().key();
        if (key.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);

        auto element = delStats_.find(TestContainerStats(key));
        if (element == delStats_.end()) return;

        TestContainerStats stats = *element;

        delStats_.erase(element);

        stats.completed();
        stats.print(std::cout);

        if (stopTest_) {

            if (delStats_.empty()) { onValueId_.disconnect(); }

            doRemoveFromTimer(false);
            return;
        }

        doAddToTimer();
    }
}

void TestContainer::doAddToTimer(bool const wait)
{
    if (addToActive_) return;

    size_t msWait = 0;

    if (wait) {
        std::uniform_int_distribution<> distrib(500, 1000);
        msWait = distrib(mt_);
    }

    addToTimer_.expires_from_now(boost::posix_time::milliseconds(msWait));
    addToTimer_.async_wait(std::bind(&TestContainer::onAddToTimer, shared_from_this(), std::placeholders::_1));

    addToActive_ = true;
}

void TestContainer::onAddToTimer(boost::system::error_code const& error)
{
    if (error && error != boost::asio::error::operation_aborted) {
        std::cout << error.message() << std::endl;
        return;
    }

    std::uniform_int_distribution<> distrib(1, 8);

    int maxActions = distrib(mt_);
    for (int i = 0; i < maxActions; ++i) {
        std::string key = createRandomName();

        TestContainerStats stats(key, CompanEdgeProtocol::ContainerAddTo);

        std::lock_guard<std::mutex> lock(mutex_);
        addStats_.emplace(std::move(stats));
        added_.insert(key);

        msClient_.ws().addToContainer(valueId_, key);
    }

    addToActive_ = false;
}

void TestContainer::doRemoveFromTimer(bool const wait)
{
    if (removeFromActive_) return;

    size_t msWait = 0;

    if (wait) { msWait = 500; }

    removeFromTimer_.expires_from_now(boost::posix_time::milliseconds(msWait));
    removeFromTimer_.async_wait(
            std::bind(&TestContainer::onRemoveFromTimer, shared_from_this(), std::placeholders::_1));

    removeFromActive_ = true;
}

void TestContainer::onRemoveFromTimer(boost::system::error_code const& error)
{
    if (error && error != boost::asio::error::operation_aborted) {
        std::cout << error.message() << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& key : added_) {
        TestContainerStats stats(key, CompanEdgeProtocol::ContainerRemoveFrom);
        delStats_.emplace(std::move(stats));
        msClient_.ws().removeFromContainer(valueId_, key);
    }

    added_.clear();

    removeFromActive_ = false;
}
