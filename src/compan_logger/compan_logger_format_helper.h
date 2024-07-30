/**
 Copyright © 2024 COMPAN REF
 @file Compan_logger_format_helper.h
 @brief Logger format helper object
 */

#ifndef __Compan_LOGGER_FORMAT_HELPER_H__
#define __Compan_LOGGER_FORMAT_HELPER_H__

#include <memory>

namespace Compan{
namespace Edge {

class CompanLogger;

/*!
 * @brief CompanLogger Format Helper
 *
 * Functionality and settings related to formatting log output
 *
 */
class CompanLoggerFormatHelper {
public:
    using Ptr = std::shared_ptr<CompanLoggerFormatHelper>;

    virtual ~CompanLoggerFormatHelper();

    void updateMaxLoggerNameLength(const CompanLogger& logger);

    unsigned maxLoggerNameLength() const;

    void resetLoggerNameLength();

    void logIncreaseIndent();

    void logDecreaseIndent();

    unsigned getIndentLevel() const;

    /// Single instance access
    static CompanLoggerFormatHelper::Ptr get();

protected:
    CompanLoggerFormatHelper();

private:
    unsigned maxLoggerNameLength_;
    unsigned indentLevel_;
};

} // namespace Edge
} // namespace Compan

#endif //__Compan_LOGGER_FORMAT_HELPER_H__
