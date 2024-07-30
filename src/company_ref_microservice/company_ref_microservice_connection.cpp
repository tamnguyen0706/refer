/**
 Copyright © 2023 COMPAN REF
 @file company_ref_microservice_connection.cpp
 @brief UDS Connection manager
 */
#include "company_ref_microservice_connection.h"
#include <company_ref_utils/company_ref_weak_bind.h>

#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {
CompanLogger MicroServiceConnectionLog("MsClient.Connection", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

ClientConnectionManager ::ClientConnectionManager(boost::asio::io_context& ctx, AsioConnectionBase::Ptr client)
    : client_(client)
    , timer_(ctx)
    , timeoutMs_(0)
    , timerActive_(false)
    , retryCount_(0)
    , retryMax_(-1)
{
    FunctionLog(MicroServiceConnectionLog);
}

ClientConnectionManager ::~ClientConnectionManager()
{
    FunctionLog(MicroServiceConnectionLog);

    stop();
}

void ClientConnectionManager::start()
{
    clientConnected_ =
            client_->connectOnConnection(WeakBind(&ClientConnectionManager ::onConnection, shared_from_this()));
    clientDisconnected_ =
            client_->connectOnDisconnection(WeakBind(&ClientConnectionManager::startTimer, shared_from_this()));
}

void ClientConnectionManager::stop()
{
    retryMax_ = 0;
    timer_.cancel();

    clientConnected_.disconnect();
    clientDisconnected_.disconnect();
}

void ClientConnectionManager::onConnection()
{
    FunctionLog(MicroServiceConnectionLog);

    timer_.cancel();
    timeoutMs_ = 0;
}

void ClientConnectionManager ::startTimer()
{
    FunctionLog(MicroServiceConnectionLog);

    if (timerActive_) return;

    if (!(++retryCount_ < retryMax_)) return;

    // cycle up from 250,500,1000,2000,4000,8000 milliseconds
    // Then, restarts at 250

    if (timeoutMs_ == 0)
        timeoutMs_ = ReconnectTimeoutMs;
    else
        timeoutMs_ *= 2;

    if (timeoutMs_ > 8000) timeoutMs_ = ReconnectTimeoutMs;

    timer_.expires_from_now(boost::posix_time::milliseconds(timeoutMs_));
    timer_.async_wait(std::bind(&ClientConnectionManager ::timerHandler, this, std::placeholders::_1));
    timerActive_ = true;
}

void ClientConnectionManager ::timerHandler(boost::system::error_code const& error)
{
    FunctionLog(MicroServiceConnectionLog);

    timerActive_ = false;
    if (error) {
        //        ErrorLog(PersistorLog) << __FUNCTION__ << ": " << error.message() << std::endl;
        return;
    }

    client_->connect();
    startTimer();
}
