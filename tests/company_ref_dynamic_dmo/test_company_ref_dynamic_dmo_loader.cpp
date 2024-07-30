/**
  Copyright © 2024 COMPAN REF
  @file test_company_ref_dynamic_dmo_loader.cpp
  @brief Test VariantValue helper class for Config/Status actions
*/

#include "company_ref_dynamic_dmo_mock.h"

#include "company_ref_dynamic_dmo_loader.h"

#include <company_ref_sdk_value_ids/dynamic_dmo_value_ids.h>

#include <company_ref_dmo/company_ref_dmo_helper.h>
#include <company_ref_dynamic_dmo/company_ref_dynamic_dmo_load_actions.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>

#include <company_ref_variant_valuestore/company_ref_variant_enum_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>

class DynamicDmoLoaderTest : public DynamicDmoTest {
public:
    DynamicDmoLoaderTest()
        : DynamicDmoTest()
        , dmoHelper_(dmo_, ws_)
    {
        createMockDmoFile();

        dmoHelper_.insertChild("system.ws.dmo", MicroServiceName);
    }

    virtual ~DynamicDmoLoaderTest() = default;

    DmoValueStoreHelper dmoHelper_;
};

TEST_F(DynamicDmoLoaderTest, Fail)
{
    DynamicDmoValueIds dynamicDmoIds(ws_);

    DynamicDmoValueIds::Dmo microServiceDmo(ws_, dynamicDmoIds.dmo->id(), MicroServiceName);

    dynamicDmoIds.dmoPath->set(DmoDirName.generic_string());
    microServiceDmo.config->path->set("none");

    DynamicDmoLoader::Ptr dmoLoader(std::make_shared<DynamicDmoLoader>(
            ctx_,
            ws_,
            dmo_,
            dynamicDmoIds.dmoPath,
            microServiceDmo.config->path,
            microServiceDmo.config->action,
            microServiceDmo.status->action));

    dmoLoader->init();

    {
        microServiceDmo.config->action->set(DynamicDmoValueIds::Dmo ::Config ::Load);
        run();

        EXPECT_EQ(microServiceDmo.status->action->get(), DynamicDmoValueIds::Dmo::Status::FileNotFound);
        EXPECT_EQ(coutWrapper_.pop(), "dynamicdmo.loader: Warning: \"./none\" doesn't exist\n");

        microServiceDmo.status->action->set(DynamicDmoValueIds::Dmo::Status::None);
    }

    {
        microServiceDmo.config->action->set(DynamicDmoValueIds::Dmo ::Config ::Unload);
        run();
        EXPECT_EQ(microServiceDmo.status->action->get(), DynamicDmoValueIds::Dmo::Status::FileNotFound);
        EXPECT_EQ(coutWrapper_.pop(), "dynamicdmo.loader: Warning: \"./none\" doesn't exist\n");

        microServiceDmo.status->action->set(DynamicDmoValueIds::Dmo::Status::None);
    }

    {
        microServiceDmo.config->action->set(DynamicDmoValueIds::Dmo ::Config ::OverWrite);
        run();

        EXPECT_EQ(microServiceDmo.status->action->get(), DynamicDmoValueIds::Dmo::Status::FileNotFound);
        EXPECT_EQ(coutWrapper_.pop(), "dynamicdmo.loader: Warning: \"./none\" doesn't exist\n");
    }
}

TEST_F(DynamicDmoLoaderTest, Load)
{
    DynamicDmoValueIds dynamicDmoIds(ws_);

    DynamicDmoValueIds::Dmo microServiceDmo(ws_, dynamicDmoIds.dmo->id(), MicroServiceName);

    dynamicDmoIds.dmoPath->set(DmoDirName.generic_string());
    microServiceDmo.config->path->set(DmoFileName.generic_string());

    DynamicDmoLoader::Ptr dmoLoader(std::make_shared<DynamicDmoLoader>(
            ctx_,
            ws_,
            dmo_,
            dynamicDmoIds.dmoPath,
            microServiceDmo.config->path,
            microServiceDmo.config->action,
            microServiceDmo.status->action));

    dmoLoader->init();

    microServiceDmo.status->action->connectChangedListener([](VariantValue::Ptr const value) {
        EXPECT_EQ(std::dynamic_pointer_cast<VariantEnumValue>(value)->get(), DynamicDmoValueIds::Dmo::Status::Complete);
    });

    microServiceDmo.config->action->set(DynamicDmoValueIds::Dmo ::Config ::Load);

    run();

    EXPECT_TRUE(dmoHelper_.insertChild("system.process.running.status.processes", "1"));
    EXPECT_TRUE(ws_.has("system.process.running.status.processes.1.rssSize"));
}

