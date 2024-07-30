/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_loader.cpp
 @brief VariantValue helper class for Config/Status actions
 */

#include <company_ref_utils/company_ref_weak_bind.h>
#include <company_ref_variant_valuestore/company_ref_variant_bool_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_enum_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h>
#include <Compan_logger/Compan_logger.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "company_ref_dynamic_dmo_load_actions.h"
#include "company_ref_dynamic_dmo_loader.h"

#include <company_ref_sdk_value_ids/dynamic_dmo_value_ids.h>

using namespace Compan::Edge;

CompanLogger DynamicDmoLoaderLog("dynamicdmo.loader", LogLevel::Information);

DynamicDmoLoader::DynamicDmoLoader(
        boost::asio::io_context& ctx,
        VariantValueStore& ws,
        DmoContainer& dmo,
        VariantTextValuePtr dmoDirPath,
        VariantTextValuePtr path,
        VariantEnumValuePtr action,
        VariantEnumValuePtr status)
    : ctx_(ctx)
    , ws_(ws)
    , dmo_(dmo)
    , dmoDirPath_(dmoDirPath)
    , action_(action)
    , path_(path)
    , status_(status)
{
    FunctionLog(DynamicDmoLoaderLog);
}

DynamicDmoLoader::~DynamicDmoLoader()
{
    FunctionLog(DynamicDmoLoaderLog);
}

void DynamicDmoLoader::init()
{
    onConfigAction_ = action_->connectChangedListener(
            WeakBind(&DynamicDmoLoader::onConfigAction, shared_from_this(), std::placeholders::_1));

    onConfigAction(nullptr);
}

void DynamicDmoLoader::onConfigAction(VariantValuePtr const)
{
    FunctionLog(DynamicDmoLoaderLog);

    int configActionEnum(action_->get());

    if (configActionEnum == DynamicDmoValueIds::Dmo::Config::None) return;

    SignalConnectionDisableScoped disable(onConfigAction_);
    action_->set(DynamicDmoValueIds::Dmo::Config::None);

    boost::filesystem::path path = boost::filesystem::path(dmoDirPath_->str()) / boost::filesystem::path(path_->str());

    if (!boost::filesystem::is_regular_file(path)) {

        WarnLog(DynamicDmoLoaderLog) << path << " doesn't exist" << std::endl;
        status_->set(DynamicDmoValueIds::Dmo::Status::FileNotFound);

        return;
    }

    status_->set(DynamicDmoValueIds::Dmo ::Config ::None);

    switch (configActionEnum) {
    case DynamicDmoValueIds::Dmo ::Config ::Load:
        boost::asio::post(ctx_, std::bind(&DynamicDmoLoader::onLoad, shared_from_this(), path));
        break;
    case DynamicDmoValueIds::Dmo ::Config ::Unload:
        boost::asio::post(ctx_, WeakBind(&DynamicDmoLoader::onUnload, shared_from_this(), path));
        break;
    case DynamicDmoValueIds::Dmo ::Config ::OverWrite:
        boost::asio::post(ctx_, WeakBind(&DynamicDmoLoader::onOverWrite, shared_from_this(), path));
        break;
    }
}

void DynamicDmoLoader::onLoad(boost::filesystem::path const dmoFilePath)
{
    FunctionArgLog(DynamicDmoLoaderLog) << dmoFilePath << std::endl;

    boost::filesystem::ifstream strm(dmoFilePath);

    int statusActionEnum(DynamicDmoValueIds::Dmo::Status::Complete);

    if (!DmoLoadActions::loadDmo(strm, dmo_, ws_)) {
        ErrorLog(DynamicDmoLoaderLog) << "load " << dmoFilePath << " failed" << std::endl;

        statusActionEnum = DynamicDmoValueIds::Dmo::Status::Failed;
    }

    status_->set(statusActionEnum);
}

void DynamicDmoLoader::onUnload(boost::filesystem::path const dmoFilePath)
{
    FunctionArgLog(DynamicDmoLoaderLog) << dmoFilePath << std::endl;

    boost::filesystem::ifstream strm(dmoFilePath);

    action_->set(DynamicDmoValueIds::Dmo::Config::None);

    int statusActionEnum(DynamicDmoValueIds::Dmo::Status::Complete);

    if (!DmoLoadActions::unloadDmo(strm, dmo_, ws_)) {
        ErrorLog(DynamicDmoLoaderLog) << "unload " << dmoFilePath << " failed" << std::endl;

        statusActionEnum = DynamicDmoValueIds::Dmo::Status::Failed;
    }

    status_->set(statusActionEnum);
}

void DynamicDmoLoader::onOverWrite(boost::filesystem::path const dmoFilePath)
{
    FunctionArgLog(DynamicDmoLoaderLog) << dmoFilePath << std::endl;

    boost::filesystem::ifstream strm(dmoFilePath);

    action_->set(DynamicDmoValueIds::Dmo::Config::None);

    int statusActionEnum(DynamicDmoValueIds::Dmo::Status::Complete);

    if (!DmoLoadActions::reloadDmo(strm, dmo_, ws_)) {
        ErrorLog(DynamicDmoLoaderLog) << "reload " << dmoFilePath << " failed" << std::endl;

        statusActionEnum = DynamicDmoValueIds::Dmo::Status::Failed;
    }

    status_->set(statusActionEnum);
}
