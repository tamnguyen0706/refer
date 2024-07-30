/**
  Copyright © 2024 COMPAN REF
  @file test_company_ref_dynamic_dmo.cpp
  @brief Test DynamicDmo controller
*/

#include "company_ref_dynamic_dmo_mock.h"

#include <company_ref_dmo/company_ref_dmo_helper.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>

#include <company_ref_dynamic_dmo/company_ref_dynamic_dmo_controller.h>
#include <company_ref_sdk_value_ids/dynamic_dmo_value_ids.h>

TEST_F(DynamicDmoTest, ControllerCtorTest)
{
    DynamicDmoController controller(ctx_, ws_, dmo_);

    EXPECT_EQ(controller.dmoPath(), std::string());
    EXPECT_EQ(controller.persistPath(), std::string());

    controller.dmoPath(DmoDirName.generic_string());
    EXPECT_EQ(controller.dmoPath(), DmoDirName);

    controller.persistPath(DmoDirName.generic_string());
    EXPECT_EQ(controller.persistPath(), DmoDirName);
}

TEST_F(DynamicDmoTest, ControllerAddMicroservice)
{
    DynamicDmoController controller(ctx_, ws_, dmo_);
    DmoValueStoreHelper dmoHelper(dmo_, ws_);

    DynamicDmoValueIds dynamicDmoIds(ws_);

    dmoHelper.insertChild(dynamicDmoIds.dmo->id(), MicroServiceName);

    run();
}
