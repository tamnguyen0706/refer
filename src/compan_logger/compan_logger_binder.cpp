/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_binder.cpp
 @brief CompanLogger Bind class
 */

#include "Compan_logger_binder.h"
#include "Compan_logger.h"

using namespace Compan::Edge;

CompanLoggerBinder::CompanLoggerBinder(std::string const& name, CompanLogger& logger)
    : name_(name)
    , logger_(logger)
    , loggerConnection_(
              logger_.connectLogLevelListener(std::bind(&CompanLoggerBinder::onLogLevel, this, std::placeholders::_1)))
{
}

CompanLoggerBinder::~CompanLoggerBinder()
{
    loggerConnection_.disconnect();
}

LogLevel CompanLoggerBinder::logLevel() const
{
    return logger_.logLevel();
}

void CompanLoggerBinder::logLevel(LogLevel const arg)
{
    logger_.logLevel(arg);
}
