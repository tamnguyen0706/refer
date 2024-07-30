/**
 Copyright © 2024 COMPAN REF
 @file ms_test_microservice.cpp
 @brief -*-*-*-*-*-
 */
#include "ms_test_microservice.h"

#include "test_base.h"
#include "test_container.h"
#include "test_subscribe.h"

using namespace Compan::Edge;

MsTestMicroSerivce::MsTestMicroSerivce(
        boost::asio::io_context& ctx,
        std::string const& clientName,
        int const timeOut,
        std::string const& udsPath)
    : ctx_(ctx)
    , shutdownTimer_(ctx_)
    , msClient_(ctx_, clientName, udsPath)
    , forceShutdown_(false)
{
    doShutdownTimer(timeOut * 1000);
}

MsTestMicroSerivce::MsTestMicroSerivce(
        boost::asio::io_context& ctx,
        std::string const& clientName,
        int const timeOut,
        std::string const& ipAddr,
        std::string const& port)
    : ctx_(ctx)
    , shutdownTimer_(ctx_)
    , msClient_(ctx_, clientName, ipAddr, port)
    , forceShutdown_(false)
{
    doShutdownTimer(timeOut * 1000);
}

MsTestMicroSerivce::~MsTestMicroSerivce()
{
    if (testPtr_) {
        testPtr_->stop();
        testPtr_.reset();
    }
}

void MsTestMicroSerivce::doShutdownTimer(int const timeOutMs)
{
    shutdownTimer_.expires_from_now(boost::posix_time::milliseconds(timeOutMs));
    shutdownTimer_.async_wait(std::bind(&MsTestMicroSerivce::onShutdownTimer, this, std::placeholders::_1));
}

void MsTestMicroSerivce::onShutdownTimer(boost::system::error_code const& error)
{
    // cancelation means - we don't want to exit!
    if (error == boost::asio::error::operation_aborted) return;

    if (!forceShutdown_) {

        if (testPtr_) {
            testPtr_->stop();
            testPtr_.reset();
        }

        doShutdownTimer(2000);
        forceShutdown_ = true;
        return;
    }

    ctx_.stop();
}

void MsTestMicroSerivce::doSubscribe(ValueId const& valueId)
{
    testPtr_ = std::make_shared<TestSubscribe>(ctx_, msClient_, valueId);
    testPtr_->start();
}

void MsTestMicroSerivce::doContainer(ValueId const& valueId)
{
    testPtr_ = std::make_shared<TestContainer>(ctx_, msClient_, valueId);
    testPtr_->start();
}
