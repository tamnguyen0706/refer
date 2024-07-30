/**
 Copyright © 2024 COMPAN REF
 @file company_ref_microservice_dynamic_dmo.h
 @brief -*-*-*-*-*-
 */
#ifndef __company_ref_MICROSERVICE_DYNAMIC_DMO_H__
#define __company_ref_MICROSERVICE_DYNAMIC_DMO_H__

#include <company_ref_sdk_value_ids/dynamic_dmo_value_ids.h>

#include <memory>
#include <string>

namespace Compan{
namespace Edge {

class VariantValueStore;
class SignalScopedConnection;

class VariantValue;
using VariantValuePtr = std::shared_ptr<VariantValue>;

/*!
 * DynamicDmo interface object
 *
 * Allows a microservice to dynamically load a DMO on the WS Server
 */
class MicroserviceDynamicDmo : public std::enable_shared_from_this<MicroserviceDynamicDmo> {
public:
    using Ptr = std::shared_ptr<MicroserviceDynamicDmo>;
    using InitDmoCompleteCb = std::function<void(bool const)>;
    using LoadActionCompleteCb = std::function<void(DynamicDmoValueIds::Dmo::Status::ActionEnum const)>;

    MicroserviceDynamicDmo(VariantValueStore& ws, std::string const& appName);
    virtual ~MicroserviceDynamicDmo() = default;

    /// Initializes the required DMO Entites in the local valuestore
    void init(InitDmoCompleteCb const& cb);
    void load(std::string const&, LoadActionCompleteCb const&);
    void unload(std::string const&, LoadActionCompleteCb const&);
    void overWrite(std::string const&, LoadActionCompleteCb const&);

protected:
    void onDmoAddedToContainer(VariantValuePtr const, InitDmoCompleteCb const&);
    void onDmoStatus(VariantValuePtr const, LoadActionCompleteCb const&);

private:
    VariantValueStore& ws_;
    std::string const& appName_;
    bool init_;

    SignalScopedConnection onDmoAddedToContainer_;
    SignalScopedConnection onDmoStatus_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_MICROSERVICE_DYNAMIC_DMO_H__
