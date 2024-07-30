/**
 Copyright © 2023 COMPAN REF
 @file company_ref_tcp_message_handler.h
 @brief Tcp -> Uds client Message bridge
 */
#ifndef __company_ref_TCP_MESSAGE_HANDLER_H__
#define __company_ref_TCP_MESSAGE_HANDLER_H__

#include "company_ref_uds_message_handler.h"

#include <company_ref_asio/company_ref_asio_msg_handler.h>
#include <company_ref_asio/company_ref_asio_msg_handler_factory.h>
#include <company_ref_asio/company_ref_asio_uds_connection.h>

#include <boost/asio/io_context.hpp>
#include <memory>

namespace Compan{
namespace Edge {

/*!
 * Bridges Tcp server messages to the Uds client library
 */
class CompanEdgeTcpMessageHandler : public AsioMsgHandler,
                                  public std::enable_shared_from_this<CompanEdgeTcpMessageHandler> {
public:
    CompanEdgeTcpMessageHandler(boost::asio::io_context& ctx, std::string const& udsPath, uint32_t const connectonId);
    virtual ~CompanEdgeTcpMessageHandler();

    virtual void start();
    virtual void stop();
    virtual void recvData(BufferType&&);

    virtual void passthruData(BufferType&&);

private:
    CompanEdgeUdsMessageHandler::Ptr udsHandler_;
    AsioUdsConnection::Ptr udsClient_;

    AsioMsgHandler::SendConnection udsSendConnection_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_TCP_MESSAGE_HANDLER_H__
