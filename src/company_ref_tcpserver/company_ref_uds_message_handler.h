/**
 Copyright © 2024 COMPAN REF
 @file company_ref_uds_message_handler.h
 @brief -*-*-*-*-*-
 */
#ifndef __company_ref_UDS_MESSAGE_HANDLER_H__
#define __company_ref_UDS_MESSAGE_HANDLER_H__

#include <company_ref_asio/company_ref_asio_msg_handler.h>
#include <memory>

namespace Compan{
namespace Edge {

class CompanEdgeUdsMessageHandler : public AsioMsgHandler,
                                  public std::enable_shared_from_this<CompanEdgeUdsMessageHandler> {
public:
    using Ptr = std::shared_ptr<CompanEdgeUdsMessageHandler>;

    CompanEdgeUdsMessageHandler();

    virtual void start();
    virtual void stop();
    virtual void recvData(BufferType&&);

    /// "slot" for sending data to the Connection object
    SendConnection connectSendPassThruData(SendFunction const& f);

private:
    std::weak_ptr<SendFunction> sendPassThruFunction_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_UDS_MESSAGE_HANDLER_H__
