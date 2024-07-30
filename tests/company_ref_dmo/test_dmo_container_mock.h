/**
 Copyright © 2023 COMPAN REF
 @file test_dmo_container_mock.h
 @brief Common definitions for Value/MetaData
 */
#ifndef __TEST_DMO_CONTAINER_MOCK_H__
#define __TEST_DMO_CONTAINER_MOCK_H__

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_variant.h>
#include <Compan_logger/Compan_logger_sink_buffered.h>

using namespace Compan::Edge;

class DmoContainerTest : public testing::Test {
public:
    typedef std::vector<std::pair<std::string, CompanEdgeProtocol::Value_Type>> ValueDefType;

    DmoContainerTest();
    virtual ~DmoContainerTest();

    void addValueDefs();
    void addMetaDefs();
    void addInstanceDefs();

    ValueDefType valueDefs_;
    ValueDefType resultsValuesVisited_;
    ValueDefType metaDefs_;
    ValueDefType resultsMetaVisited_;
    ValueDefType instanceDefs_;
    ValueDefType resultsInstanceVisited_;

    DmoContainer dmo_;

    CompanLoggerSinkBuffered coutWrapper_;

    static std::string const DmoFile;
};

#endif // __TEST_DMO_CONTAINER_MOCK_H__
