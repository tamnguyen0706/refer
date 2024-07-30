/**
  Copyright © 2024 COMPAN REF
  @file test_Compan_dmo_ini_converter.cpp
  @brief Test Create a DMO container from an IniConfigFile
*/

#include "test_dmo_tools_utils_mock.h"

#include <dmo_tools_utils/Compan_dmo_ini_converter.h>

#include <company_ref_protocol_utils/company_ref_stream.h>

#include <sstream>

TEST_F(DmoToolsUtilsMock, SingleDepthValues)
{
    std::stringstream strm(SingleDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    EXPECT_TRUE(dmo_.has("a.interval"));
    EXPECT_TRUE(dmo_.has("a.bool"));
    EXPECT_EQ(dmo_.getValueDefinition("a.interval").type(), CompanEdgeProtocol::UInterval);
    EXPECT_EQ(dmo_.getValueDefinition("a.bool").type(), CompanEdgeProtocol::Bool);
}

TEST_F(DmoToolsUtilsMock, MultiDepthValues)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    EXPECT_TRUE(dmo_.has("a.b1.interval"));
    EXPECT_TRUE(dmo_.has("a.b1.bool"));
    EXPECT_TRUE(dmo_.has("a.b2.text"));

    EXPECT_EQ(dmo_.getValueDefinition("a.b1.interval").type(), CompanEdgeProtocol::UInterval);
    EXPECT_EQ(dmo_.getValueDefinition("a.b1.bool").type(), CompanEdgeProtocol::Bool);
    EXPECT_EQ(dmo_.getValueDefinition("a.b2.text").type(), CompanEdgeProtocol::Text);
}

TEST_F(DmoToolsUtilsMock, ConfigDmocDef)
{
    std::stringstream strm(ConfigDmocDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    EXPECT_EQ(converter.baseId(), ValueId("something.else"));
    EXPECT_EQ(converter.className(), std::string("SomethingElseEntirely"));
    EXPECT_EQ(converter.microserviceName(), std::string("Something"));
}

TEST_F(DmoToolsUtilsMock, DerivedDataDef)
{
    std::stringstream strm(DerivedDataDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    {
        ASSERT_TRUE(dmo_.has("Compan.vectorValue"));
        CompanEdgeProtocol::Value argValue(dmo_.getValueDefinition("Compan.vectorValue"));
        EXPECT_EQ(argValue.type(), CompanEdgeProtocol::Vector);

        std::stringstream argStream;
        argStream << argValue;
        EXPECT_EQ(argStream.str(), "vectorValue (Vector): {1,2,3,2,3,1}");
    }

    {
        ASSERT_TRUE(dmo_.has("Compan.setValue"));
        CompanEdgeProtocol::Value argValue(dmo_.getValueDefinition("Compan.setValue"));
        EXPECT_EQ(argValue.type(), CompanEdgeProtocol::Set);

        std::stringstream argStream;
        argStream << argValue;
        EXPECT_EQ(argStream.str(), "setValue (Set): {a,b,c}");
    }

    {
        ASSERT_TRUE(dmo_.has("Compan.unorderedSetValue"));
        CompanEdgeProtocol::Value argValue(dmo_.getValueDefinition("Compan.unorderedSetValue"));
        EXPECT_EQ(argValue.type(), CompanEdgeProtocol::UnorderedSet);

        std::stringstream argStream;
        argStream << argValue;
        EXPECT_EQ(argStream.str(), "unorderedSetValue (UnorderedSet): {y,z,x}");
    }
}
