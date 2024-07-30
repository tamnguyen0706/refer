/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_binder_variantvalue.h
 @brief Binds a Logger to a Variant ValueStore element
 */
#ifndef __Compan_LOGGER_BINDER_VARIANTVALUE_H__
#define __Compan_LOGGER_BINDER_VARIANTVALUE_H__

#include <company_ref_variant_valuestore/company_ref_variant_valuestore_variant.h>
#include <Compan_logger/Compan_logger_binder.h>

namespace Compan{
namespace Edge {

/*
 * Binds a Logger to a Variant ValueStore element
 */
class VariantValueLoggerBinder : public CompanLoggerBinder {
public:
    VariantValueLoggerBinder(VariantValue::Ptr valuePtr, CompanLogger& logger);
    virtual ~VariantValueLoggerBinder() = default;

    void onChanged(VariantValue::Ptr const);
    virtual void onLogLevel(LogLevel const logLevel);

private:
    VariantValue::Ptr valuePtr_;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_BINDER_VARIANTVALUE_H__
