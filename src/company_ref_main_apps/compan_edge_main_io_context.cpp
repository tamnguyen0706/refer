/**
 Copyright © 2023 COMPAN REF
 @file company_ref_main_io_context.cpp
  @brief Application's main io_context
 */
#include "company_ref_main_io_context.h"

#include <Compan_logger/Compan_logger_controller.h>
#include <Compan_logger/Compan_logger_dispatcher_asio.h>

#include <queue>
#include <thread>

using namespace Compan::Edge;

MainIoContext ::MainIoContext(CompanLogger& logger)
    : ctx_()
    , signalHandler_(ctx_, SIGINT, SIGTERM)
    , logger_(logger)
{
}

MainIoContext ::~MainIoContext()
{
}

void MainIoContext ::run(size_t const threadCount)
{
    std::queue<std::thread> threads;

    //    CompanLoggerController::get()->setDispatcher(std::make_unique<CompanLoggerDispatcherAsio>(ctx_));

    signalHandler_.async_wait(std::bind(&MainIoContext::onSignal, this, std::placeholders::_1, std::placeholders::_2));

    for (size_t i = 0; i < threadCount; ++i) {
        std::thread runThread([&] { ctx_.run(); });
        threads.emplace(std::move(runThread));
    }

    while (!threads.empty()) {
        try {
            if (threads.front().joinable()) threads.front().join();
        } catch (std::exception& e) {
            ErrorLog(logger_) << e.what() << std::endl;
        }
        threads.pop();
    }
}

void MainIoContext::onSignal(boost::system::error_code const& error, int signal_number)
{
    if (error) {
        ErrorLog(logger_) << "Received signal " << signal_number << ", " << error.message() << std::endl;
    } else {
        // A signal occurred.
        ctx_.stop();
    }
}
