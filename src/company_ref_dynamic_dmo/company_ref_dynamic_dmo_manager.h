/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_manager.h
 @brief -*-*-*-*-*-
 */
#ifndef __company_ref_DYNAMIC_DMO_MANAGER_H__
#define __company_ref_DYNAMIC_DMO_MANAGER_H__

#include <company_ref_sdk_value_ids/dynamic_dmo_value_ids.h>

#include <memory>
#include <string>
#include <vector>

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

class VariantTextValue;
using VariantTextValuePtr = std::shared_ptr<VariantTextValue>;
class DynamicDmoLoader;
using DynamicDmoLoaderPtr = std::shared_ptr<DynamicDmoLoader>;

/*!
 *
 */
class DynamicDmoManager : public std::enable_shared_from_this<DynamicDmoManager> {
public:
    using Ptr = std::shared_ptr<DynamicDmoManager>;

    DynamicDmoManager(
            boost::asio::io_context& ctx,
            VariantValueStore& ws,
            DmoContainer& dmo,
            DynamicDmoValueIds::Dmo&& dmoValuesIds,
            VariantTextValuePtr dmoDirPath_,
            VariantTextValuePtr persistDirPath);

    virtual ~DynamicDmoManager() = default;

    void init();

protected:
    void addLoader(DynamicDmoLoaderPtr);

    void onAddPersisted(VariantValuePtr const);

private:
    boost::asio::io_context& ctx_;
    VariantValueStore& ws_;
    DmoContainer& dmo_;

    DynamicDmoValueIds::Dmo dmoValuesIds_;
    VariantTextValuePtr dmoDirPath_;
    VariantTextValuePtr persistDirPath_;

    std::vector<DynamicDmoLoaderPtr> dmoLoaders_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_DYNAMIC_DMO_MANAGER_H__
