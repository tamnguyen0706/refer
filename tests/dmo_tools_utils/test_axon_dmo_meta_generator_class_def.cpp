/**
  Copyright © 2024 COMPAN REF
  @file test_Compan_dmo_meta_generator_class_def.cpp
  @brief -*-*-*-*-*-
*/

#include "test_dmo_tools_utils_mock.h"

#include <dmo_tools_utils/Compan_dmo_ini_converter.h>
#include <dmo_tools_utils/Compan_dmo_meta_generator_class_def.h>

TEST_F(DmoToolsUtilsMock, MetaClassDefGenerateHeaderFoot)
{
    std::stringstream strm(LoggerDataDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    MetaGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "LoggerMetaData");

    std::stringstream oStrm;
    generator.generateHead(oStrm);
    generator.generateFoot(oStrm);

    std::string const output = "#ifndef __LOGGER_META_DATA_H__\n"
                               "#define __LOGGER_META_DATA_H__\n"
                               "\n"
                               "#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>\n"
                               "#include <company_ref_dmo/company_ref_dmo_container.h>\n"
                               "#include <memory>\n"
                               "\n"
                               "namespace Compan{\n"
                               "namespace Edge {\n"
                               "\n"
                               "} // namespace edge\n"
                               "} // namespace Compan\n"
                               "\n"
                               "#endif // __LOGGER_META_DATA_H__\n"
                               "\n";

    EXPECT_EQ(oStrm.str(), output);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, MetaClassDefGenerateLoggerDataDef)
{
    std::stringstream strm(LoggerDataDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    MetaGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBody(oStrm);

    std::string const output = "// logger\n"
                               "class LoggerMetaData {\n"
                               "public:\n"
                               "    ~LoggerMetaData() = default;\n"
                               "\n"
                               "    /// Creation function for VariantValueStore\n"
                               "    static void create(VariantValueStore& ws, DmoContainer& dmoContainer);\n"
                               "\n"
                               "    /// Creation function for MicroServices\n"
                               "    static void create(VariantValueStore& ws);\n"
                               "};\n"
                               "\n";

    EXPECT_EQ(oStrm.str(), output);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, MetaClassDefGenerateMixedConfigDef)
{
    std::stringstream strm(MixedConfigDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    MetaGeneratorClassDef generator(dmo_);

    generator.baseId(converter.baseId());
    generator.microserviceName(converter.microserviceName());
    generator.className(converter.className());

    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBody(oStrm);

    std::string const output = "// system.ws\n"
                               "class DynamicDmoMetaData {\n"
                               "public:\n"
                               "    ~DynamicDmoMetaData() = default;\n"
                               "\n"
                               "    /// Creation function for VariantValueStore\n"
                               "    static void create(VariantValueStore& ws, DmoContainer& dmoContainer);\n"
                               "\n"
                               "    /// Creation function for MicroServices\n"
                               "    static void create(VariantValueStore& ws);\n"
                               "};\n"
                               "\n";

    EXPECT_EQ(oStrm.str(), output);
    //    std::cout << oStrm.str() << std::endl;
}
