/**
  Copyright © 2023 COMPAN REF
  @file Compan_logger.cpp
  @brief CompanLogger
*/

#include "Compan_logger.h"
#include "Compan_logger_controller.h"
#include "Compan_logger_format_helper.h"

using namespace Compan::Edge;

CompanLogger::CompanLogger(std::string const& name, LogLevel initialLevel)
    : name_(name)
    , currentLevel_(initialLevel)
{
    CompanLoggerController::get()->addLogger(*this);
}

CompanLogger::~CompanLogger()
{
    CompanLoggerController::get()->removeLogger(*this);
}

void CompanLogger::logLevel(LogLevel level)
{
    currentLevel_ = level;
    if (!logLevelSignal_.empty()) logLevelSignal_(level);
}

_FunctionLog_::_FunctionLog_(CompanLogger& logger, char const* func, bool logEnter)
    : logger_(logger)
    , func_(func)
{
    CompanLoggerFormatHelper::get()->logIncreaseIndent();
    if (logEnter && logger_.isEnabled(FunctionLogLevel())) {
        DoLog(logger_, FunctionLogLevel()) << "Enter: " << func_ << std::endl;
    }
}

_FunctionLog_::~_FunctionLog_()
{
    if (logger_.isEnabled(FunctionLogLevel())) {
        DoLog(logger_, FunctionLogLevel()) << "Exit : " << func_ << std::endl;
    }
    CompanLoggerFormatHelper::get()->logDecreaseIndent();
}