TEST_F(DynamicDmoLoaderTest, Unload)
{
    // load up the DMO/WS
    std::stringstream strm;
    createMockDmoStream(strm);
    EXPECT_TRUE(DmoLoadActions::loadDmo(strm, dmo_, ws_));
    EXPECT_TRUE(dmoHelper_.insertChild("system.process.running.status.processes", "1"));
    EXPECT_TRUE(ws_.has("system.process.running.status.processes.1.rssSize"));

    DynamicDmoValueIds dynamicDmoIds(ws_);
    DynamicDmoValueIds::Dmo microServiceDmo(ws_, dynamicDmoIds.dmo->id(), MicroServiceName);

    dynamicDmoIds.dmoPath->set(DmoDirName.generic_string());
    microServiceDmo.config->path->set(DmoFileName.generic_string());

    DynamicDmoLoader::Ptr dmoLoader(std::make_shared<DynamicDmoLoader>(
            ctx_,
            ws_,
            dmo_,
            dynamicDmoIds.dmoPath,
            microServiceDmo.config->path,
            microServiceDmo.config->action,
            microServiceDmo.status->action));

    dmoLoader->init();

    microServiceDmo.status->action->connectChangedListener([](VariantValue::Ptr const value) {
        EXPECT_EQ(std::dynamic_pointer_cast<VariantEnumValue>(value)->get(), DynamicDmoValueIds::Dmo::Status::Complete);
    });

    microServiceDmo.config->action->set(DynamicDmoValueIds::Dmo ::Config ::Unload);
    run();

    EXPECT_FALSE(ws_.has("system.process.running.status.processes.1.rssSize"));
    EXPECT_TRUE(ws_.has("system.ws"));
    EXPECT_TRUE(dmo_.has("system.ws"));
}

TEST_F(DynamicDmoLoaderTest, OverWrite)
{
    // load up the DMO/WS
    std::stringstream strm;
    createMockDmoStream(strm);
    EXPECT_TRUE(DmoLoadActions::loadDmo(strm, dmo_, ws_));
    EXPECT_TRUE(dmoHelper_.insertChild("system.process.running.status.processes", "1"));
    EXPECT_TRUE(ws_.has("system.process.running.status.processes.1.rssSize"));

    {
        VariantUIntervalValue::Ptr overwritePtr =
                ws_.get<VariantUIntervalValue>("system.process.monitor.config.pollingInterval");
        ASSERT_NE(overwritePtr, nullptr);
        overwritePtr->set(1);
        EXPECT_EQ(overwritePtr->get(), 1);
    }

    {
        VariantMapValue::Ptr mapPtr = ws_.get<VariantMapValue>("system.process.running.status.processes");
        ASSERT_NE(mapPtr, nullptr);
        EXPECT_EQ(mapPtr->size(), 1);
        EXPECT_FALSE(mapPtr->empty());
    }

    DynamicDmoValueIds dynamicDmoIds(ws_);
    DynamicDmoValueIds::Dmo microServiceDmo(ws_, dynamicDmoIds.dmo->id(), MicroServiceName);

    dynamicDmoIds.dmoPath->set(DmoDirName.generic_string());
    microServiceDmo.config->path->set(DmoFileName.generic_string());

    DynamicDmoLoader::Ptr dmoLoader(std::make_shared<DynamicDmoLoader>(
            ctx_,
            ws_,
            dmo_,
            dynamicDmoIds.dmoPath,
            microServiceDmo.config->path,
            microServiceDmo.config->action,
            microServiceDmo.status->action));
    dmoLoader->init();

    microServiceDmo.status->action->connectChangedListener([](VariantValue::Ptr const value) {
        EXPECT_EQ(std::dynamic_pointer_cast<VariantEnumValue>(value)->get(), DynamicDmoValueIds::Dmo::Status::Complete);
    });

    microServiceDmo.config->action->set(DynamicDmoValueIds::Dmo ::Config ::OverWrite);
    run();

    EXPECT_FALSE(ws_.has("system.process.running.status.processes.1.rssSize"));
    EXPECT_TRUE(ws_.has("system.process.running.status.processes"));

    {
        VariantUIntervalValue::Ptr overwritePtr =
                ws_.get<VariantUIntervalValue>("system.process.monitor.config.pollingInterval");
        EXPECT_EQ(overwritePtr->get(), 0);
    }

    {
        VariantMapValue::Ptr mapPtr = ws_.get<VariantMapValue>("system.process.running.status.processes");
        ASSERT_NE(mapPtr, nullptr);
        EXPECT_TRUE(mapPtr->empty());
    }
}
