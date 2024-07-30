/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_mock.cpp
 @brief DynamicDmo mock
 */
#include "company_ref_dynamic_dmo_mock.h"

#include <company_ref_dmo/company_ref_dmo_file.h>

using namespace Compan::Edge;

boost::filesystem::path const DynamicDmoTest::DmoDirName("./");
boost::filesystem::path const DynamicDmoTest::DmoFileName("MockFile.dmo");
std::string const DynamicDmoTest::MicroServiceName("MockMicroService");

DynamicDmoTest::DynamicDmoTest()
    : ctx_()
    , ws_(ctx_)
{
    DynamicDmoMetaData::create(ws_, dmo_);
}

DynamicDmoTest::~DynamicDmoTest()
{
    EXPECT_TRUE(coutWrapper_.empty());

    if (!coutWrapper_.empty()) std::cout << coutWrapper_.pop() << std::endl;
}

bool DynamicDmoTest::createMockDmoStream(std::ostream& os)
{
    ValueDefType const dmoFileData(
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

    DmoContainer mockDmoContainer;
    for (auto& child : dmoFileData) {

        CompanEdgeProtocol::Value vsValue;
        vsValue.set_id(child.first);
        vsValue.set_type(child.second);
        mockDmoContainer.insertMetaData(ValueId(child.first), vsValue);
    }

    mockDmoContainer.sort();

    return DmoFile::write(os, mockDmoContainer);
}

bool DynamicDmoTest::createMockDmoFile()
{
    boost::filesystem::ofstream oFile(DmoDirName / DmoFileName, std::ios::binary);
    if (!oFile.is_open()) return false;

    return createMockDmoStream(oFile);
}

void DynamicDmoTest::run()
{
    ctx_.restart();
    ctx_.run();
}
