/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_dmo_file.cpp
 @brief Test datamodel Read/Write functions
*/

#include "test_dmo_container_mock.h"

#include <company_ref_protocol_utils/company_ref_stream.h>

#include "company_ref_dmo/company_ref_dmo_file.h"
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_visitor.h>

using namespace Compan::Edge;
using namespace CompanEdgeProtocol;

TEST_F(DmoContainerTest, AddValuesToFile)
{
    DmoContainer readContainer;

    addValueDefs();

    EXPECT_TRUE(DmoFile::write(DmoFile, dmo_));

    EXPECT_TRUE(DmoFile::read(DmoFile, readContainer));

    ValueDefType valueVisited;
    readContainer.visitValues([&valueVisited](CompanEdgeProtocol::Value const& value) {
        valueVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(valueVisited, resultsValuesVisited_);
}

TEST_F(DmoContainerTest, AddMetaDataToFile)
{
    DmoContainer readContainer;

    addMetaDefs();

    EXPECT_TRUE(DmoFile::write(DmoFile, dmo_));

    EXPECT_TRUE(DmoFile::read(DmoFile, readContainer));

    ValueDefType metaVisited;
    readContainer.visitMetaData([&metaVisited](CompanEdgeProtocol::Value const& value) {
        metaVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(metaVisited, resultsMetaVisited_);
}

TEST_F(DmoContainerTest, AddMixedTofile)
{
    DmoContainer readContainer;

    addValueDefs();
    addMetaDefs();

    EXPECT_TRUE(DmoFile::write(DmoFile, dmo_));

    EXPECT_TRUE(DmoFile::read(DmoFile, readContainer));

    ValueDefType valueVisited;
    readContainer.visitValues([&valueVisited](CompanEdgeProtocol::Value const& value) {
        valueVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(valueVisited, resultsValuesVisited_);

    ValueDefType metaVisited;
    readContainer.visitMetaData([&metaVisited](CompanEdgeProtocol::Value const& value) {
        metaVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(metaVisited, resultsMetaVisited_);
}

TEST_F(DmoContainerTest, DmoContainerValueStoreFromFile)
{
    DmoContainer readContainer;
    boost::asio::io_context ctx;
    VariantValueStore ws(ctx);

    addValueDefs();
    addMetaDefs();
    addInstanceDefs();

    EXPECT_TRUE(DmoFile::write(DmoFile, dmo_));

    DmoFile::read(DmoFile, readContainer, ws);

    ValueDefType metaVisited;
    readContainer.visitMetaData([&metaVisited](CompanEdgeProtocol::Value const& value) {
        metaVisited.push_back(std::make_pair(value.id(), value.type()));
    });
    EXPECT_EQ(metaVisited, resultsMetaVisited_);

    ValueDefType valueVisited;
    ws.visitValues([&valueVisited](VariantValue::Ptr const valuePtr) {
        valueVisited.push_back(std::make_pair(valuePtr->id(), valuePtr->type()));
    });
    EXPECT_EQ(valueVisited, resultsInstanceVisited_);
}

TEST_F(DmoContainerTest, ValueStoreFromFile)
{
    DmoContainer readContainer;
    boost::asio::io_context ctx;
    VariantValueStore ws(ctx);

    addValueDefs();
    addMetaDefs();
    addInstanceDefs();

    EXPECT_TRUE(DmoFile::write(DmoFile, dmo_));

    DmoFile::read(DmoFile, ws);

    ValueDefType valueVisited;
    ws.visitValues([&valueVisited](VariantValue::Ptr const valuePtr) {
        valueVisited.push_back(std::make_pair(valuePtr->id(), valuePtr->type()));
    });
    EXPECT_EQ(valueVisited, resultsInstanceVisited_);
}

TEST_F(DmoContainerTest, SystemProcessSample)
{
    ValueDefType sampleMetaData(
            {std::make_pair("system", CompanEdgeProtocol::Struct),
             std::make_pair("system.process", CompanEdgeProtocol::Struct),
             std::make_pair("system.process.monitor.config", CompanEdgeProtocol::Struct),
             std::make_pair("system.process.monitor.config.pollingInterval", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.monitor.config.pollingRequest", CompanEdgeProtocol::Bool),
             std::make_pair("system.process.monitor.status", CompanEdgeProtocol::Struct),
             std::make_pair("system.process.monitor.status.processes", CompanEdgeProtocol::Container),
             std::make_pair("system.process.monitor.status.processes.+.pid", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.monitor.status.processes.+.command", CompanEdgeProtocol::Text),
             std::make_pair("system.process.monitor.status.processes.+.cpuTime", CompanEdgeProtocol::ULLInterval),
             std::make_pair("system.process.monitor.status.processes.+.priority", CompanEdgeProtocol::Interval),
             std::make_pair("system.process.monitor.status.processes.+.memSize", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.monitor.status.processes.+.rssSize", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.monitor.status.processes.+.state", CompanEdgeProtocol::Enum),
             std::make_pair("system.process.monitor.status.processes.+.ppid", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.running", CompanEdgeProtocol::Struct),
             std::make_pair("system.process.running.config", CompanEdgeProtocol::Struct),
             std::make_pair("system.process.running.config.pollingInterval", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.running.config.pollingRequest", CompanEdgeProtocol::Bool),
             std::make_pair("system.process.running.config.displayKernelProcesses", CompanEdgeProtocol::Bool),
             std::make_pair("system.process.running.status", CompanEdgeProtocol::Struct),
             std::make_pair("system.process.running.status.processes", CompanEdgeProtocol::Container),
             std::make_pair("system.process.running.status.processes.+.pid", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.running.status.processes.+.command", CompanEdgeProtocol::Text),
             std::make_pair("system.process.running.status.processes.+.cpuTime", CompanEdgeProtocol::ULLInterval),
             std::make_pair("system.process.running.status.processes.+.priority", CompanEdgeProtocol::Interval),
             std::make_pair("system.process.running.status.processes.+.memSize", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.running.status.processes.+.rssSize", CompanEdgeProtocol::UInterval),
             std::make_pair("system.process.running.status.processes.+.state", CompanEdgeProtocol::Enum),
             std::make_pair("system.process.running.status.processes.+.ppid", CompanEdgeProtocol::UInterval)});

    for (auto& child : sampleMetaData) {

        CompanEdgeProtocol::Value vsValue;
        vsValue.set_id(child.first);
        vsValue.set_type(child.second);
        EXPECT_TRUE(dmo_.insertMetaData(ValueId(child.first), vsValue));
    }

    dmo_.sort();

    ValueDefType writeDmo;
    dmo_.visitMetaData([&writeDmo](CompanEdgeProtocol::Value const& value) {
        writeDmo.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_TRUE(DmoFile::write(DmoFile, dmo_));

    DmoContainer readContainer;
    EXPECT_TRUE(DmoFile::read(DmoFile, readContainer));

    ValueDefType readDmo;
    readContainer.visitMetaData([&readDmo](CompanEdgeProtocol::Value const& value) {
        readDmo.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(writeDmo, readDmo);
}

TEST_F(DmoContainerTest, RestoreAgainstMetaData)
{
    addMetaDefs();

    boost::asio::io_context ctx;
    VariantValueStore ws(ctx);

    // these don't exist ... yet
    EXPECT_FALSE(ws.has("a.b.0.s.bool"));
    EXPECT_FALSE(ws.has("a.b.1.s.bool"));
    EXPECT_FALSE(ws.has("a.b.2.s.nope"));
    EXPECT_FALSE(ws.has("a.q"));

    // create a DMO that has ONLY values store in it (persistence mechanism)
    DmoContainer restoreDmo;
    ValueDefType insertedValues({
            std::make_pair("a.b.0.c.container1.Z.eui48", CompanEdgeProtocol::EUI48),
            std::make_pair("a.b.1.s.interval", CompanEdgeProtocol::Interval),
            std::make_pair("a.b.2.s.nope", CompanEdgeProtocol::Interval),
            std::make_pair("a.q", CompanEdgeProtocol::Interval),
    });
    for (auto& child : insertedValues) {

        CompanEdgeProtocol::Value vsValue;
        vsValue.set_id(child.first);
        vsValue.set_type(child.second);

        EXPECT_TRUE(restoreDmo.insertValue(vsValue));
    }
    std::stringstream strm;
    EXPECT_TRUE(DmoFile::write(strm, restoreDmo));

    // Read from the persisted DMO and restore the elements into the DMO
    EXPECT_TRUE(DmoFile::read(strm, dmo_, ws));

    EXPECT_TRUE(ws.has("a.b.0.s.bool"));
    EXPECT_TRUE(ws.has("a.b.1.s.bool"));
    EXPECT_FALSE(ws.has("a.b.2.s.nope"));
    EXPECT_TRUE(ws.has("a.q"));

    EXPECT_EQ(coutWrapper_.pop(), "dmo.file: Warning: Value is not present in metaData: a.b.2.s.nope\n");
}
