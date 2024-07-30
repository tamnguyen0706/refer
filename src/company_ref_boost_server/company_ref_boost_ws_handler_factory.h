/**
 Copyright © 2023 COMPAN REF
 @file company_ref_boost_ws_handler_factory.h
 @brief VariantValueStore message handler factory
 */
#ifndef __company_ref_BOOST_WS_HANDLER_FACTORY_H__
#define __company_ref_BOOST_WS_HANDLER_FACTORY_H__

#include "company_ref_boost_handler_factory.h"

namespace Compan{
namespace Edge {

/// VariantValueStore message handler factory
class CompanEdgeBoostWsMessageHandlerFactory : public CompanEdgeBoostMessageHandlerFactory {
public:
    CompanEdgeBoostWsMessageHandlerFactory(boost::asio::io_context& ctx, VariantValueStore& ws, DmoContainer& dmo);
    virtual ~CompanEdgeBoostWsMessageHandlerFactory() = default;

    virtual CompanEdgeBoostMessageHandler::Ptr makeHandler(uint32_t const connectionId);

private:
    boost::asio::io_context& ctx_;
    VariantValueStore& ws_;
    DmoContainer& dmo_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_BOOST_WS_HANDLER_FACTORY_H__
