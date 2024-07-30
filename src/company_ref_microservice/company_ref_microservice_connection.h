/**
 Copyright © 2023 COMPAN REF
 @file company_ref_microservice_connection.h
 @brief UDS Connection manager
 */
#ifndef __company_ref_MICROSERVICE_CONNECTION_H__
#define __company_ref_MICROSERVICE_CONNECTION_H__

#include <company_ref_asio/company_ref_asio_connection.h>
#include <boost/asio/deadline_timer.hpp>

#include <memory>

namespace Compan{
namespace Edge {

/*
 * @brief
 */
class ClientConnectionManager : public std::enable_shared_from_this<ClientConnectionManager> {
public:
    using Ptr = std::shared_ptr<ClientConnectionManager>;

    ClientConnectionManager(boost::asio::io_context& ctx, AsioConnectionBase::Ptr client);
    virtual ~ClientConnectionManager();

    void start();
    void stop();

    /// Number of times to retry until it's a failure
    void setRetryCount(size_t const maxCount);

protected:
    /// notification we've connected
    void onConnection();

    /// Sets the reconnection timer
    void startTimer();

    /// Executes the connection
    void timerHandler(boost::system::error_code const& error);

private:
    AsioConnectionBase::Ptr client_;
    boost::asio::deadline_timer timer_; //!< Timer for reconnects
    uint32_t timeoutMs_;                //!< Last timeout value in milliseconds
    bool timerActive_;                  //!< If the timer has been set

    size_t retryCount_; //!< Number of retries
    size_t retryMax_;   //!< Max allowed of retries

    SignalScopedConnection clientConnected_;
    SignalScopedConnection clientDisconnected_;

    static uint32_t const ReconnectTimeoutMs = 250;
};

inline void ClientConnectionManager::setRetryCount(size_t const maxCount)
{
    retryMax_ = maxCount;
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_MICROSERVICE_CONNECTION_H__
