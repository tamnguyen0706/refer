/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_output.h
  @brief cli output
*/

#ifndef __company_ref_CLI_OUTPUT__
#define __company_ref_CLI_OUTPUT__

#include "company_ref_cli_command_status.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_stream.h>

namespace Compan{
namespace Edge {

enum PrintAttribute {
    PRINT_VALUES_DELIMITER = 0,
    PRINT_VALUEID_VALUE_SEPARATOR,
    PRINT_VALUE_CATEGORY_SHOWN,
    PRINT_VALUE_ONLY,
    PRINT_TIMEZONE
};

class CompanEdgeCliOutput {
public:
    CompanEdgeCliOutput();
    virtual ~CompanEdgeCliOutput() = default;

    virtual void print(google::protobuf::RepeatedPtrField<::CompanEdgeProtocol::Value> const&);
    virtual void printCsv(google::protobuf::RepeatedPtrField<::CompanEdgeProtocol::Value> const&);
    virtual void print(CompanEdgeProtocol::VsResult const&);
    virtual void printValue(CompanEdgeProtocol::VsResult const&);
    virtual void print(CompanEdgeProtocol::VsMultiGetResult const&);
    virtual void print(CompanEdgeProtocol::VsMultiSetResult const&);
    virtual void print(CompanEdgeProtocol::ValueChanged const&);
    virtual void print(CompanEdgeProtocol::ValueRemoved const&);
    virtual void print(CompanEdgeProtocol::ClientMessage const& msg);
    virtual std::string toString(CompanEdgeProtocol::Value const&);
    virtual void printError(std::string const& description);
    void setAttribute(PrintAttribute attrib, std::string&& valueStr);
    void setFormatOption(FormatOption format);
    void setDefaultAttribute();

protected:
    std::string msgDelimiter_;
    std::string valuesDelimiter_;
    std::string valueIdValueDelimiter_;
    std::string timeZoneRule_;
    bool valueCategoryShown_;
    bool valueOnly_;
    FormatOption format_;

protected:
    static const std::string ValueChangedTypeName;
    static const std::string ValueRemovedTypeName;
    static const std::string VsResultTypeName;
    static const std::string ErrorHeader;
};

class CompanEdgeCliSimpleOutput : public CompanEdgeCliOutput {
public:
    CompanEdgeCliSimpleOutput() = default;
    virtual ~CompanEdgeCliSimpleOutput() = default;

private:
};

class CompanEdgeCliVerboseOutput : public CompanEdgeCliOutput {
public:
    CompanEdgeCliVerboseOutput() = default;
    virtual ~CompanEdgeCliVerboseOutput() = default;

    virtual void print(CompanEdgeProtocol::VsResult const& res) override;
    virtual void printValue(CompanEdgeProtocol::VsResult const& res) override;
    virtual void print(CompanEdgeProtocol::VsMultiGetResult const& res) override;
    virtual void print(CompanEdgeProtocol::VsMultiSetResult const& res) override;
    virtual void print(CompanEdgeProtocol::ValueChanged const& res) override;
    virtual void print(CompanEdgeProtocol::ValueRemoved const& res) override;
    virtual void print(CompanEdgeProtocol::ClientMessage const& msg) override;

private:
};

} // namespace Edge
} // namespace Compan

#endif