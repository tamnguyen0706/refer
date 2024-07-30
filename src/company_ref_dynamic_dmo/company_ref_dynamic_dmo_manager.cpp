/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_manager.cpp
 @brief -*-*-*-*-*-
 */

#include "company_ref_dynamic_dmo_manager.h"
#include "company_ref_dynamic_dmo_loader.h"

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_utils/company_ref_weak_bind.h>
#include <company_ref_variant_valuestore/company_ref_variant_container_util.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>
#include <Compan_logger/Compan_logger.h>

#include <boost/filesystem.hpp>

using namespace Compan::Edge;

CompanLogger DynamicDmoManagerLog("dynamicdmo.manager", LogLevel::Information);

DynamicDmoManager::DynamicDmoManager(
        boost::asio::io_context& ctx,
        VariantValueStore& ws,
        DmoContainer& dmo,
        DynamicDmoValueIds::Dmo&& dmoValuesIds,
        VariantTextValuePtr dmoDirPath,
        VariantTextValuePtr persistDirPath)
    : ctx_(ctx)
    , ws_(ws)
    , dmo_(dmo)
    , dmoValuesIds_(std::move(dmoValuesIds))
    , dmoDirPath_(dmoDirPath)
    , persistDirPath_(persistDirPath)
{
    FunctionLog(DynamicDmoManagerLog);
    addLoader(std::make_shared<DynamicDmoLoader>(
            ctx_,
            ws_,
            dmo_,
            dmoDirPath_,
            dmoValuesIds_.config->path,
            dmoValuesIds_.config->action,
            dmoValuesIds_.status->action));
}

void DynamicDmoManager::init()
{
    dmoValuesIds_.persisted->connectValueAddToContainerListener(
            WeakBind(&DynamicDmoManager::onAddPersisted, shared_from_this(), std::placeholders::_1));
}

void DynamicDmoManager::addLoader(DynamicDmoLoaderPtr loader)
{
    loader->init();
    dmoLoaders_.push_back(loader);
}

void DynamicDmoManager::onAddPersisted(VariantValuePtr const valuePtr)
{
    FunctionLog(DynamicDmoManagerLog);

    std::string const key = VariantContainerUtil::getKey(valuePtr);

    DynamicDmoValueIds::Dmo::Persisted valuesIds(ws_, dmoValuesIds_.persisted->id(), key);

    if (!valuesIds.isValid()) {
        ErrorLog(DynamicDmoManagerLog) << key << " isn't a valid entry" << std::endl;
        return;
    }

    addLoader(std::make_shared<DynamicDmoLoader>(
            ctx_,
            ws_,
            dmo_,
            persistDirPath_,
            valuesIds.config->path,
            valuesIds.config->action,
            valuesIds.status->action));
}
