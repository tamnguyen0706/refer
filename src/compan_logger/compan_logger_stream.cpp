/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_stream.cpp
 @brief Logger Capture Stream
 */
#include "Compan_logger_stream.h"
#include "Compan_logger.h"
#include "Compan_logger_controller.h"

using namespace Compan::Edge;

CompanLoggerStream ::CompanLoggerStream(CompanLogger const& logger, LogLevel level)
    : logger_(logger)
    , level_(level)
{
    (void)logger_;
}

CompanLoggerStream ::~CompanLoggerStream()
{
    std::string str = stream_.str();
    size_t found = str.find_last_not_of('\n');
    if (found != std::string::npos) str.erase(found + 1);

    CompanLoggerController::get()->log(logger_, level_, str);
}

std::ostream& CompanLoggerStream ::log()
{
    return stream_;
}
