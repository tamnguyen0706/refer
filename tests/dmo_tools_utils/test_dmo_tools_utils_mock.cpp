/**
 Copyright © 2024 COMPAN REF
 @file test_dmo_tools_utils_mock.cpp
 @brief Common ini format definitions for DMO
 */
#include "test_dmo_tools_utils_mock.h"

std::string const DmoToolsUtilsMock::WideDepth("[Values]\n"
                                               "a.b.c.interval [UInterval]\n");

std::string const DmoToolsUtilsMock::MultiWideDepth("[Values]\n"
                                                    "a.b.c.interval [UInterval]\n"
                                                    "a.b.c.interval [UInterval]\n"
                                                    "a.b.c.bool     [Bool]\n"
                                                    "a.b.c.text     [Text]\n");

std::string const DmoToolsUtilsMock::SingleDepth("[Values]\n"
                                                 "a.interval [UInterval]\n"
                                                 "a.bool     [Bool]\n");

std::string const DmoToolsUtilsMock::MultiDepth("[Values]\n"
                                                "a.b1.interval [UInterval]\n"
                                                "a.b1.bool     [Bool]\n"
                                                "a.b2.text     [Text]\n");

std::string const DmoToolsUtilsMock::Enums("[Enums]\n"
                                           "AnswerStatus=(0, No),(1, Yes),(2, Maybe)\n"
                                           "[Values]\n"
                                           "a.enum     [Enum AnswerStatus]\n");

std::string const DmoToolsUtilsMock::DataDefs("[DataModel]\n"
                                              "a.b [Container]\n"
                                              "a.b.+.bool [Bool]\n"
                                              "a.b.+.text [Text]\n");

std::string const DmoToolsUtilsMock::DuplicateDataDefs("[DataModel]\n"
                                                       "a.l1.c.+.bool [Bool]\n"
                                                       "a.l1.c.+.text [Text]\n"
                                                       "a.l2.c.+.bool [Bool]\n"
                                                       "a.l2.c.+.text [Text]\n");

std::string const DmoToolsUtilsMock::ComplexDataDef("[Enums]\n"
                                                    "AnswerStatus=(0, No),(1, Yes),(2, Maybe)\n"
                                                    "[DataModel]\n"
                                                    "abstract.me-you                    [Container]\n"
                                                    "abstract.me-you.+.somthing         [Struct]\n"
                                                    "abstract.me-you.+.somthing.message [Text]\n"
                                                    "abstract.me-you.+.nothing.nothing  [Enum AnswerStatus] = Maybe\n"
                                                    "abstract.Count                     [Interval]\n");

std::string const DmoToolsUtilsMock::DataDefNonStruct("[DataModel]\n"
                                                      "a.params          [Container Text]\n");

std::string const DmoToolsUtilsMock::DoubleContainerDef(
        "[Enums]\n"
        "LogType=(0, DebugLog),(1,Messages),(2,Kernel),(3,Extra)\n"
        "[DataModel]\n"
        "system.log.status                               [Struct]\n"
        "system.log.status.logFiles                      [Container]\n"
        "system.log.status.logFiles.+.files              [Container]\n"
        "system.log.status.logFiles.+.files.+.file       [Text]\n"
        "system.log.status.logFiles.+.files.+.size       [UInterval] = 0\n"
        "system.log.status.logFiles.+.type               [Enum LogType] = DebugLog\n"
        "system.log.status.logFiles.+.folder             [Text]\n"
        "system.log.status.logFiles.+.prefix             [Text]\n");

std::string const DmoToolsUtilsMock::ConfigDmocMicroserviceDef("[ConfigDmoc]\n"
                                                               "MicroServiceName=Something\n");

std::string const DmoToolsUtilsMock::ConfigDmocDef("[ConfigDmoc]\n"
                                                   "BaseId=something.else\n"
                                                   "ClassName=SomethingElseEntirely\n"
                                                   "MicroServiceName=Something\n");

std::string const DmoToolsUtilsMock::MixedConfigDef(
        "[ConfigDmoc]\n"
        "ClassName=DynamicDmo\n"
        "[Enums]\n"
        "LoadAction=(0,None),(1,Load),(2,Unload),(3,OverWrite)\n"
        "LoadStatus=(0,None),(1,Complete),(2,Failed),(3,DmoExists),(4,FileNotFound)\n"
        "[DataModel]\n"
        "system.ws.dmoPath [Text]\n"
        "system.ws.persistPath [Text]\n"
        "system.ws.dmo.+.config.path [Text]\n"
        "system.ws.dmo.+.config.action [Enum LoadAction] = None\n"
        "system.ws.dmo.+.status.action [Enum LoadStatus] = None\n"
        "system.ws.dmo.+.persisted.+.config.path     [Text]\n"
        "system.ws.dmo.+.persisted.+.config.action   [Enum LoadAction] = None\n"
        "system.ws.dmo.+.persisted.+.status.action   [Enum LoadStatus] = None\n");

std::string const DmoToolsUtilsMock::DerivedDataDef("[Values]\n"
                                                    "Compan.vectorValue [Vector] = {1,2,3,2,3,1}\n"
                                                    "Compan.setValue  [Set]    = { a, b, c, c  }\n"
                                                    "Compan.unorderedSetValue [UnorderedSet] = {y,z,x,y}\n");

std::string const DmoToolsUtilsMock::EnumsNonNormal("[Enums]\n"
                                                    "AnswerStatus=(0, 200Mhz),(1, With Space),(2, 1 with space)\n"
                                                    "[Values]\n"
                                                    "a.enum     [Enum AnswerStatus]\n");

std::string const DmoToolsUtilsMock::InstanceDataDef(
        "[DataModel]\n"
        "system.services.tcpServer.instances           [Container]\n"
        "system.services.tcpServer.instances.+.config      [Struct]\n"
        "system.services.tcpServer.instances.+.config.enable       [Bool] = false\n"
        "system.services.tcpServer.instances.+.config.port         [USInterval] = 0\n"
        "system.services.tcpServer.instances.+.status      [Struct]\n"
        "system.services.tcpServer.instances.+.status.hasError     [Bool] = false\n"
        "\n"
        "[Values]\n"
        "system.services.tcpServer.instances.default.config.enable = false\n"
        "system.services.tcpServer.instances.default.config.port = 9998\n");

std::string const DmoToolsUtilsMock::LoggerDataDef(
        "[Enums]\n"
        "logLevels=(0,None),(1,Error),(2,Warning),(3,Information),(4,State),(5,Debug),(6,Trace)\n"
        "[DataModel]\n"
        "logger.+.+ [Container Enum logLevels]=Information\n");

DmoToolsUtilsMock::DmoToolsUtilsMock()
{
}

DmoToolsUtilsMock::~DmoToolsUtilsMock()
{
    EXPECT_TRUE(coutWrapper_.empty());

    if (!coutWrapper_.empty()) std::cout << coutWrapper_.pop() << std::endl;
}
