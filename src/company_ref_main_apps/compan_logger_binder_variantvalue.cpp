/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_binder_variantvalue.cpp
 @brief Binds a Logger to a Variant ValueStore element
 */
#include "Compan_logger_binder_variantvalue.h"

#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h>

using namespace Compan::Edge;

VariantValueLoggerBinder::VariantValueLoggerBinder(VariantValue::Ptr valuePtr, CompanLogger& logger)
    : CompanLoggerBinder(valuePtr->id(), logger)
    , valuePtr_(valuePtr)
{
    valuePtr->connectChangedListener(std::bind(&VariantValueLoggerBinder::onChanged, this, std::placeholders::_1));

    onChanged(valuePtr);
}

void VariantValueLoggerBinder::onChanged(VariantValue::Ptr const valuePtr)
{
    if (valuePtr == nullptr) return;

    if (valuePtr->type() != CompanEdgeProtocol::Enum) return;

    CompanEdgeProtocol::Value enumValue = valuePtr->get();

    logLevel(static_cast<LogLevel>(enumValue.enumvalue().value()));
}

void VariantValueLoggerBinder::onLogLevel(LogLevel const logLevel)
{
    LogLevelStrBiMap const& logLevelBiMap = getLogLevelBiMap();

    auto iter = logLevelBiMap.left.find(logLevel);
    if (iter != logLevelBiMap.left.end()) valuePtr_->set(iter->second);
}
