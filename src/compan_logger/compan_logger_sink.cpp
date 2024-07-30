/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_sink.cpp
 @brief CompanLogger output sink
 */

#include "Compan_logger_sink.h"
#include "Compan_logger_controller.h"
#include "Compan_logger_format_helper.h"

using namespace Compan::Edge;

const std::string CompanLoggerSink::nonIndentStr_("");

CompanLoggerSink::CompanLoggerSink()
    : CompanLoggerSink(false, 0)
{
}

CompanLoggerSink::CompanLoggerSink(bool indent, int indentSize)
    : indent_(indent)
    , indentSize_(indentSize)
{
    // add this sink to the controller's sink list
    CompanLoggerController::get()->addSink(this);
}

CompanLoggerSink::~CompanLoggerSink()
{
    // remove this sink from the controller's sink list
    CompanLoggerController::get()->removeSink(this);
}

std::string const CompanLoggerSink::indent() const
{
    if (isIndent() && CompanLoggerFormatHelper::get()->getIndentLevel() > 0) {
        return std::string(CompanLoggerFormatHelper::get()->getIndentLevel() * indentSize_, ' ');
    }
    return nonIndentStr_;
}
