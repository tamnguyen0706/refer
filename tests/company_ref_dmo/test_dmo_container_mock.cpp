/**
 Copyright © 2023 COMPAN REF
 @file test_dmo_container_mock.cpp
 @brief Common definitions for Value/MetaData
 */
#include "test_dmo_container_mock.h"
#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_enum.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <Compan_logger/Compan_logger_loglevel.h>

std::string const DmoContainerTest::DmoFile("./test.dmo");

DmoContainerTest::DmoContainerTest()
    : valueDefs_({
            //  These are inserted OUT of order - expecting to be sorted correctly by the container
            std::make_pair("a.b", CompanEdgeProtocol::Container),
            std::make_pair("a.bool", CompanEdgeProtocol::Bool),
            std::make_pair("a.enum", CompanEdgeProtocol::Enum),
            std::make_pair("a.struct.enum", CompanEdgeProtocol::Enum),
            std::make_pair("a.struct", CompanEdgeProtocol::Struct),
            std::make_pair("a.struct.container", CompanEdgeProtocol::Container),
            std::make_pair("x.text", CompanEdgeProtocol::Text),
    })
    , resultsValuesVisited_({
              std::make_pair("a", CompanEdgeProtocol::Struct),
              std::make_pair("a.b", CompanEdgeProtocol::Container),
              std::make_pair("a.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.enum", CompanEdgeProtocol::Enum),
              std::make_pair("a.struct", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container", CompanEdgeProtocol::Container),
              std::make_pair("a.struct.enum", CompanEdgeProtocol::Enum),
              std::make_pair("x", CompanEdgeProtocol::Struct),
              std::make_pair("x.text", CompanEdgeProtocol::Text),
      })
    , metaDefs_({
              std::make_pair("a.b", CompanEdgeProtocol::Container),
              std::make_pair("a.b.+.c", CompanEdgeProtocol::Struct),
              std::make_pair("a.b.+.c.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.b.+.c.container1", CompanEdgeProtocol::Container),
              std::make_pair("a.b.+.c.container1.+.eui48", CompanEdgeProtocol::EUI48),
              std::make_pair("a.b.+.c.container2", CompanEdgeProtocol::Container),
              std::make_pair("a.b.+.c.container2.+", CompanEdgeProtocol::Text),
              std::make_pair("a.b.+.c.interval", CompanEdgeProtocol::Interval),
              std::make_pair("a.b.+.c.text", CompanEdgeProtocol::Text),
              std::make_pair("a.b.+.s", CompanEdgeProtocol::Struct),
              std::make_pair("a.b.+.s.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.b.+.s.interval", CompanEdgeProtocol::Interval),
              std::make_pair("a.struct.container", CompanEdgeProtocol::Container),
              std::make_pair("a.struct.container.+.c", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container.+.c.bool", CompanEdgeProtocol::Bool),
      })

    , resultsMetaVisited_({
              std::make_pair("a.b", CompanEdgeProtocol::Container),
              std::make_pair("a.b.+.c", CompanEdgeProtocol::Struct),
              std::make_pair("a.b.+.c.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.b.+.c.container1", CompanEdgeProtocol::Container),
              std::make_pair("a.b.+.c.container1.+.eui48", CompanEdgeProtocol::EUI48),
              std::make_pair("a.b.+.c.container2", CompanEdgeProtocol::Container),
              std::make_pair("a.b.+.c.container2.+", CompanEdgeProtocol::Text),
              std::make_pair("a.b.+.c.enum", CompanEdgeProtocol::Enum),
              std::make_pair("a.b.+.c.interval", CompanEdgeProtocol::Interval),
              std::make_pair("a.b.+.c.text", CompanEdgeProtocol::Text),
              std::make_pair("a.b.+.s", CompanEdgeProtocol::Struct),
              std::make_pair("a.b.+.s.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.b.+.s.interval", CompanEdgeProtocol::Interval),
              std::make_pair("a.struct", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container", CompanEdgeProtocol::Container),
              std::make_pair("a.struct.container.+.c", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container.+.c.bool", CompanEdgeProtocol::Bool),
      })

    , instanceDefs_({
              std::make_pair("a.struct.container.0.c", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container.0.c.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.struct.container.1.c", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container.1.c.bool", CompanEdgeProtocol::Bool),
      })
    , resultsInstanceVisited_({
              std::make_pair("a", CompanEdgeProtocol::Struct),
              std::make_pair("a.b", CompanEdgeProtocol::Container),
              std::make_pair("a.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.enum", CompanEdgeProtocol::Enum),
              std::make_pair("a.struct", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container", CompanEdgeProtocol::Container),
              std::make_pair("a.struct.container.0", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container.0.c", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container.0.c.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.struct.container.1", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container.1.c", CompanEdgeProtocol::Struct),
              std::make_pair("a.struct.container.1.c.bool", CompanEdgeProtocol::Bool),
              std::make_pair("a.struct.enum", CompanEdgeProtocol::Enum),
              std::make_pair("x", CompanEdgeProtocol::Struct),
              std::make_pair("x.text", CompanEdgeProtocol::Text),
      })
{
}

DmoContainerTest::~DmoContainerTest()
{
    EXPECT_TRUE(coutWrapper_.empty());

    if (!coutWrapper_.empty()) std::cout << coutWrapper_.pop() << std::endl;
}

void DmoContainerTest::addValueDefs()
{
    for (auto& child : valueDefs_) {

        CompanEdgeProtocol::Value vsValue;
        vsValue.set_id(child.first);
        vsValue.set_type(child.second);

        EXPECT_TRUE(dmo_.insertValue(vsValue));
    }

    EXPECT_FALSE(dmo_.empty());
    dmo_.sort();
}

void DmoContainerTest::addMetaDefs()
{
    ValueId const mapName("a.b");
    EXPECT_TRUE(dmo_.insertMetaContainer(mapName, CompanEdgeProtocol::Container));

    for (auto& child : metaDefs_) {

        CompanEdgeProtocol::Value vsValue;
        vsValue.set_id(child.first);
        vsValue.set_type(child.second);
        EXPECT_TRUE(dmo_.insertMetaData(ValueId(child.first), vsValue));
    }

    // need to add an EnumValue
    ValueId enumId("a.b.+.c.enum");
    std::set<std::pair<uint32_t, std::string>> enumerators;
    for (auto& iter : COMPAN::REF::getLogLevelBiMap().left)
        enumerators.insert(std::make_pair(static_cast<uint32_t>(iter.first), iter.second));

    CompanEdgeProtocol::Value enumValue;

    valueInit(enumValue, enumerators);
    enumValue.set_id(enumId);
    valueSet<int32_t>(enumValue, static_cast<int32_t>(LogLevel::Information));

    // sets the VariantValue to match the CompanLogger's current log level
    //    std::stringstream logLevelStr;
    //    logLevelStr << LogLevel::Information;
    //    enumPtr->set(logLevelStr.str());

    EXPECT_TRUE(dmo_.insertMetaData(enumId, enumValue));

    EXPECT_FALSE(dmo_.empty());
    dmo_.sort();
}

void DmoContainerTest::addInstanceDefs()
{
    for (auto& child : instanceDefs_) {

        CompanEdgeProtocol::Value vsValue;
        vsValue.set_id(child.first);
        vsValue.set_type(child.second);

        EXPECT_TRUE(dmo_.insertValue(vsValue, false));
    }

    EXPECT_FALSE(dmo_.empty());
    dmo_.sort();
}
