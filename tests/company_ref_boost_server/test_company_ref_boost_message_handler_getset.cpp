/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_boost_message_handler_getset.cpp
  @brief Testing GetAll, GetValue, SetValue
*/

#include "company_ref_boost_message_handler_mock.h"
#include <gtest/gtest.h>

TEST_F(CompanEdgeBoostMessageHandlerTest, VsGetAll)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    reqMsg.mutable_vsgetall()->set_sequenceno(1);
    handler_->handleMessage(reqMsg);

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
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsGetValue)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    reqMsg.mutable_vsgetvalue()->set_sequenceno(1);
    reqMsg.mutable_vsgetvalue()->set_id(nonExistId_);
    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultNotFound(rspMsg);

    reqMsg.mutable_vsgetvalue()->set_sequenceno(1);
    reqMsg.mutable_vsgetvalue()->set_id(enumId_);
    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultSingle(rspMsg, enumId_);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsSetValue)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    ValueIdListener listener(textId_, 1, 1);

    populateValueStore();

    // when using trackable - you must use boost::placeholders
    ws_.connectValueChangedListener(std::bind(&ValueIdListener::doNotification, &listener, std::placeholders::_1));

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(textId_);
        reqMsg.mutable_vssetvalue()->set_value("something");
        handler_->handleMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultSingle(rspMsg, textId_);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(nonExistId_);
        reqMsg.mutable_vssetvalue()->set_value("nothing");
        handler_->handleMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultNotFound(rspMsg);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(rangedId_);
        reqMsg.mutable_vssetvalue()->set_value("nostrings");
        handler_->handleMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultWrongValueType(rspMsg);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(rangedId_);
        reqMsg.mutable_vssetvalue()->set_value("100");
        handler_->handleMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultRangeError(rspMsg);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(enumId_);
        reqMsg.mutable_vssetvalue()->set_value("nope");
        handler_->handleMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultEnumError(rspMsg);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(boolId_);
        reqMsg.mutable_vssetvalue()->set_value("false");
        handler_->handleMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultAccessError(rspMsg);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsGetObjectSimple)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    reqMsg.mutable_vsgetobject()->set_sequenceno(1);
    reqMsg.mutable_vsgetobject()->set_id("a.b.*");
    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultOffset(rspMsg, "a.b", 0);
    Validate_VsResultOffset(rspMsg, "a.b.container1", 1);
    Validate_VsResultOffset(rspMsg, "a.b.container2", 2);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsGetObjectWildCard)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("0");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->handleMessage(reqMsg);
        queueGet();
        reqMsg.Clear();
    }
    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("1");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->handleMessage(reqMsg);
        queueGet();
        reqMsg.Clear();
    }
    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("2");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->handleMessage(reqMsg);
        queueGet();
        reqMsg.Clear();
    }
    // clear the queue of all the add to container responses
    msgQueue_ = std::queue<CompanEdgeProtocol::ClientMessage>();

    reqMsg.mutable_vsgetobject()->set_sequenceno(1);
    reqMsg.mutable_vsgetobject()->set_id("a.b.container1.*.interval");
    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultOffset(rspMsg, "a.b.container1.0.interval", 1);
    Validate_VsResultOffset(rspMsg, "a.b.container1.1.interval", 2);
    Validate_VsResultOffset(rspMsg, "a.b.container1.2.interval", 3);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsGetObjectComplexWildCard)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("0");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->handleMessage(reqMsg);
        queueGet();
        reqMsg.Clear();

        {
            CompanEdgeProtocol::Value addToValue2;
            addToValue2.set_id("a.b.container1.0.container");
            addToValue2.mutable_addtocontainer()->set_key("a");
            *reqMsg.mutable_valuechanged()->add_value() = addToValue2;
            handler_->handleMessage(reqMsg);
            queueGet();
            reqMsg.Clear();
        }

        {
            CompanEdgeProtocol::Value addToValue2;
            addToValue2.set_id("a.b.container1.0.container");
            addToValue2.mutable_addtocontainer()->set_key("b");
            *reqMsg.mutable_valuechanged()->add_value() = addToValue2;
            handler_->handleMessage(reqMsg);
            queueGet();
            reqMsg.Clear();
        }

        {
            CompanEdgeProtocol::Value addToValue2;
            addToValue2.set_id("a.b.container1.0.container");
            addToValue2.mutable_addtocontainer()->set_key("c");
            *reqMsg.mutable_valuechanged()->add_value() = addToValue2;
            handler_->handleMessage(reqMsg);
            queueGet();
            reqMsg.Clear();
        }
    }
    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("1");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->handleMessage(reqMsg);
        queueGet();
        reqMsg.Clear();
    }

    // clear the queue of all the add to container responses
    msgQueue_ = std::queue<CompanEdgeProtocol::ClientMessage>();

    reqMsg.mutable_vsgetobject()->set_sequenceno(1);
    reqMsg.mutable_vsgetobject()->set_id("a.b.container1.*.container.*.eui48");
    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultOffset(rspMsg, "a.b.container1.0.container.a.eui48", 0);
    Validate_VsResultOffset(rspMsg, "a.b.container1.0.container.b.eui48", 1);
    Validate_VsResultOffset(rspMsg, "a.b.container1.0.container.c.eui48", 2);
}
