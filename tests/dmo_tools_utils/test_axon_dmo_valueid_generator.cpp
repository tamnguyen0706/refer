/**
  Copyright © 2024 COMPAN REF
  @file test_Compan_dmo_valueid_generator.cpp
  @brief Test ValueId code generator static functions
*/

#include "test_dmo_tools_utils_mock.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <dmo_tools_utils/Compan_dmo_ini_converter.h>
#include <dmo_tools_utils/Compan_dmo_valueid_generator.h>

class ValueIdGeneratorTest : public ValueIdGenerator {
public:
    ValueIdGeneratorTest(DmoContainer& dmo)
        : ValueIdGenerator(dmo, "ValueIds")
    {
    }

    bool doPreGeneration()
    {
        includePaths_.push_back("string");
        return ValueIdGenerator::doPreGeneration();
    }

    virtual void generateHead(std::ostream& strm)
    {
        strm << __FUNCTION__ << std::endl;
    }

    virtual void generateBody(std::ostream& strm)
    {
        strm << __FUNCTION__ << std::endl;
    }

    virtual void generateFoot(std::ostream& strm)
    {
        strm << __FUNCTION__ << std::endl;
    }

    virtual void generateClass(ValueId const&, DmoContainer::TreeType const&, std::ostream& strm, int const)
    {
        strm << __FUNCTION__ << std::endl;
    }

    virtual void generateClassMembers(ValueId const&, DmoContainer::TreeType const&, std::ostream& strm, int const)
    {
        strm << __FUNCTION__ << std::endl;
    }

    virtual void generateClassInstance(ValueId const&, DmoContainer::TreeType const&, std::ostream& strm, int const)
    {
        strm << __FUNCTION__ << std::endl;
    }
};

TEST_F(DmoToolsUtilsMock, CamelCase)
{
    EXPECT_EQ(ValueIdGenerator::camelCase("camel"), "Camel");
    EXPECT_EQ(ValueIdGenerator::camelCase("camel_"), "Camel_");
    EXPECT_EQ(ValueIdGenerator::camelCase("camel_case"), "Camel_Case");
}

TEST_F(DmoToolsUtilsMock, MakeValueTypeClassName)
{
    EXPECT_EQ(ValueIdGenerator::makeValueTypeClassName(CompanEdgeProtocol::Text), "VariantTextValue");
}

TEST_F(DmoToolsUtilsMock, ConvertParent)
{
    EXPECT_EQ(ValueIdGenerator::convertParent(ValueId("a")), "a::");
    EXPECT_EQ(ValueIdGenerator::convertParent(ValueId("a.b")), "a::b::");
}

TEST_F(DmoToolsUtilsMock, MakeNormalizedName)
{
    EXPECT_EQ(ValueIdGenerator::makeNormalizedName("one"), "one");
    EXPECT_EQ(ValueIdGenerator::makeNormalizedName("one-two"), "one_two");
    EXPECT_EQ(ValueIdGenerator::makeNormalizedName("one-two-three"), "one_two_three");
}

TEST_F(DmoToolsUtilsMock, GetMemberName)
{
    CompanEdgeProtocol::Value value;

    value.set_type(CompanEdgeProtocol::Struct);

    value.set_id("nada");
    EXPECT_EQ(ValueIdGenerator::getMemberName(value), "nada");

    value.set_id("na-da");
    EXPECT_EQ(ValueIdGenerator::getMemberName(value), "na_da");
}

TEST_F(DmoToolsUtilsMock, GetMemberType)
{
    CompanEdgeProtocol::Value value;

    value.set_id("nada");
    value.set_type(CompanEdgeProtocol::Text);

    EXPECT_EQ(ValueIdGenerator::getMemberType(value), "VariantTextValue");

    value.set_type(CompanEdgeProtocol::Struct);
    EXPECT_EQ(ValueIdGenerator::getMemberType(value), "Nada");

    value.set_id("na-da");
    EXPECT_EQ(ValueIdGenerator::getMemberType(value), "Na_Da");
}

TEST_F(DmoToolsUtilsMock, OuterClassName1)
{
    std::stringstream strm(SingleDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;
    ValueId className = ValueIdGenerator::getOuterClassName(parentId, dmo_.root());
    EXPECT_EQ(className.name(), "a");
}

TEST_F(DmoToolsUtilsMock, OuterClassName2)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;
    ValueId className = ValueIdGenerator::getOuterClassName(parentId, dmo_.root());
    EXPECT_EQ(className.name(), "a");
}

