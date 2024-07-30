/**
 Copyright © 2023 COMPAN REF
 @file company_ref_persistor_bl.cpp
 @brief Persistor Business Logic
 */

#include "company_ref_persistor_bl.h"
#include <company_ref_utils/company_ref_ini_config_file.h>

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_dmo/company_ref_dmo_file.h>
#include <Compan_logger/Compan_logger.h>

#include <sstream>

namespace Compan{
namespace Edge {
extern CompanLogger PersistorMainLog;
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

PersistorBl::PersistorBl(boost::asio::io_context& ioContext, std::string const& udsPath, std::string const& cfgPath)
    : ioContext_(ioContext)
    , udsPath_(udsPath)
    , cfgPath_(cfgPath)
    , client_(ioContext_, "Persistor", udsPath)
    , clientConnectedListener(
              client_.clientConnection()->connectOnConnection(std::bind(&PersistorBl::onConnected, this)))
    , clientDisconnectedListener(
              client_.clientConnection()->connectOnDisconnection(std::bind(&PersistorBl::onDisconnected, this)))

{
}

PersistorBl::~PersistorBl()
{
    persistorClient_.reset();
}

void PersistorBl::onConnected()
{
    FunctionLog(PersistorMainLog);

    persistorClient_ = std::make_shared<PersistorClient>(ioContext_, udsPath_);
    if (!persistorClient_) ErrorLog(PersistorMainLog) << "Failed to create persistor client" << std::endl;

    persistorClient_->start(cfgPath_);

    client_.setStartupCompleted();
}

void PersistorBl::onDisconnected()
{
    FunctionLog(PersistorMainLog);
    persistorClient_.reset();
}
