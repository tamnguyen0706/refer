/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_protocol_message_handler_sync.cpp
  @brief Testing VsSync
*/

#include "company_ref_asio_server_protocol_handler_mock.h"

TEST_F(ServerProtocolHandlerTest, VsSync_Global)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;
    ValueIdListener listener(textId_, 1, 1);

    populateValueStore();

    ws_.connectValueChangedListener(std::bind(&ValueIdListener::doNotification, &listener, std::placeholders::_1));

    reqMsg.mutable_vssync();
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();
    Validate_ValueChangedAll(rspMsg);

    EXPECT_TRUE(msgQueue_.empty());

    // change a value
    {
        VariantValue::Ptr valuePtr = ws_.get(textId_);
        valuePtr->set("something");
    }
    rspMsg = queueGet();
    // received the notification
    Validate_ValueChangedSingle(rspMsg, textId_);

    rspMsg = queueGet();
    // received the notification for the parent
    Validate_ValueChangedSingle(rspMsg, "test");

    // save for validation
    uint64_t hashToken = getHashToken(textId_);

    // delete a value
    EXPECT_TRUE(ws_.del(textId_));
    EXPECT_FALSE(ws_.has(textId_));
    rspMsg = queueGet();

    // received the notification
    Validate_ValueRemoved(rspMsg, textId_, hashToken);

    // add a value
    ws_.set(textValue_);
    rspMsg = queueGet();

    // received the notification
    Validate_ValueChangedSingle(rspMsg, textId_);
}

TEST_F(ServerProtocolHandlerTest, VsSync_Discrete)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;
    ValueIdListener listener(textId_, 1, 3);

    populateValueStore();

    ws_.connectValueChangedListener(std::bind(&ValueIdListener::doNotification, &listener, std::placeholders::_1));

    *reqMsg.mutable_vssync()->add_ids() = textId_;
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    EXPECT_TRUE(rspMsg.has_vssynccompleted());
    EXPECT_TRUE(rspMsg.has_valuechanged());
    Validate_ValueChangedSingle(rspMsg, textId_);

    // sets something other than what we've subscribed for
    {
        VariantValue::Ptr valuePtr = ws_.get(enumId_);
        valuePtr->set("Maybe");
    }

    // no messages sent
    EXPECT_TRUE(msgQueue_.empty());

    {
        // change subscribed value
        VariantValue::Ptr valuePtr = ws_.get(textId_);
        valuePtr->set("something");
    }

    rspMsg = queueGet();
    Validate_ValueChangedSingle(rspMsg, textId_);

    // save for validation
    uint64_t hashToken = getHashToken(textId_);

    // delete subscribed value
    EXPECT_TRUE(ws_.del(textId_));
    rspMsg = queueGet();

    // received the notification
    Validate_ValueRemoved(rspMsg, textId_, hashToken);
    EXPECT_TRUE(msgQueue_.empty());

    // remove a non-subscribed value
    ws_.del(enumId_);
}

TEST_F(ServerProtocolHandlerTest, VsSync_DiscreteChildren)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    *reqMsg.mutable_vssync()->add_ids() = structId_;
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    EXPECT_TRUE(rspMsg.has_vssynccompleted());
    EXPECT_TRUE(rspMsg.has_valuechanged());
    Validate_ValueChangedOffset(rspMsg, structId_, 0);
    Validate_ValueChangedOffset(rspMsg, containerId1_, 1);
    Validate_ValueChangedOffset(rspMsg, containerId2_, 2);
}
