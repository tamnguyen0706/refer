/**
Copyright © 2024 COMPAN REF
@file company_ref_asio_msg_handler_factory.h
@brief Message Handler factory
*/
#ifndef __company_ref_ASIO_MSG_HANDLER_FACTORY_H__
#define __company_ref_ASIO_MSG_HANDLER_FACTORY_H__

#include <memory>

namespace Compan{
namespace Edge {

class AsioMsgHandler;
using AsioMsgHandlerPtr = std::shared_ptr<AsioMsgHandler>;

/// Creates a new instance of a message handler
class AsioMsgHandlerFactory {
public:
    AsioMsgHandlerFactory();
    virtual ~AsioMsgHandlerFactory() = default;

    virtual AsioMsgHandlerPtr make(uint32_t const connectionId) = 0;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_MSG_HANDLER_FACTORY_H__
