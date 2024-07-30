/**
 Copyright © 2024 COMPAN REF
 @file test_dmo_tools_utils_mock.h
 @brief Common ini format definitions for DMO
 */
#ifndef __TEST_DMO_TOOLS_UTILS_MOCK_H__
#define __TEST_DMO_TOOLS_UTILS_MOCK_H__

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_utils/company_ref_ini_config_file.h>
#include <Compan_logger/Compan_logger_sink_buffered.h>

using namespace Compan::Edge;

class DmoToolsUtilsMock : public testing::Test {
public:
    DmoToolsUtilsMock();
    virtual ~DmoToolsUtilsMock();

    IniConfigFile cfg_;
    DmoContainer dmo_;

    CompanLoggerSinkBuffered coutWrapper_;

    static std::string const WideDepth;
    static std::string const MultiWideDepth;
    static std::string const SingleDepth;
    static std::string const MultiDepth;
    static std::string const Enums;
    static std::string const DataDefs;

    static std::string const DuplicateDataDefs;
    static std::string const DuplicateDataDefsMismatched1;
    static std::string const DuplicateDataDefsMismatched2;
    static std::string const DuplicateDataDefsMismatched3;

    static std::string const ComplexDataDef;
    static std::string const DataDefNonStruct;

    static std::string const DoubleContainerDef;

    static std::string const ConfigDmocMicroserviceDef;
    static std::string const ConfigDmocDef;

    static std::string const MixedConfigDef;

    static std::string const DerivedDataDef;
    static std::string const EnumsNonNormal;

    static std::string const InstanceDataDef;

    static std::string const LoggerDataDef;
};

#endif // __TEST_DMO_TOOLS_UTILS_MOCK_H__
