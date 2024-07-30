/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_controller.h
 @brief DynamicDmo controller
 */
#ifndef __company_ref_DYNAMIC_DMO_CONTROLLER_H__
#define __company_ref_DYNAMIC_DMO_CONTROLLER_H__

#include <company_ref_sdk_value_ids/dynamic_dmo_value_ids.h>

#include <map>
#include <memory>
#include <string>

namespace boost {
namespace asio {
class io_context;
} // namespace asio
} // namespace boost

namespace Compan{
namespace Edge {

class VariantValue;
using VariantValuePtr = std::shared_ptr<VariantValue>;
class VariantValueStore;
class DmoContainer;
class ValueId;

class DynamicDmoManager;
using DynamicDmoManagerPtr = std::shared_ptr<DynamicDmoManager>;

/*!
 *  This class is used by the WS Server to instantiate
 *  the dynamic DMO loading mechanism
 */
class DynamicDmoController {
public:
    DynamicDmoController(boost::asio::io_context& ctx, VariantValueStore& ws, DmoContainer& dmo);

    virtual ~DynamicDmoController() = default;

    // get/set the default DMO path location
    void dmoPath(std::string const& arg);
    std::string dmoPath();

    // get/set the default persistence storage path location
    void persistPath(std::string const& arg);
    std::string persistPath();

protected:
    void onAddMicroservice(VariantValuePtr const);

private:
    boost::asio::io_context& ctx_;
    VariantValueStore& ws_;
    DmoContainer& dmo_;

    DynamicDmoValueIds valueIds_;

    std::map<std::string, DynamicDmoManagerPtr> microservices_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_DYNAMIC_DMO_CONTROLLER_H__
