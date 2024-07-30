/**
  Copyright © 2024 COMPAN REF
  @file test_dmo_from_trxml_dmdocument.cpp
  @brief -*-*-*-*-*-
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <Compan_logger/Compan_logger_sink_buffered.h>

#include <dmo_tools_utils/Compan_dmo_from_trxml_dmdocument.h>

using namespace Compan::Edge;

class DmDocumentMock : public testing::Test {
public:
    DmDocumentMock()
    {
    }
    virtual ~DmDocumentMock()
    {
        EXPECT_TRUE(coutWrapper_.empty());

        if (!coutWrapper_.empty()) std::cout << coutWrapper_.pop() << std::endl;
    }

    DmoContainer dmo_;

    CompanLoggerSinkBuffered coutWrapper_;
};

TEST_F(DmDocumentMock, DeclaredDataTypes)
{
    DmDocument dmDocument(dmo_);

    ASSERT_TRUE(dmDocument.parse("./xml_snippets/dm_declared_datatypes.xml"));

    // DMO container should not have any definitions in it when only reading the declared
    //  data types
    ASSERT_TRUE(dmo_.empty());

    DmDocument::DeclaredDataTypes declaredDataTypes = dmDocument.getDeclaredDataTypes();

    // This count comes from the snippet file
    ASSERT_EQ(declaredDataTypes.size(), 19);

    ASSERT_EQ(declaredDataTypes["CommonType"].type(), CompanEdgeProtocol::Text);
    ASSERT_EQ(declaredDataTypes["CommonDerived"].type(), CompanEdgeProtocol::Text);
    ASSERT_EQ(declaredDataTypes["CommonDoubleDerived"].type(), CompanEdgeProtocol::Text);
    ASSERT_EQ(declaredDataTypes["PatternAttrib"].type(), CompanEdgeProtocol::Text);

    ASSERT_EQ(declaredDataTypes["Int"].type(), CompanEdgeProtocol::Interval);
    ASSERT_EQ(declaredDataTypes["UnsignedInt"].type(), CompanEdgeProtocol::UInterval);

    ASSERT_EQ(declaredDataTypes["Long"].type(), CompanEdgeProtocol::LLInterval);
    ASSERT_EQ(declaredDataTypes["UnsignedLong"].type(), CompanEdgeProtocol::ULLInterval);

    ASSERT_EQ(declaredDataTypes["RangeMinAttrib"].type(), CompanEdgeProtocol::Interval);
    ASSERT_EQ(declaredDataTypes["RangeMinAttrib"].intervalvalue().min(), 100);
    ASSERT_EQ(declaredDataTypes["RangeMinAttrib"].intervalvalue().max(), 2147483647);

    ASSERT_EQ(declaredDataTypes["RangeMaxAttrib"].type(), CompanEdgeProtocol::Interval);
    ASSERT_EQ(declaredDataTypes["RangeMaxAttrib"].intervalvalue().min(), -2147483648);
    ASSERT_EQ(declaredDataTypes["RangeMaxAttrib"].intervalvalue().max(), 100);

    ASSERT_EQ(declaredDataTypes["RangeMinMaxAttrib"].type(), CompanEdgeProtocol::Interval);
    ASSERT_EQ(declaredDataTypes["RangeMinMaxAttrib"].intervalvalue().min(), 1);
    ASSERT_EQ(declaredDataTypes["RangeMinMaxAttrib"].intervalvalue().max(), 65535);

    ASSERT_EQ(declaredDataTypes["EnumType"].type(), CompanEdgeProtocol::Enum);

    {
        CompanValueTypes::EnumValue enumValue = declaredDataTypes["EnumType"].enumvalue();
        ASSERT_EQ(enumValue.enumerators(0).text(), std::string("Zero"));
        ASSERT_EQ(enumValue.enumerators(1).text(), std::string("One"));
        ASSERT_EQ(enumValue.enumerators(2).text(), std::string("Two"));
    }

    ASSERT_EQ(declaredDataTypes["BoolType"].type(), CompanEdgeProtocol::Bool);
    ASSERT_EQ(declaredDataTypes["DateTimeType"].type(), CompanEdgeProtocol::TimeVal);
    ASSERT_EQ(declaredDataTypes["HexBinaryType"].type(), CompanEdgeProtocol::Text);
    ASSERT_EQ(declaredDataTypes["Base64Type"].type(), CompanEdgeProtocol::Text);

    ASSERT_EQ(declaredDataTypes["DecimalType"].type(), CompanEdgeProtocol::DInterval);

    ASSERT_EQ(declaredDataTypes["ListType"].type(), CompanEdgeProtocol::UnorderedSet);

    {
        CompanValueTypes::EnumValue enumValue = declaredDataTypes["BadEnumValues"].enumvalue();
        ASSERT_EQ(enumValue.enumerators_size(), 3);
        ASSERT_EQ(enumValue.enumerators(0).text(), std::string("_EMPTY_"));
        ASSERT_EQ(enumValue.enumerators(1).text(), std::string("Wildcard"));
        ASSERT_EQ(enumValue.enumerators(2).text(), std::string("Nothing"));
    }
}

TEST_F(DmDocumentMock, ModelObjects)
{
    DmDocument dmDocument(dmo_);

    ASSERT_TRUE(dmDocument.parse("./xml_snippets/dm_model_objects.xml"));

    ASSERT_EQ(dmo_.getValueDefinition("modelName.config.BoolValue").type(), CompanEdgeProtocol::Bool);
    ASSERT_EQ(dmo_.getValueDefinition("modelName.config.StringValue").type(), CompanEdgeProtocol::Text);
    ASSERT_EQ(dmo_.getValueDefinition("modelName.config.IpValue").type(), CompanEdgeProtocol::IPv4);
    ASSERT_EQ(dmo_.getValueDefinition("modelName.config.UnsignedIntValue").type(), CompanEdgeProtocol::UInterval);
    ASSERT_EQ(dmo_.getValueDefinition("modelName.config.DateTimeValue").type(), CompanEdgeProtocol::TimeVal);
    ASSERT_EQ(dmo_.getValueDefinition("modelName.config.EnumValue").type(), CompanEdgeProtocol::Enum);
    ASSERT_EQ(dmo_.getValueDefinition("modelName.config.EnumDataRefValue").type(), CompanEdgeProtocol::Enum);

    {
        CompanValueTypes::EnumValue enumValue = dmo_.getValueDefinition("modelName.config.EnumValue").enumvalue();
        ASSERT_EQ(enumValue.enumerators_size(), 3);
        ASSERT_EQ(enumValue.enumerators(0).text(), std::string("Zero"));
        ASSERT_EQ(enumValue.enumerators(1).text(), std::string("1"));
        ASSERT_EQ(enumValue.enumerators(2).text(), std::string("T wo"));
    }

    {
        CompanValueTypes::EnumValue enumValue = dmo_.getValueDefinition("modelName.config.EnumDataRefValue").enumvalue();
        ASSERT_EQ(enumValue.enumerators_size(), 7);
        ASSERT_EQ(enumValue.enumerators(0).text(), std::string("None"));
        ASSERT_EQ(enumValue.enumerators(1).text(), std::string("One"));
        ASSERT_EQ(enumValue.enumerators(2).text(), std::string("Two"));
        ASSERT_EQ(enumValue.enumerators(3).text(), std::string("Three"));
        ASSERT_EQ(enumValue.enumerators(4).text(), std::string("Four"));
        ASSERT_EQ(enumValue.enumerators(5).text(), std::string("Five"));
        ASSERT_EQ(enumValue.enumerators(6).text(), std::string("Six"));
    }
}

TEST_F(DmDocumentMock, DuplicateEntries)
{
    DmDocument dmDocument(dmo_);

    ASSERT_TRUE(dmDocument.parse("./xml_snippets/dm_duplicate_entries.xml"));

    ASSERT_EQ(dmo_.getValueDefinition("modelName.config.BoolValue").type(), CompanEdgeProtocol::Bool);

    EXPECT_EQ(
            coutWrapper_.pop(),
            "dmdocument: Error: Duplicate parameter: modelName.config.BoolValue\n"
            "dmdocument: Error: Duplicate parameter: modelName.config.BoolValue\n"
            "dmdocument: Error: Duplicate object: modelName.config\n");
}
