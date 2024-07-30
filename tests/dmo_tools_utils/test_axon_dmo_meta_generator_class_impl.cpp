/**
  Copyright © 2024 COMPAN REF
  @file test_Compan_dmo_meta_generator_class_impl.cpp
  @brief -*-*-*-*-*-
*/

#include "test_dmo_tools_utils_mock.h"

#include <dmo_tools_utils/Compan_dmo_ini_converter.h>
#include <dmo_tools_utils/Compan_dmo_meta_generator_class_impl.h>

TEST_F(DmoToolsUtilsMock, MetaClassImplGenerateHeaderFoot)
{
    std::stringstream strm(LoggerDataDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    MetaGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "LoggerMetaData");

    std::stringstream oStrm;
    generator.generateHead(oStrm);
    generator.generateFoot(oStrm);

    std::string const output = "#include \"logger_meta_data.h\"\n"
                               "\n"
                               "#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h>\n"
                               "#include <company_ref_protocol_utils/company_ref_pb_init.h>\n"
                               "#include <company_ref_protocol_utils/company_ref_pb_enum.h>\n"
                               "#include <company_ref_protocol_utils/company_ref_pb_accesors.h>\n"
                               "#include <Compan_logger/Compan_logger.h>\n"
                               "#include <string>\n"
                               "#include <set>\n"
                               "\n"
                               "using namespace Compan::Edge;\n"
                               "\n"
                               "CompanLogger LoggerMetaDataLog(\"metadata.logger\");\n"
                               "\n";

    EXPECT_EQ(oStrm.str(), output);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, MetaClassImplGenerateLoggerDataDef)
{
    std::stringstream strm(LoggerDataDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    MetaGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    EXPECT_EQ(generator.className(), "LoggerMetaData");

    std::stringstream oStrm;
    generator.generateBody(oStrm);

    std::string const output =
            "namespace {\n"
            "\n"
            "// logger\n"
            "ValueId const loggerId(\"logger\");\n"
            "\n"
            "// logger.+\n"
            "ValueId const logger_Id(loggerId, \"+\");\n"
            "\n"
            "// logger.+.+\n"
            "ValueId const logger__Id(logger_Id, \"+\");\n"
            "\n"
            "CompanEdgeProtocol::Value createValueWrapper( ValueId const & id, CompanEdgeProtocol::Value_Type const type ) "
            "{\n"
            "    CompanEdgeProtocol::Value value;\n"
            "    valueInit(value, type);\n"
            "    value.set_id(id.name());\n"
            "    return value;\n"
            "}\n"
            "\n"
            "void createMetaData(DmoContainer& dmo) {\n"
            "\n"
            "    // logger.+\n"
            "    dmo.insertMetaData(loggerId, createValueWrapper(loggerId, CompanEdgeProtocol::Container));\n"
            "\n"
            "    // logger.+.+\n"
            "    CompanEdgeProtocol::Value logger__Value;\n"
            "    // double container type\n"
            "    dmo.setMetaDataType(logger_Id, CompanEdgeProtocol::Container);\n"
            "\n"
            "    logger__Value.set_id(logger__Id);\n"
            "    std::set<std::pair<uint32_t, std::string>> logger__Enums({\n"
            "        {0, \"None\"},\n"
            "        {1, \"Error\"},\n"
            "        {2, \"Warning\"},\n"
            "        {3, \"Information\"},\n"
            "        {4, \"State\"},\n"
            "        {5, \"Debug\"},\n"
            "        {6, \"Trace\"},\n"
            "    });\n"
            "    valueInit(logger__Value, logger__Enums);\n"
            "    valueSet<std::string>(logger__Value, \"Information\");\n"
            "    dmo.insertMetaData(logger__Id, logger__Value);\n"
            "}\n"
            "\n"
            "} // namespace\n"
            "\n"
            "void LoggerMetaData::create(VariantValueStore& ws, DmoContainer& dmo) {\n"
            "    create(ws);\n"
            "    createMetaData(dmo);\n"
            "}\n"
            "\n"
            "void LoggerMetaData::create(VariantValueStore& ws) {\n"
            "    ws.set(createValueWrapper(loggerId, CompanEdgeProtocol::Container), VariantValue::Remote);\n"
            "}\n"
            "\n";

    EXPECT_EQ(oStrm.str(), output);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, MetaClassImplGenerateMixedConfigDef)
{
    std::stringstream strm(MixedConfigDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    MetaGeneratorClassImpl generator(dmo_);

    generator.baseId(converter.baseId());
    generator.microserviceName(converter.microserviceName());
    generator.className(converter.className());

    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBody(oStrm);

    std::string const output =
            "namespace {\n"
            "\n"
            "// system\n"
            "ValueId const systemId(\"system\");\n"
            "\n"
            "// system.ws\n"
            "ValueId const systemWsId(systemId, \"ws\");\n"
            "\n"
            "// system.ws.dmo\n"
            "ValueId const systemWsDmoId(systemWsId, \"dmo\");\n"
            "\n"
            "// system.ws.dmo.+.config\n"
            "ValueId const systemWsDmo_ConfigId(ValueId(systemWsDmoId, \"+\"), \"config\");\n"
            "\n"
            "// system.ws.dmo.+.config.action\n"
            "ValueId const systemWsDmo_ConfigActionId(systemWsDmo_ConfigId, \"action\");\n"
            "\n"
            "// system.ws.dmo.+.config.path\n"
            "ValueId const systemWsDmo_ConfigPathId(systemWsDmo_ConfigId, \"path\");\n"
            "\n"
            "// system.ws.dmo.+.persisted\n"
            "ValueId const systemWsDmo_PersistedId(ValueId(systemWsDmoId, \"+\"), \"persisted\");\n"
            "\n"
            "// system.ws.dmo.+.persisted.+.config\n"
            "ValueId const systemWsDmo_Persisted_ConfigId(ValueId(systemWsDmo_PersistedId, \"+\"), \"config\");\n"
            "\n"
            "// system.ws.dmo.+.persisted.+.config.action\n"
            "ValueId const systemWsDmo_Persisted_ConfigActionId(systemWsDmo_Persisted_ConfigId, \"action\");\n"
            "\n"
            "// system.ws.dmo.+.persisted.+.config.path\n"
            "ValueId const systemWsDmo_Persisted_ConfigPathId(systemWsDmo_Persisted_ConfigId, \"path\");\n"
            "\n"
            "// system.ws.dmo.+.persisted.+.status\n"
            "ValueId const systemWsDmo_Persisted_StatusId(ValueId(systemWsDmo_PersistedId, \"+\"), \"status\");\n"
            "\n"
            "// system.ws.dmo.+.persisted.+.status.action\n"
            "ValueId const systemWsDmo_Persisted_StatusActionId(systemWsDmo_Persisted_StatusId, \"action\");\n"
            "\n"
            "// system.ws.dmo.+.status\n"
            "ValueId const systemWsDmo_StatusId(ValueId(systemWsDmoId, \"+\"), \"status\");\n"
            "\n"
            "// system.ws.dmo.+.status.action\n"
            "ValueId const systemWsDmo_StatusActionId(systemWsDmo_StatusId, \"action\");\n"
            "\n"
            "// system.ws.dmoPath\n"
            "ValueId const systemWsDmoPathId(systemWsId, \"dmoPath\");\n"
            "\n"
            "// system.ws.persistPath\n"
            "ValueId const systemWsPersistPathId(systemWsId, \"persistPath\");\n"
            "\n"
            "CompanEdgeProtocol::Value createValueWrapper( ValueId const & id, CompanEdgeProtocol::Value_Type const type ) "
            "{\n"
            "    CompanEdgeProtocol::Value value;\n"
            "    valueInit(value, type);\n"
            "    value.set_id(id.name());\n"
            "    return value;\n"
            "}\n"
            "\n"
            "void createMetaData(DmoContainer& dmo) {\n"
            "\n"
            "    // system.ws.dmo\n"
            "    dmo.insertMetaData(systemWsDmoId, createValueWrapper(systemWsDmoId, CompanEdgeProtocol::Container));\n"
            "\n"
            "    // system.ws.dmo.+.config.action\n"
            "    CompanEdgeProtocol::Value systemWsDmo_ConfigActionValue;\n"
            "    systemWsDmo_ConfigActionValue.set_id(systemWsDmo_ConfigActionId);\n"
            "    std::set<std::pair<uint32_t, std::string>> systemWsDmo_ConfigActionEnums({\n"
            "        {0, \"None\"},\n"
            "        {1, \"Load\"},\n"
            "        {2, \"Unload\"},\n"
            "        {3, \"OverWrite\"},\n"
            "    });\n"
            "    valueInit(systemWsDmo_ConfigActionValue, systemWsDmo_ConfigActionEnums);\n"
            "    valueSet<std::string>(systemWsDmo_ConfigActionValue, \"None\");\n"
            "    dmo.insertMetaData(systemWsDmo_ConfigActionId, systemWsDmo_ConfigActionValue);\n"
            "\n"
            "    // system.ws.dmo.+.config.path\n"
            "    CompanEdgeProtocol::Value systemWsDmo_ConfigPathValue;\n"
            "    systemWsDmo_ConfigPathValue.set_id(systemWsDmo_ConfigPathId);\n"
            "    dmo.insertMetaData(systemWsDmo_ConfigPathId, createValueWrapper(systemWsDmo_ConfigPathId, "
            "CompanEdgeProtocol::Text));\n"
            "\n"
            "    // system.ws.dmo.+.persisted\n"
            "    dmo.insertMetaData(systemWsDmo_PersistedId, createValueWrapper(systemWsDmo_PersistedId, "
            "CompanEdgeProtocol::Container));\n"
            "\n"
            "    // system.ws.dmo.+.persisted.+.config.action\n"
            "    CompanEdgeProtocol::Value systemWsDmo_Persisted_ConfigActionValue;\n"
            "    systemWsDmo_Persisted_ConfigActionValue.set_id(systemWsDmo_Persisted_ConfigActionId);\n"
            "    std::set<std::pair<uint32_t, std::string>> systemWsDmo_Persisted_ConfigActionEnums({\n"
            "        {0, \"None\"},\n"
            "        {1, \"Load\"},\n"
            "        {2, \"Unload\"},\n"
            "        {3, \"OverWrite\"},\n"
            "    });\n"
            "    valueInit(systemWsDmo_Persisted_ConfigActionValue, systemWsDmo_Persisted_ConfigActionEnums);\n"
            "    valueSet<std::string>(systemWsDmo_Persisted_ConfigActionValue, \"None\");\n"
            "    dmo.insertMetaData(systemWsDmo_Persisted_ConfigActionId, systemWsDmo_Persisted_ConfigActionValue);\n"
            "\n"
            "    // system.ws.dmo.+.persisted.+.config.path\n"
            "    CompanEdgeProtocol::Value systemWsDmo_Persisted_ConfigPathValue;\n"
            "    systemWsDmo_Persisted_ConfigPathValue.set_id(systemWsDmo_Persisted_ConfigPathId);\n"
            "    dmo.insertMetaData(systemWsDmo_Persisted_ConfigPathId, "
            "createValueWrapper(systemWsDmo_Persisted_ConfigPathId, CompanEdgeProtocol::Text));\n"
            "\n"
            "    // system.ws.dmo.+.persisted.+.status.action\n"
            "    CompanEdgeProtocol::Value systemWsDmo_Persisted_StatusActionValue;\n"
            "    systemWsDmo_Persisted_StatusActionValue.set_id(systemWsDmo_Persisted_StatusActionId);\n"
            "    std::set<std::pair<uint32_t, std::string>> systemWsDmo_Persisted_StatusActionEnums({\n"
            "        {0, \"None\"},\n"
            "        {1, \"Complete\"},\n"
            "        {2, \"Failed\"},\n"
            "        {3, \"DmoExists\"},\n"
            "        {4, \"FileNotFound\"},\n"
            "    });\n"
            "    valueInit(systemWsDmo_Persisted_StatusActionValue, systemWsDmo_Persisted_StatusActionEnums);\n"
            "    valueSet<std::string>(systemWsDmo_Persisted_StatusActionValue, \"None\");\n"
            "    dmo.insertMetaData(systemWsDmo_Persisted_StatusActionId, systemWsDmo_Persisted_StatusActionValue);\n"
            "\n"
            "    // system.ws.dmo.+.status.action\n"
            "    CompanEdgeProtocol::Value systemWsDmo_StatusActionValue;\n"
            "    systemWsDmo_StatusActionValue.set_id(systemWsDmo_StatusActionId);\n"
            "    std::set<std::pair<uint32_t, std::string>> systemWsDmo_StatusActionEnums({\n"
            "        {0, \"None\"},\n"
            "        {1, \"Complete\"},\n"
            "        {2, \"Failed\"},\n"
            "        {3, \"DmoExists\"},\n"
            "        {4, \"FileNotFound\"},\n"
            "    });\n"
            "    valueInit(systemWsDmo_StatusActionValue, systemWsDmo_StatusActionEnums);\n"
            "    valueSet<std::string>(systemWsDmo_StatusActionValue, \"None\");\n"
            "    dmo.insertMetaData(systemWsDmo_StatusActionId, systemWsDmo_StatusActionValue);\n"
            "}\n"
            "\n"
            "} // namespace\n"
            "\n"
            "void DynamicDmoMetaData::create(VariantValueStore& ws, DmoContainer& dmo) {\n"
            "    create(ws);\n"
            "    createMetaData(dmo);\n"
            "}\n"
            "\n"
            "void DynamicDmoMetaData::create(VariantValueStore& ws) {\n"
            "    ws.set(createValueWrapper(systemWsDmoId, CompanEdgeProtocol::Container), VariantValue::Remote);\n"
            "    ws.set(createValueWrapper(systemWsDmoPathId, CompanEdgeProtocol::Text), VariantValue::Remote);\n"
            "    ws.set(createValueWrapper(systemWsPersistPathId, CompanEdgeProtocol::Text), VariantValue::Remote);\n"
            "}\n"
            "\n";

    EXPECT_EQ(oStrm.str(), output);
    //    std::cout << oStrm.str() << std::endl;
}
