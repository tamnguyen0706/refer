/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_loader.h
 @brief VariantValue helper class for Config/Status actions
 */
#ifndef __company_ref_DYNAMIC_DMO_LOADER_H__
#define __company_ref_DYNAMIC_DMO_LOADER_H__

#include <memory>

namespace boost {
namespace filesystem {
class path;
} // namespace filesystem
namespace asio {
class io_context;
} // namespace asio
} // namespace boost

namespace Compan{
namespace Edge {

class SignalScopedConnection;
class VariantValueStore;
class DmoContainer;
class ValueId;

class VariantValue;
using VariantValuePtr = std::shared_ptr<VariantValue>;

class VariantTextValue;
using VariantTextValuePtr = std::shared_ptr<VariantTextValue>;

class VariantEnumValue;
using VariantEnumValuePtr = std::shared_ptr<VariantEnumValue>;

/*!
 * Dynamic DMO Loader
 *
 * Can be used for DMO Meta Definitions, Persisted files and factory reset files
 */
class DynamicDmoLoader : public std::enable_shared_from_this<DynamicDmoLoader> {
public:
    using Ptr = std::shared_ptr<DynamicDmoLoader>;

    DynamicDmoLoader(
            boost::asio::io_context& ctx,
            VariantValueStore& ws,
            DmoContainer& dmo,
            VariantTextValuePtr dmoDirPath,
            VariantTextValuePtr path,
            VariantEnumValuePtr action,
            VariantEnumValuePtr status);

    virtual ~DynamicDmoLoader();

    void init();

protected:
    void onConfigAction(VariantValuePtr const);

    void onLoad(boost::filesystem::path const);
    void onUnload(boost::filesystem::path const);
    void onOverWrite(boost::filesystem::path const);

private:
    boost::asio::io_context& ctx_;
    VariantValueStore& ws_;
    DmoContainer& dmo_;

    VariantTextValuePtr dmoDirPath_;
    VariantEnumValuePtr action_;
    VariantTextValuePtr path_;
    VariantEnumValuePtr status_;

    SignalScopedConnection onConfigAction_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_DYNAMIC_DMO_LOADER_H__
