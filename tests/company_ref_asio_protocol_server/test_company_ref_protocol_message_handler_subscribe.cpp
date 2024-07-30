/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_protocol_message_handler_subscribe.cpp
  @brief Testing VsSubscribe
*/

#include "company_ref_asio_server_protocol_handler_mock.h"

TEST_F(ServerProtocolHandlerTest, VsSubscribe_Global)
{

    CompanEdgeProtocol::ServerMessage reqMsg;
    CompanEdgeProtocol::ClientMessage rspMsg;
    ValueIdListener listener(textId_, 2, 2);

    populateValueStore();

    ws_.connectValueChangedListener(std::bind(&ValueIdListener::doNotification, &listener, std::placeholders::_1));

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

    EXPECT_TRUE(msgQueue_.empty());

    // change a value
    {
        VariantValue::Ptr valuePtr = ws_.get(textId_);
        valuePtr->set("something");
    }

    // received the notification
    rspMsg = queueGet();
    Validate_ValueChangedSingle(rspMsg, textId_);

    // received the notification for the parent
    rspMsg = queueGet();
    Validate_ValueChangedSingle(rspMsg, "test");

    // save for validation
    uint64_t hashToken = getHashToken(textId_);

    // delete a value
    EXPECT_TRUE(ws_.del(textId_));

    // received the notification
    rspMsg = queueGet();
    Validate_ValueRemoved(rspMsg, textId_, hashToken);
    EXPECT_TRUE(msgQueue_.empty());

    // add a value
    ws_.set(textValue_);

    // received the notification
    rspMsg = queueGet();
    Validate_ValueChangedSingle(rspMsg, textId_);

    ASSERT_TRUE(msgQueue_.empty());

    reqMsg.Clear();

    reqMsg.mutable_vsunsubscribe();

    handler_->doMessage(reqMsg);
    rspMsg = queueGet();

    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_success);

    // change a value
    {
        VariantValue::Ptr valuePtr = ws_.get(textId_);
        valuePtr->set("nothing");
    }

    run();
}

TEST_F(ServerProtocolHandlerTest, VsSubscribe_Discrete)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;
    ValueIdListener listener(textId_, 1, 3);

    populateValueStore();

    ws_.connectValueChangedListener(std::bind(&ValueIdListener::doNotification, &listener, std::placeholders::_1));

    *reqMsg.mutable_vssubscribe()->add_ids() = textId_;
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultSingle(rspMsg, textId_);

    // sets something other than what we've subscribed for
    {
        VariantValue::Ptr valuePtr = ws_.get(enumId_);
        valuePtr->set("Maybe");
    }

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

    // delete a value
    ws_.del(textId_);

    // received the notification
    rspMsg = queueGet();
    Validate_ValueRemoved(rspMsg, textId_, hashToken);
    EXPECT_TRUE(msgQueue_.empty());

    // remove a non-subscribed value
    ws_.del(enumId_);
}

TEST_F(ServerProtocolHandlerTest, VsSubscribe_DiscreteChildren)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    *reqMsg.mutable_vssubscribe()->add_ids() = structId_;
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultOffset(rspMsg, structId_, 0);
    Validate_VsResultOffset(rspMsg, containerId1_, 1);
    Validate_VsResultOffset(rspMsg, containerId2_, 2);
}
