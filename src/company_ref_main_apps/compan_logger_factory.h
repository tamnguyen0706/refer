/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_factory.h
 @brief Creates CompanLogger Binders to the ValueStore
 */
#ifndef __Compan_LOGGER_FACTORY_H__
#define __Compan_LOGGER_FACTORY_H__

#include <string>

namespace CompanValueTypes {
class EnumValue;
} // namespace CompanValueTypes

namespace Compan{
namespace Edge {

class VariantValueStore;
class ValueId;

/*!
 * @brief Factory creator VariantValueLoggerBinder objects
 *
 * Automatically creates elements in the VariantValueStore
 * to handle ValueStore updates for changing log levels.
 *
 * Microservices use this to auto register their local
 * CompanLogger objects
 */
class LoggerFactory {
public:
    /*!
     * Creates the
     * @param appName
     * @param ws
     * @param isMicroService
     */
    static void createBinders(std::string const& appName, VariantValueStore& ws, bool const isMicroService = true);

    // Base logger id
    static ValueId const LoggerId;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_FACTORY_H__
