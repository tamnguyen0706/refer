/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_controller.cpp
 @brief DynamicDmo controller
 */

#include "company_ref_dynamic_dmo_controller.h"
#include "company_ref_dynamic_dmo_manager.h"

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_variant_valuestore/company_ref_variant_container_util.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger DynamicDmoControllerLog("dynamicdmo.controller", LogLevel::Information);

DynamicDmoController::DynamicDmoController(boost::asio::io_context& ctx, VariantValueStore& ws, DmoContainer& dmo)
    : ctx_(ctx)
    , ws_(ws)
    , dmo_(dmo)
    , valueIds_(ws_)
{
    FunctionLog(DynamicDmoControllerLog);

    if (!valueIds_.isValid()) {
        ErrorLog(DynamicDmoControllerLog) << "Value Id's not valid" << std::endl;
        return;
    }

    // this is a non-shared_ptr object, okay to do this here
    valueIds_.dmo->connectValueAddToContainerListener(
            std::bind(&DynamicDmoController::onAddMicroservice, this, std::placeholders::_1));
}

void DynamicDmoController::dmoPath(std::string const& arg)
{
    valueIds_.dmoPath->set(arg);
}

std::string DynamicDmoController::dmoPath()
{
    return valueIds_.dmoPath->str();
}

void DynamicDmoController::persistPath(std::string const& arg)
{
    valueIds_.persistPath->set(arg);
}

std::string DynamicDmoController::persistPath()
{
    return valueIds_.persistPath->str();
}

void DynamicDmoController::onAddMicroservice(VariantValuePtr const valuePtr)
{
    FunctionArgLog(DynamicDmoControllerLog) << valuePtr->id() << std::endl;

    std::string const key = VariantContainerUtil::getKey(valuePtr);

    DynamicDmoValueIds::Dmo dmoValuesIds(ws_, valueIds_.dmo->id(), key);

    if (!dmoValuesIds.isValid()) {
        ErrorLog(DynamicDmoControllerLog) << key << " isn't a valid entry" << std::endl;
        return;
    }

    auto managerPtr(std::make_shared<DynamicDmoManager>(
            ctx_, ws_, dmo_, std::move(dmoValuesIds), valueIds_.dmoPath, valueIds_.persistPath));

    if (managerPtr == nullptr) {
        ErrorLog(DynamicDmoControllerLog) << "Failed to create DynamicDmo Manager for " << key << std::endl;
        return;
    }

    managerPtr->init();

    microservices_[key] = managerPtr;
}
