/**
  Copyright © 2024 COMPAN REF
  @file test_company_ref_dynamic_dmo_load_actions.cpp
  @brief Tests DMO load/unload functions
*/

#include "company_ref_dynamic_dmo_mock.h"

#include <company_ref_dmo/company_ref_dmo_helper.h>
#include <company_ref_dynamic_dmo/company_ref_dynamic_dmo_load_actions.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>

TEST_F(DynamicDmoTest, LoadActionsLoad)
{
    std::stringstream strm;

    createMockDmoStream(strm);

    EXPECT_TRUE(DmoLoadActions::loadDmo(strm, dmo_, ws_));

    DmoValueStoreHelper dmoHelper(dmo_, ws_);

    EXPECT_TRUE(dmoHelper.insertChild("system.process.running.status.processes", "1"));

    EXPECT_TRUE(ws_.has("system.process.running.status.processes.1.rssSize"));
}

TEST_F(DynamicDmoTest, LoadActionsUnload)
{
    std::stringstream loadStrm;
    createMockDmoStream(loadStrm);
    EXPECT_TRUE(DmoLoadActions::loadDmo(loadStrm, dmo_, ws_));

    DmoValueStoreHelper dmoHelper(dmo_, ws_);

    // we create an instance value, which, will get removed from both the WS and the DMO
    EXPECT_TRUE(dmoHelper.insertChild("system.process.running.status.processes", "1"));

    EXPECT_TRUE(ws_.has("system.process.running.status.processes.1.rssSize"));

    /// Inserts some other value/metaData - to make sure we don't OVER erase
    CompanEdgeProtocol::Value xyzValue;
    xyzValue.set_id("system.xyz");
    valueInit(xyzValue, CompanEdgeProtocol::Text);
    ws_.set(xyzValue);
    dmo_.insertValue(xyzValue);

    EXPECT_TRUE(ws_.has("system.xyz"));
    EXPECT_TRUE(dmo_.has("system.xyz"));

    int removeCount = 0;
    ws_.connectValueRemoveFromContainerListener([&removeCount](VariantValue::Ptr const valuePtr) {
        if (valuePtr->id() == "system.process.running.status.processes") ++removeCount;
    });

    std::stringstream unloadStrm;
    createMockDmoStream(unloadStrm);
    EXPECT_TRUE(DmoLoadActions::unloadDmo(unloadStrm, dmo_, ws_));

    run();

    EXPECT_FALSE(ws_.has("system.process.running.status.processes.1.rssSize"));
    EXPECT_TRUE(ws_.has("system.xyz"));
    EXPECT_TRUE(dmo_.has("system.xyz"));
    EXPECT_EQ(removeCount, 1);
}

TEST_F(DynamicDmoTest, LoadActionsReload)
{
    std::stringstream loadStrm;
    createMockDmoStream(loadStrm);
    EXPECT_TRUE(DmoLoadActions::loadDmo(loadStrm, dmo_, ws_));

    DmoValueStoreHelper dmoHelper(dmo_, ws_);

    // we create an instance value, which, will get removed from both the WS and the DMO
    EXPECT_TRUE(dmoHelper.insertChild("system.process.running.status.processes", "1"));

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

    std::stringstream reloadStrm;
    createMockDmoStream(reloadStrm);
    EXPECT_TRUE(DmoLoadActions::reloadDmo(reloadStrm, dmo_, ws_));

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
