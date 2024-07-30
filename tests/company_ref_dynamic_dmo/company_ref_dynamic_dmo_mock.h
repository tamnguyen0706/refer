/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_mock.h
 @brief DynamicDmo mock
 */
#ifndef TESTS_company_ref_DYNAMIC_DMO_company_ref_DYNAMIC_DMO_MOCK_H_
#define TESTS_company_ref_DYNAMIC_DMO_company_ref_DYNAMIC_DMO_MOCK_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <Compan_logger/Compan_logger_sink_buffered.h>

#include <company_ref_sdk_value_ids/dynamic_dmo_meta_data.h>

#include <ostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace Compan::Edge;

class DynamicDmoTest : public testing::Test {
public:
    typedef std::vector<std::pair<std::string, CompanEdgeProtocol::Value_Type>> ValueDefType;

    DynamicDmoTest();
    virtual ~DynamicDmoTest();

    bool createMockDmoStream(std::ostream&);
    bool createMockDmoFile();

    void run();

    boost::asio::io_context ctx_;
    VariantValueStore ws_;
    DmoContainer dmo_;

    CompanLoggerSinkBuffered coutWrapper_;

    static boost::filesystem::path const DmoDirName;
    static boost::filesystem::path const DmoFileName;
    static std::string const MicroServiceName;
};

#endif /* TESTS_company_ref_DYNAMIC_DMO_company_ref_DYNAMIC_DMO_MOCK_H_ */
