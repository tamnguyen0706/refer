/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_protocol_message_handler_remove.cpp
  @brief Testing ValueRemoved
*/

#include "company_ref_asio_server_protocol_handler_mock.h"

TEST_F(ServerProtocolHandlerTest, ValueRemoved)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;
    ValueIdListener listener(textId_);

    populateValueStore();

    // when using trackable - you must use boost::placeholders
    ws_.connectValueRemovedListener(std::bind(&ValueIdListener::doNotification, &listener, std::placeholders::_1));

    // subscribed to the global notifications
    reqMsg.mutable_vssubscribe();
    handler_->doMessage(reqMsg);
    rspMsg = queueGet();

    Validate_VsResultAll(rspMsg);
    Validate_VsResultOffset(rspMsg, "a", 0);
    Validate_VsResultOffset(rspMsg, "a.b", 1);
    Validate_VsResultOffset(rspMsg, "a.b.container1", 2);
    Validate_VsResultOffset(rspMsg, "a.b.container2", 3);
    Validate_VsResultOffset(rspMsg, "test", 4);
    Validate_VsResultOffset(rspMsg, "test.bool", 5);
    Validate_VsResultOffset(rspMsg, "test.enum", 6);
    Validate_VsResultOffset(rspMsg, "test.ranged", 7);
    Validate_VsResultOffset(rspMsg, "test.string", 8);

    reqMsg.Clear();
    reqMsg.Clear();

    // save for validation; need to get it before deleting the value
    uint64_t hashToken = getHashToken(textId_);

    // Request remove value
    *reqMsg.mutable_valueremoved()->add_id() = textId_;
    handler_->doMessage(reqMsg);

    // received the notification
    rspMsg = queueGet();

    Validate_ClientMessageHasNoVsResult(rspMsg);
    Validate_ValueRemoved(rspMsg, textId_, hashToken);
}