TEST_F(DmoToolsUtilsMock, OuterClassName3)
{
    std::stringstream strm(WideDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;
    ValueId className = ValueIdGenerator::getOuterClassName(parentId, dmo_.root());
    EXPECT_EQ(className.name(), "a.b");
}

TEST_F(DmoToolsUtilsMock, OuterClassName4)
{
    std::stringstream strm(Enums);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;
    ValueId className = ValueIdGenerator::getOuterClassName(parentId, dmo_.root());
    EXPECT_EQ(className.name(), "a");
}

TEST_F(DmoToolsUtilsMock, OuterClassName5)
{
    std::stringstream strm(DataDefs);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;
    ValueId className = ValueIdGenerator::getOuterClassName(parentId, dmo_.root());
    EXPECT_EQ(className.name(), "a");
}

TEST_F(DmoToolsUtilsMock, OuterClassName6)
{
    std::stringstream strm(LoggerDataDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;
    ValueId className = ValueIdGenerator::getOuterClassName(parentId, dmo_.root());
    EXPECT_EQ(className.name(), "logger");
}

TEST_F(DmoToolsUtilsMock, MakeFilename1)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);

    generator.fileName("hardware_system_load");

    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.fileName(), "hardware_system_load_value_ids");
}

TEST_F(DmoToolsUtilsMock, MakeFilename2)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);

    generator.fileName("tr098_orchestrator");

    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.fileName(), "tr098_orchestrator_value_ids");
}

TEST_F(DmoToolsUtilsMock, PreGenerationClassName1)
{
    std::stringstream strm(SingleDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "AValueIds");
    EXPECT_EQ(generator.fileName(), "a_value_ids");
    EXPECT_EQ(generator.baseId(), ValueId("a"));

    EXPECT_TRUE(generator.microserviceName().empty());
}

TEST_F(DmoToolsUtilsMock, PreGenerationClassName2)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "AValueIds");
    EXPECT_EQ(generator.fileName(), "a_value_ids");
    EXPECT_EQ(generator.baseId(), ValueId("a"));

    EXPECT_TRUE(generator.microserviceName().empty());
}

TEST_F(DmoToolsUtilsMock, PreGenerationClassName3)
{
    std::stringstream strm(WideDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "ABValueIds");
    EXPECT_EQ(generator.fileName(), "a_b_value_ids");
    EXPECT_EQ(generator.baseId(), ValueId("a.b"));
    EXPECT_TRUE(generator.microserviceName().empty());
}

TEST_F(DmoToolsUtilsMock, PreGenerationClassName4)
{
    std::stringstream strm(WideDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "ABValueIds");
    EXPECT_EQ(generator.fileName(), "a_b_value_ids");
    EXPECT_EQ(generator.baseId(), ValueId("a.b"));
    EXPECT_TRUE(generator.microserviceName().empty());
}

TEST_F(DmoToolsUtilsMock, PreGenerationClassName5)
{
    std::stringstream strm(DataDefs);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "AValueIds");
    EXPECT_EQ(generator.fileName(), "a_value_ids");
    EXPECT_EQ(generator.baseId(), ValueId("a"));
    EXPECT_TRUE(generator.microserviceName().empty());
}

TEST_F(DmoToolsUtilsMock, PreGenerationClassName6)
{
    std::stringstream strm(LoggerDataDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "LoggerValueIds");
    EXPECT_EQ(generator.fileName(), "logger_value_ids");
    EXPECT_EQ(generator.baseId(), ValueId("logger"));
    EXPECT_TRUE(generator.microserviceName().empty());
}

TEST_F(DmoToolsUtilsMock, GenerateIncludes)
{
    std::stringstream strm(WideDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateIncludes(oStrm);

    EXPECT_EQ(oStrm.str(), "#include <string>\n\n");
}

TEST_F(DmoToolsUtilsMock, Generate)
{
    std::stringstream strm(WideDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generate(oStrm);

    EXPECT_EQ(
            oStrm.str(),
            "generateHead\n"
            "generateBody\n"
            "generateFoot\n");
}

TEST_F(DmoToolsUtilsMock, GeneratorConfigDmocDef)
{
    std::stringstream strm(ConfigDmocDef + WideDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorTest generator(dmo_);

    generator.baseId(converter.baseId());
    generator.microserviceName(converter.microserviceName());
    generator.className(converter.className());

    std::stringstream oStrm;
    EXPECT_TRUE(generator.generate(oStrm));

    EXPECT_EQ(generator.className(), "SomethingElseEntirelyValueIds");
    EXPECT_EQ(generator.fileName(), "something_else_entirely_value_ids");
    EXPECT_EQ(generator.baseId(), ValueId("something.else"));
    EXPECT_EQ(generator.microserviceName(), "Something");
}
