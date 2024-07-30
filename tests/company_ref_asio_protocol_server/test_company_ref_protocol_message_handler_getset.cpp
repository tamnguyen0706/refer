/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_protocol_message_handler_getset.cpp
  @brief Testing GetAll, GetValue, SetValue
*/

#include "company_ref_asio_server_protocol_handler_mock.h"

TEST_F(ServerProtocolHandlerTest, VsGetAll)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    reqMsg.mutable_vsgetall()->set_sequenceno(1);
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
}

TEST_F(ServerProtocolHandlerTest, VsGetValue)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    reqMsg.mutable_vsgetvalue()->set_sequenceno(1);
    reqMsg.mutable_vsgetvalue()->set_id(nonExistId_);
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultNotFound(rspMsg);

    reqMsg.mutable_vsgetvalue()->set_sequenceno(1);
    reqMsg.mutable_vsgetvalue()->set_id(enumId_);
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultSingle(rspMsg, enumId_);
}

TEST_F(ServerProtocolHandlerTest, VsSetValue)
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
        handler_->doMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultSingle(rspMsg, textId_);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(nonExistId_);
        reqMsg.mutable_vssetvalue()->set_value("nothing");
        handler_->doMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultNotFound(rspMsg);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(rangedId_);
        reqMsg.mutable_vssetvalue()->set_value("nostrings");
        handler_->doMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultWrongValueType(rspMsg);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(rangedId_);
        reqMsg.mutable_vssetvalue()->set_value("100");
        handler_->doMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultRangeError(rspMsg);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(enumId_);
        reqMsg.mutable_vssetvalue()->set_value("nope");
        handler_->doMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultEnumError(rspMsg);

    {
        CompanEdgeProtocol::ServerMessage reqMsg;
        reqMsg.mutable_vssetvalue()->set_sequenceno(1);
        reqMsg.mutable_vssetvalue()->set_id(boolId_);
        reqMsg.mutable_vssetvalue()->set_value("false");
        handler_->doMessage(reqMsg);
    }

    rspMsg = queueGet();
    Validate_VsResultAccessError(rspMsg);
}

TEST_F(ServerProtocolHandlerTest, VsGetObjectSimple)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    reqMsg.mutable_vsgetobject()->set_sequenceno(1);
    reqMsg.mutable_vsgetobject()->set_id("a.b.*");
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultOffset(rspMsg, "a.b", 0);
    Validate_VsResultOffset(rspMsg, "a.b.container1", 1);
    Validate_VsResultOffset(rspMsg, "a.b.container2", 2);
}

TEST_F(ServerProtocolHandlerTest, VsGetObjectWildCard)
{

    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("0");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->doMessage(reqMsg);
        queueGet();
        reqMsg.Clear();
    }
    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("1");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->doMessage(reqMsg);
        queueGet();
        reqMsg.Clear();
    }
    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("2");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->doMessage(reqMsg);
        queueGet();
        reqMsg.Clear();
    }

    reqMsg.mutable_vsgetobject()->set_sequenceno(1);
    reqMsg.mutable_vsgetobject()->set_id("a.b.container1.*.interval");
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultOffset(rspMsg, "a.b.container1.0.interval", 1);
    Validate_VsResultOffset(rspMsg, "a.b.container1.1.interval", 2);
    Validate_VsResultOffset(rspMsg, "a.b.container1.2.interval", 3);
}

TEST_F(ServerProtocolHandlerTest, VsGetObjectComplexWildCard)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("0");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->doMessage(reqMsg);
        queueGet();
        reqMsg.Clear();

        {
            CompanEdgeProtocol::Value addToValue2;
            addToValue2.set_id("a.b.container1.0.container");
            addToValue2.mutable_addtocontainer()->set_key("a");
            *reqMsg.mutable_valuechanged()->add_value() = addToValue2;
            handler_->doMessage(reqMsg);
            queueGet();
            reqMsg.Clear();
        }

        {
            CompanEdgeProtocol::Value addToValue2;
            addToValue2.set_id("a.b.container1.0.container");
            addToValue2.mutable_addtocontainer()->set_key("b");
            *reqMsg.mutable_valuechanged()->add_value() = addToValue2;
            handler_->doMessage(reqMsg);
            queueGet();
            reqMsg.Clear();
        }

        {
            CompanEdgeProtocol::Value addToValue2;
            addToValue2.set_id("a.b.container1.0.container");
            addToValue2.mutable_addtocontainer()->set_key("c");
            *reqMsg.mutable_valuechanged()->add_value() = addToValue2;
            handler_->doMessage(reqMsg);
            queueGet();
            reqMsg.Clear();
        }
    }
    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("1");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;
        handler_->doMessage(reqMsg);
        queueGet();
        reqMsg.Clear();
    }

    //    ws_.visitValues([](VariantValue::Ptr const& visitPtr) {
    //        std::cout << visitPtr->id() << std::endl;
    //    });

    reqMsg.mutable_vsgetobject()->set_sequenceno(1);
    reqMsg.mutable_vsgetobject()->set_id("a.b.container1.*.container.*.eui48");
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsResultOffset(rspMsg, "a.b.container1.0.container.a.eui48", 0);
    Validate_VsResultOffset(rspMsg, "a.b.container1.0.container.b.eui48", 1);
    Validate_VsResultOffset(rspMsg, "a.b.container1.0.container.c.eui48", 2);
}
