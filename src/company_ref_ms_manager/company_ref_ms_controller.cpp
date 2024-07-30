/**
 Copyright © 2023 COMPAN REF
 @file company_ref_ms_controller.cpp
 @brief Main controlling interface
 */
#include "company_ref_ms_controller.h"
#include "company_ref_ms_applet.h"

#include <company_ref_sdk_value_ids/microservices_value_ids.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>

#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {

CompanLogger MicroServiceControllerLog("ms.controller", LogLevel::Information);

} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

MicroServiceController::MicroServiceController(boost::asio::io_context& ioContext, std::string const& udsPath)
    : ioContext_(ioContext)
    , udsPath_(udsPath)
    , client_(ioContext_, "MicroServices", udsPath)
{
    client_.subscribeWithCompletion(
            MicroservicesValueIds::BaseValueId, std::bind(&MicroServiceController::subscriptionCompleted, this));
}

MicroServiceController::~MicroServiceController()
{
    // shut all down when ms manager is graciously terminated
    disconnectApplets();
}

void MicroServiceController::subscriptionCompleted()
{
    MicroservicesValueIds systemValues(client_.ws());
    if (!systemValues.isValid()) {
        ErrorLog(MicroServiceControllerLog) << "Subscription failed" << std::endl;
        return;
    }

    // It is safe to create the applet objects to manage individual microservices
    for (auto& child : systemValues.microservices->getChildren()) {

        ValueId appletId(MicroservicesValueIds::BaseValueId, child.first);

        MicroServiceApplet::Ptr appletPtr(
                std::make_shared<MicroServiceApplet>(ioContext_, client_.ws(), appletId, udsPath_));
        appletPtr->start();

        applets_.emplace(applets_.end(), std::move(appletPtr));
    }

    systemValues.microservices->connectChangedListener(
            std::bind(&MicroServiceController::onMicroServices, this, std::placeholders::_1));
}

void MicroServiceController::disconnectApplets()
{
    for (auto& applet : applets_) { applet->stop(); }

    applets_.clear();
}

void MicroServiceController::onMicroServices(VariantValue::Ptr const valuePtr)
{
    if (valuePtr == nullptr) return;

    if (valuePtr->type() == CompanEdgeProtocol::ContainerAddTo) {
        ValueId appletId(MicroservicesValueIds::BaseValueId, valuePtr->get().addtocontainer().key());
        MicroServiceApplet::Ptr appletPtr(
                std::make_shared<MicroServiceApplet>(ioContext_, client_.ws(), appletId, udsPath_));

        appletPtr->start();

        applets_.emplace(applets_.end(), std::move(appletPtr));

        return;
    }

    if (valuePtr->type() == CompanEdgeProtocol::ContainerRemoveFrom) {
        ValueId appletId(MicroservicesValueIds::BaseValueId, valuePtr->get().removefromcontainer().key());

        for (auto iter = applets_.begin(); iter != applets_.end(); ++iter) {
            MicroServiceApplet::Ptr appletPtr = *iter;

            if (appletPtr->appletId() == appletId) {
                appletPtr->stop();
                applets_.erase(iter);
                break;
            }
        }

        return;
    }
}
