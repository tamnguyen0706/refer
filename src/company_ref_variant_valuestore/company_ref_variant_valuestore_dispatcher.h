/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_dispatcher.h
 @brief VariantValue Data dispatcher
 */
#ifndef __company_ref_VARIANT_VALUESTORE_DISPATCHER_H__
#define __company_ref_VARIANT_VALUESTORE_DISPATCHER_H__

#include <boost/core/noncopyable.hpp>

#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valuedata.h>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <future>
#include <thread>

namespace Compan{
namespace Edge {

/// VariantValue Data dispatcher
class VariantValueDispatcher : private boost::noncopyable {
public:
    using Ptr = std::shared_ptr<VariantValueDispatcher>;

    using ProtocolValueGet = std::packaged_task<CompanEdgeProtocol::Value()>;
    using ProtocolValueGetPtr = std::shared_ptr<ProtocolValueGet>;
    using ProtocolValueSet = std::packaged_task<ValueDataSet::Results(CompanEdgeProtocol::Value const&)>;
    using ProtocolValueSetPtr = std::shared_ptr<ProtocolValueSet>;
    using ProtocolValueHasData = std::packaged_task<bool()>;
    using ProtocolValueHasDataPtr = std::shared_ptr<ProtocolValueHasData>;

public:
    VariantValueDispatcher();
    virtual ~VariantValueDispatcher() = default;

    /// starts the Dispatcher's thread
    void start();

    /// Stops the Dispatcher's thread
    void stop();

    boost::asio::io_context& getIoContext();

    /// Performs a get operation from a CompanEdgeProtocolValueData compliant object
    void getCompanEdgeProtocolValue(ProtocolValueGetPtr);

    /// Performs a set operation on a CompanEdgeProtocolValueData compliant object
    void setCompanEdgeProtocolValue(ProtocolValueSetPtr, CompanEdgeProtocol::Value const&);

    /// Performs a has data operation on a CompanEdgeProtocolValueData compliant object
    void hasDataCompanEdgeProtocolValue(ProtocolValueHasDataPtr);

private:
    // single context, single thread - keeps the requests in order
    boost::asio::io_context ctx_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard_;

    std::thread thread_;
};

inline boost::asio::io_context& VariantValueDispatcher::getIoContext()
{
    return ctx_;
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_VALUESTORE_DISPATCHER_H__
