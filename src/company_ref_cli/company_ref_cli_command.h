/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_command.h
  @brief CompanEdge - CLI command
*/

#ifndef __company_ref_CLI_company_ref_CLI_COMMAND_H__
#define __company_ref_CLI_company_ref_CLI_COMMAND_H__

#include "company_ref_cli_command_status.h"
#include "company_ref_cli_output.h"

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace CompanEdgeProtocol {
class ClientMessage;
class ServerMessage;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

typedef std::map<std::string, CompanEdgeProtocol::Value> ProtocolValueMap;

class CompanEdgeCliCommand {
public:
    CompanEdgeCliCommand(ProtocolValueMap& savedValues);
    virtual ~CompanEdgeCliCommand();
    virtual bool process(CompanEdgeProtocol::ServerMessage&) = 0;
    virtual CommandStatus process(const CompanEdgeProtocol::ClientMessage&) = 0;

    void setPrinter(std::shared_ptr<CompanEdgeCliOutput>);
    void setFormatOption(FormatOption format);
    void setExpectNoResponseMessage(const bool);
    bool expectNoResponseMsg();
    FormatOption getFormatOption();

protected:
    std::string convert2SetValue(
            const ProtocolValueMap& savedValues,
            const std::string& valueId,
            const std::string& currentValue);

protected:
    uint32_t queueDelayMs_;
    template <typename T>
    void setSequenceNo(T&);
    unsigned int sequenceNo() const;
    std::shared_ptr<CompanEdgeCliOutput> cliOutput_;
    ProtocolValueMap& savedValues_;

private:
    bool expectNoResponseMsg_;
    static unsigned int sequenceNo_;
    FormatOption format_;
};

template <typename T>
void CompanEdgeCliCommand::setSequenceNo(T& msg)
{
    msg.set_sequenceno(++sequenceNo_);
}

class Get : public CompanEdgeCliCommand {
public:
    Get(const std::string& id, ProtocolValueMap& savedValues);
    virtual ~Get();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    const std::string id_;
};

//
// Querying variant valuestore values and store them in a map cache
//
class GetValues : public CompanEdgeCliCommand {
public:
    GetValues(const std::vector<std::pair<std::string, std::string>>& ids, ProtocolValueMap& savedValues);
    virtual ~GetValues();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    std::vector<std::pair<std::string, std::string>> ids_;
};

class GetAll : public CompanEdgeCliCommand {
public:
    GetAll(ProtocolValueMap& savedValues);
    virtual ~GetAll();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);
};

class GetObject : public CompanEdgeCliCommand {
public:
    GetObject(const std::string& id, ProtocolValueMap& savedValues);
    virtual ~GetObject();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    const std::string id_;
};

class Set : public CompanEdgeCliCommand {
public:
    Set(const std::string& id, const std::string& value, ProtocolValueMap& savedValues);
    virtual ~Set();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    const std::string id_;
    std::string value_;
};

class EnableMonitor : public CompanEdgeCliCommand {
public:
    EnableMonitor(ProtocolValueMap& savedValues);
    virtual ~EnableMonitor();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);
};

class AddToContainer : public CompanEdgeCliCommand {
public:
    AddToContainer(const std::string& containerId, const std::string& keyId, ProtocolValueMap& savedValues);
    virtual ~AddToContainer();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    const std::string containerId_;
    const std::string keyId_;
};

class RemoveFromContainer : public CompanEdgeCliCommand {
public:
    RemoveFromContainer(const std::string& containerId, const std::string& keyId, ProtocolValueMap& savedValues);
    virtual ~RemoveFromContainer();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    const std::string containerId_;
    const std::string keyId_;
};

class MultiGet : public CompanEdgeCliCommand {
public:
    MultiGet(const std::vector<std::pair<std::string, std::string>>& ids, ProtocolValueMap& savedValues);
    virtual ~MultiGet();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    std::vector<std::pair<std::string, std::string>> ids_;
};

class MultiSet : public CompanEdgeCliCommand {
public:
    MultiSet(const std::vector<std::pair<std::string, std::string>>& ids, ProtocolValueMap& savedValues);
    virtual ~MultiSet();
    bool process(CompanEdgeProtocol::ServerMessage&);
    CommandStatus process(const CompanEdgeProtocol::ClientMessage&);

private:
    std::vector<std::pair<std::string, std::string>> ids_;
};

inline unsigned int CompanEdgeCliCommand::sequenceNo() const
{
    return sequenceNo_;
}

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_CLI_company_ref_CLI_COMMAND_H__*/
