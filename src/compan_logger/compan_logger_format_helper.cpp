/**
 Copyright © 2024 COMPAN REF
 @file Compan_logger_format_helper.h
 @brief Logger format helper object
 */

#include "Compan_logger_format_helper.h"
#include "Compan_logger.h"

#include <algorithm>

using namespace Compan::Edge;

CompanLoggerFormatHelper::CompanLoggerFormatHelper()
    : maxLoggerNameLength_(0)
    , indentLevel_(0)
{
}

CompanLoggerFormatHelper::~CompanLoggerFormatHelper() = default;

unsigned CompanLoggerFormatHelper::maxLoggerNameLength() const
{
    return maxLoggerNameLength_;
}

void CompanLoggerFormatHelper::resetLoggerNameLength()
{
    maxLoggerNameLength_ = 0;
}

void CompanLoggerFormatHelper::logIncreaseIndent()
{
    ++indentLevel_;
}

void CompanLoggerFormatHelper::logDecreaseIndent()
{
    // catch wrap-around on unsigned 0 decrement
    if (indentLevel_ == 0) return;
    --indentLevel_;
}

unsigned CompanLoggerFormatHelper::getIndentLevel() const
{
    return indentLevel_;
}

CompanLoggerFormatHelper::Ptr CompanLoggerFormatHelper::get()
{
    static CompanLoggerFormatHelper::Ptr formatHelper = CompanLoggerFormatHelper::Ptr(new CompanLoggerFormatHelper);
    return formatHelper;
}

void CompanLoggerFormatHelper::updateMaxLoggerNameLength(const CompanLogger& logger)
{
    maxLoggerNameLength_ = std::max<unsigned>(maxLoggerNameLength_, logger.name().size());
}
