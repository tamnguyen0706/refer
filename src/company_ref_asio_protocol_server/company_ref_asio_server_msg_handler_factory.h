/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_server_msg_handler_factory.h
 @brief ServerProtocolSerializer MsgHandler factory
 */
#ifndef __company_ref_ASIO_SERVER_MSG_HANDLER_FACTORY_H_
#define __company_ref_ASIO_SERVER_MSG_HANDLER_FACTORY_H_

#include <company_ref_asio/company_ref_asio_msg_handler_factory.h>
#include <memory>
#include <mutex>

namespace boost {
namespace asio {
class io_context;
} // namespace asio
} // namespace boost

namespace Compan{
namespace Edge {

class VariantValueStore;
class DmoContainer;

class AsioMsgHandlerFactory;
class AsioMsgHandler;
using AsioMsgHandlerPtr = std::shared_ptr<AsioMsgHandler>;

/*!
 *
 */
class ServerMsgHandlerFactory : public AsioMsgHandlerFactory {
public:
    ServerMsgHandlerFactory(boost::asio::io_context& ctx, VariantValueStore& ws, DmoContainer& dmo);

    virtual ~ServerMsgHandlerFactory();

    virtual AsioMsgHandlerPtr make(uint32_t const connectionId);

private:
    boost::asio::io_context& ctx_;
    VariantValueStore& ws_;
    DmoContainer& dmo_;

    std::mutex wsContainerMutex_; // used to make sure two connections aren't attempting to
                                  // add/remove elements
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_SERVER_MSG_HANDLER_FACTORY_H_
