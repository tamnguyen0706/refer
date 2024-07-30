/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_factory.cpp
  @brief -*-*-*-*-*-
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Compan_logger_sink_buffered.h"
#include <company_ref_main_apps/Compan_logger_factory.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <sstream>

using namespace Compan::Edge;

TEST(LoggerFactory, TestAutoCreate)
{
    boost::asio::io_context ctx;
    VariantValueStore ws(ctx);

    CompanEdgeProtocol::Value logger;
    logger.set_id("logger");
    logger.set_access(CompanEdgeProtocol::Value::ReadWrite);
    logger.set_type(CompanEdgeProtocol::Container);
    logger.mutable_unknownvalue()->set_value(std::string("MapValue"));
    ws.set(logger);

    LoggerFactory::createBinders(__FUNCTION__, ws, false);

    EXPECT_TRUE(ws.has("logger.TestBody.variantvalue.data"));
    EXPECT_TRUE(ws.has("logger.TestBody.variantvalue.store"));
}
