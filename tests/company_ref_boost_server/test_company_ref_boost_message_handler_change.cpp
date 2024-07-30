/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_boost_message_handler_change.cpp
  @brief Testing ValueChange
*/

#include "company_ref_boost_message_handler_mock.h"
#include <gtest/gtest.h>

TEST_F(CompanEdgeBoostMessageHandlerTest, ValueChanged_NotFound)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    ValueIdListener listener(textId_, 0, 0);

    ws_.connectValueChangedListener(std::bind(&ValueIdListener::doNotification, &listener, std::placeholders::_1));

    CompanEdgeProtocol::Value changeValue(textValue_);

    changeValue.set_id(nonExistId_);
    handler_->handleMessage(reqMsg);

    queueGet();
}

TEST_F(CompanEdgeBoostMessageHandlerTest, ValueChanged_Success)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    ValueIdListener listener(textId_, 1, 1);

    ws_.connectValueChangedListener(std::bind(&ValueIdListener::doNotification, &listener, std::placeholders::_1));

    CompanEdgeProtocol::Value changeValue(textValue_);
    changeValue.mutable_textvalue()->set_value("some-text");
    *reqMsg.mutable_valuechanged()->add_value() = changeValue;

    handler_->handleMessage(reqMsg);
    queueGet();
}

TEST_F(CompanEdgeBoostMessageHandlerTest, ValueChanged_SameValue)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    *reqMsg.mutable_valuechanged()->add_value() = intervalValue_;

    handler_->handleMessage(reqMsg);
    queueGet();
}

TEST_F(CompanEdgeBoostMessageHandlerTest, ValueChanged_InvalidType)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::Value changeValue(textValue_);
    changeValue.set_type(CompanEdgeProtocol::Interval);
    *reqMsg.mutable_valuechanged()->add_value() = changeValue;

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();
    Validate_ValueChangedSingle(rspMsg, textId_);

    // check the error streams and reset them
    EXPECT_EQ(
            coutWrapper_.pop(),
            std::string("server.message.handler: Error: Variant ValueStore set failed on valueId: test.string - "
                        "InvalidType\n"));
}

TEST_F(CompanEdgeBoostMessageHandlerTest, ValueChanged_RangeError)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::Value changeValue(intervalValue_);
    changeValue.mutable_intervalvalue()->set_value(150);
    *reqMsg.mutable_valuechanged()->add_value() = changeValue;

    handler_->handleMessage(reqMsg);
    rspMsg = queueGet();
    Validate_ValueChangedSingle(rspMsg, rangedId_);

    // check the error streams and reset them
    EXPECT_EQ(
            coutWrapper_.pop(),
            std::string("server.message.handler: Error: Variant ValueStore set failed on valueId: test.ranged - "
                        "RangeError\n"));
}

TEST_F(CompanEdgeBoostMessageHandlerTest, ValueChanged_AccessError)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::Value changeValue(boolValue_);
    changeValue.mutable_boolvalue()->set_value(false);
    *reqMsg.mutable_valuechanged()->add_value() = changeValue;

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();
    Validate_ValueChangedSingle(rspMsg, boolId_);

    // check the error streams and reset them
    EXPECT_EQ(
            coutWrapper_.pop(),
            std::string("server.message.handler: Error: Variant ValueStore set failed on valueId: test.bool - "
                        "AccessError\n"));
}
