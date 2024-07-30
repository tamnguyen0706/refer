/**
 Copyright © 2024 COMPAN REF
 @file company_ref_microservice_dynamic_dmo.cpp
 @brief -*-*-*-*-*-
 */

#include "company_ref_microservice_dynamic_dmo.h"

// #include <company_ref_variant_valuestore/company_ref_variant_bool_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_enum_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>
// #include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>
#include <company_ref_utils/company_ref_signals.h>
#include <company_ref_utils/company_ref_weak_bind.h>
#include <company_ref_variant_valuestore/company_ref_variant_container_util.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

using namespace Compan::Edge;

MicroserviceDynamicDmo::MicroserviceDynamicDmo(VariantValueStore& ws, std::string const& appName)
    : ws_(ws)
    , appName_(appName)
    , init_(false)
{
}

void MicroserviceDynamicDmo::init(InitDmoCompleteCb const& cb)
{
    // We want to make an entry for dynamic DMO's
    DynamicDmoValueIds dynamicDmoIds(ws_);
    dynamicDmoIds.dmo->insert(appName_);

    onDmoAddedToContainer_ = dynamicDmoIds.dmo->connectValueAddToContainerListener(
            WeakBind(&MicroserviceDynamicDmo::onDmoAddedToContainer, shared_from_this(), std::placeholders::_1, cb));
}

void MicroserviceDynamicDmo::onDmoAddedToContainer(VariantValue::Ptr const valuePtr, InitDmoCompleteCb const& cb)
{
    if (valuePtr->setUpdateType() == VariantValue::Local) return;

    if (VariantContainerUtil::getKey(valuePtr) != appName_) return;

    init_ = true;

    if (cb) cb(true);

    onDmoAddedToContainer_.disconnect();
}

void MicroserviceDynamicDmo::load(std::string const& dmoFileName, LoadActionCompleteCb const& cb)
{
    if (!init_) {
        if (cb) cb(DynamicDmoValueIds::Dmo::Status::Failed);
        return;
    }

    DynamicDmoValueIds::Dmo valueIds(ws_, ValueId(DynamicDmoValueIds::BaseValueId, "dmo"), appName_);
    if (!valueIds.isValid()) {
        if (cb) cb(DynamicDmoValueIds::Dmo::Status::Failed);
        return;
    }

    valueIds.config->path->set(dmoFileName);
    valueIds.config->action->set(DynamicDmoValueIds::Dmo::Config::Load);

    onDmoStatus_ = valueIds.status->action->connectChangedListener(
            WeakBind(&MicroserviceDynamicDmo::onDmoStatus, shared_from_this(), std::placeholders::_1, cb));
}

void MicroserviceDynamicDmo::unload(std::string const& dmoFileName, LoadActionCompleteCb const& cb)
{
    if (!init_) {
        if (cb) cb(DynamicDmoValueIds::Dmo::Status::Failed);
        return;
    }

    DynamicDmoValueIds::Dmo valueIds(ws_, ValueId(DynamicDmoValueIds::BaseValueId, "dmo"), appName_);
    if (!valueIds.isValid()) {
        if (cb) cb(DynamicDmoValueIds::Dmo::Status::Failed);
        return;
    }

    valueIds.config->path->set(dmoFileName);
    valueIds.config->action->set(DynamicDmoValueIds::Dmo::Config::Unload);

    onDmoStatus_ = valueIds.status->action->connectChangedListener(
            WeakBind(&MicroserviceDynamicDmo::onDmoStatus, shared_from_this(), std::placeholders::_1, cb));
}

void MicroserviceDynamicDmo::overWrite(std::string const& dmoFileName, LoadActionCompleteCb const& cb)
{
    if (!init_) {
        if (cb) cb(DynamicDmoValueIds::Dmo::Status::Failed);
        return;
    }

    DynamicDmoValueIds::Dmo valueIds(ws_, ValueId(DynamicDmoValueIds::BaseValueId, "dmo"), appName_);
    if (!valueIds.isValid()) {
        if (cb) cb(DynamicDmoValueIds::Dmo::Status::Failed);
        return;
    }

    valueIds.config->path->set(dmoFileName);
    valueIds.config->action->set(DynamicDmoValueIds::Dmo::Config::OverWrite);

    onDmoStatus_ = valueIds.status->action->connectChangedListener(
            WeakBind(&MicroserviceDynamicDmo::onDmoStatus, shared_from_this(), std::placeholders::_1, cb));
}

void MicroserviceDynamicDmo::onDmoStatus(VariantValuePtr const valuePtr, LoadActionCompleteCb const& cb)
{
    if (valuePtr->setUpdateType() == VariantValue::Local) return;

    VariantEnumValue::Ptr enumValuePtr = std::dynamic_pointer_cast<VariantEnumValue>(valuePtr);
    if (enumValuePtr == nullptr) return;

    if (enumValuePtr->setUpdateType() == VariantValue::Local) return;

    if (cb) cb(static_cast<DynamicDmoValueIds::Dmo::Status::ActionEnum>(enumValuePtr->get()));

    onDmoStatus_.disconnect();
}
