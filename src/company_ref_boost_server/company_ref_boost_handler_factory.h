/**
 Copyright © 2023 COMPAN REF
 @file company_ref_boost_handler_factory.h
 @brief CompanEdgeBoostMessageHandler creator
 */
#ifndef __company_ref_BOOST_HANDLER_FACTORY_H__
#define __company_ref_BOOST_HANDLER_FACTORY_H__

#include "company_ref_boost_message_handler.h"

namespace Compan{
namespace Edge {

/*!
 * Factory creator object for CompanEdgeBoostMessageHandler
 *
 * Allows for customized message handlers
 */
class CompanEdgeBoostMessageHandlerFactory {
public:
    CompanEdgeBoostMessageHandlerFactory();
    virtual ~CompanEdgeBoostMessageHandlerFactory() = default;

    virtual CompanEdgeBoostMessageHandler::Ptr makeHandler(uint32_t const connectionId) = 0;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_BOOST_HANDLER_FACTORY_H__
