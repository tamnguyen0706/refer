/**
 Copyright © 2023 COMPAN REF
 @file company_ref_persistor_client.cpp
 @brief Uds Client for pipelineing the values to persistance
 */
#include "company_ref_persistor_client.h"

#include <company_ref_utils/company_ref_ini_config_file.h>
#include <Compan_logger/Compan_logger.h>

#include <company_ref_protocol_utils/company_ref_stream.h>

#include <fstream>

namespace Compan{
namespace Edge {
CompanLogger PersistorClientLog("persistor.client", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

std::string const PersistorClient::PersistSectionName("persist");
std::string const PersistorClient::PersistFileName("file");
std::string const PersistorClient::PersistFlushTimeoutName("flush_timeout");
std::string const PersistorClient::ValueSectionName("values");

PersistorClient::PersistorClient(boost::asio::io_context& ioContext, std::string const& udsPath)
    : udsClient_(std::make_shared<CompanEdgeBoostUdsClient>(ioContext, udsPath))
    , flushTimer_(ioContext)
    , persistPath_("")
    , flushTimeoutSeconds_(5)
    , flushTimerActive_(false)
    , clientMessageConnection_(udsClient_->connectClientMessageListener(
              std::bind(&PersistorClient::onMessageReceived, this, std::placeholders::_1)))

{
    FunctionLog(PersistorClientLog);

    udsClient_->setWantRead([]() { return true; });
}

PersistorClient::~PersistorClient()
{
    FunctionLog(PersistorClientLog);

    udsClient_->close();

    // cancel the timer
    flushTimer_.cancel();
    flushTimerActive_ = false;
}

void PersistorClient::start(std::string const& cfgPath)
{
    FunctionLog(PersistorClientLog);

    IniConfigFile cfgFile;

    if (!cfgFile.read(cfgPath)) {
        ErrorLog(PersistorClientLog) << "Failed to parse " << cfgPath << std::endl;
        return;
    }

    persistPath_ = cfgFile.getValue(PersistSectionName, PersistFileName);

    {
        uint32_t argValue(0);
        std::istringstream(cfgFile.getValue(PersistSectionName, PersistFlushTimeoutName)) >> argValue;

        if (argValue) flushTimeoutSeconds_ = argValue;
    }

    CompanEdgeProtocol::ServerMessage requestMessage;
    CompanEdgeProtocol::VsSubscribe* vsSubscribe = requestMessage.mutable_vssubscribe();

    IniConfigFile::ValuePairMap values = cfgFile.get(ValueSectionName);
    for (auto& iter : values) { vsSubscribe->add_ids(iter.first); }

    udsClient_->write(requestMessage);
}

void PersistorClient::onMessageReceived(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    if (rspMsg.has_valuechanged()) onMessage(rspMsg.valuechanged());
    if (rspMsg.has_valueremoved()) onMessage(rspMsg.valueremoved());
    if (rspMsg.has_vsresult()) onMessage(rspMsg.vsresult());

    setFlushTimer();
}

void PersistorClient::onMessage(CompanEdgeProtocol::ValueChanged const& valueChanged)
{
    FunctionLog(PersistorClientLog);
    for (auto& value : valueChanged.value()) insertValueChange(value);
}

void PersistorClient::onMessage(CompanEdgeProtocol::ValueRemoved const& valueRemoved)
{
    FunctionLog(PersistorClientLog);
    for (auto& valueId : valueRemoved.id()) removeIds_.insert(valueId);
}

void PersistorClient::onMessage(CompanEdgeProtocol::VsResult const& vsResult)
{
    FunctionLog(PersistorClientLog);

    if (vsResult.status() != CompanEdgeProtocol::VsResult::success) return;

    for (auto& value : vsResult.values()) insertValueChange(value);
}

void PersistorClient::insertValueChange(CompanEdgeProtocol::Value const& value)
{
    // don't pollute the restore file with non-data information
    if (value.type() == CompanEdgeProtocol::Container || value.type() == CompanEdgeProtocol::Struct
        || value.has_addtocontainer() || value.has_removefromcontainer()) {
        return;
    }

    changeValues_.emplace(value);
}

void PersistorClient::setFlushTimer()
{
    FunctionLog(PersistorClientLog);

    if (flushTimerActive_) return;

    flushTimer_.expires_from_now(boost::posix_time::seconds(flushTimeoutSeconds_));
    flushTimer_.async_wait(std::bind(&PersistorClient::flushTimerHandler, this, std::placeholders::_1));
    flushTimerActive_ = true;
}

void PersistorClient::flushTimerHandler(boost::system::error_code const& error)
{
    FunctionLog(PersistorClientLog);

    if (error && error != boost::asio::error::operation_aborted) {
        ErrorLog(PersistorClientLog) << __FUNCTION__ << ": " << error.message() << std::endl;
        return;
    }

    saveToFile();
    flushTimerActive_ = false;
}

void PersistorClient::saveToFile()
{
    FunctionLog(PersistorClientLog);

    CompanEdgeDataModel::DataModelMessage dmo;

    std::ifstream iFile(persistPath_.c_str(), std::ios::binary);
    if (iFile.is_open()) { dmo.ParseFromIstream(&iFile); }

    CompanEdgeProtocol::ValueChanged* entities = dmo.mutable_dataentities();

    // we do an inplace remove or replace to the existing entities
    for (auto iter = entities->mutable_value()->begin(); iter != entities->mutable_value()->end(); ++iter) {
        CompanEdgeProtocol::Value value = *iter;

        // remove takes priortity over any change
        if (removeIds_.count(value.id())) {
            auto nextIter = entities->mutable_value()->erase(iter);
            removeIds_.erase(value.id());

            if (nextIter == entities->mutable_value()->end()) break;

            // step back one because of for() loop's incrementor
            iter = nextIter - 1;
            continue;
        }

        auto valueSetIter = changeValues_.find(value);
        if (valueSetIter != changeValues_.end()) {
            *iter = *valueSetIter;
            changeValues_.erase(valueSetIter);
        }
    }

    // anything left in the incoming value changes gets added
    while (changeValues_.begin() != changeValues_.end()) {
        *entities->add_value() = *changeValues_.begin();
        changeValues_.erase(changeValues_.begin());
    }

    removeIds_.clear();
    changeValues_.clear();

    std::ofstream oFile(persistPath_.c_str(), std::ios::binary);
    if (!oFile.is_open()) {
        ErrorLog(PersistorClientLog) << "Failed to open file for writing: " << persistPath_ << std::endl;
        return;
    }

    dmo.SerializeToOstream(&oFile);
}
