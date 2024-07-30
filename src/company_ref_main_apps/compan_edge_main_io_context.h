/**
 Copyright © 2023 COMPAN REF
 @file company_ref_main_io_context.h
 @brief Application's main io_context
 */
#ifndef __company_ref_MAIN_IO_CONTEXT_H__
#define __company_ref_MAIN_IO_CONTEXT_H__

#include <Compan_logger/Compan_logger.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

namespace Compan{
namespace Edge {

/*!
 * Utility class that owns a boost::asio::io_context
 * This class takes care of the following:
 *  - ownership of the main io_context of an application
 *  - Signal handling
 *  - configuration of CompanLogger's ASIO dispatcher
 *  - Running multiple threads of the io_context::run
 *
 */
class MainIoContext {
public:
    MainIoContext(CompanLogger& logger);
    virtual ~MainIoContext();

    /// Creates the required amount of threads to execute the io_context
    void run(size_t const threadCount = 1);

    /// Returns the application's main io_context
    boost::asio::io_context& getIoContext();

protected:
    /// signal handler for termination signals
    void onSignal(boost::system::error_code const& error, int signal_number);

private:
    boost::asio::io_context ctx_;
    boost::asio::signal_set signalHandler_;

    CompanLogger& logger_;
};

inline boost::asio::io_context& MainIoContext::getIoContext()
{
    return ctx_;
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_MAIN_IO_CONTEXT_H__
