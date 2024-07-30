/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_protocol_message_handler_container.cpp
  @brief Testing AddToContainer, RemoveFromContainer
*/

#include "company_ref_asio_server_protocol_handler_mock.h"

TEST_F(ServerProtocolHandlerTest, AddToContainer)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    reqMsg.mutable_vssubscribe();
    handler_->doMessage(reqMsg);
    queueGet();
    reqMsg.Clear();

    ASSERT_TRUE(msgQueue_.empty());
    ASSERT_TRUE(coutWrapper_.empty());

    IdValueTypeVector addedValues;
    ws_.connectValueAddedListener([&addedValues](VariantValue::Ptr const value) {
        addedValues.push_back(std::make_pair(value->id(), value->type()));
    });

    IdValueTypeVector container1_0(
            {std::make_pair("a.b.container1.0", CompanEdgeProtocol::Struct),
             std::make_pair("a.b.container1.0.interval", CompanEdgeProtocol::Interval),
             std::make_pair("a.b.container1.0.text", CompanEdgeProtocol::Text),
             std::make_pair("a.b.container1.0.dinterval", CompanEdgeProtocol::DInterval),
             std::make_pair("a.b.container1.0.struct", CompanEdgeProtocol::Struct),
             std::make_pair("a.b.container1.0.struct.bool", CompanEdgeProtocol::Bool),
             std::make_pair("a.b.container1.0.container", CompanEdgeProtocol::Container)});

    CompanEdgeProtocol::Value addToValue(containerValue1_);
    addToValue.mutable_addtocontainer()->set_key("0");
    *reqMsg.mutable_valuechanged()->add_value() = addToValue;

    // capture the change notification
    int changeCount(0);
    SignalScopedConnection connection =
            ws_.connectValueAddToContainerListener([this, &changeCount](VariantValue::Ptr const v) {
                if (v->id() == containerValue1_.id()) {
                    EXPECT_TRUE(v->get().has_addtocontainer());
                    EXPECT_EQ(v->get().addtocontainer().key(), "0");
                    ++changeCount;
                    return;
                }
            });

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0", 0));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.container", 1));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.dinterval", 2));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.interval", 3));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.struct", 4));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.struct.bool", 5));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.text", 6));
    EXPECT_TRUE(Validate_ValueAddToContainer(rspMsg, "a.b.container1", 7));

    ASSERT_TRUE(msgQueue_.empty());
    ASSERT_TRUE(coutWrapper_.empty());
    EXPECT_EQ(changeCount, 1);

    connection.disconnect();

    EXPECT_EQ(addedValues, container1_0);

    // start next insert
    addedValues.clear();
    reqMsg.Clear();

    IdValueTypeVector container1_0_1(
            {std::make_pair("a.b.container1.0.container.1", CompanEdgeProtocol::Struct),
             std::make_pair("a.b.container1.0.container.1.eui48", CompanEdgeProtocol::EUI48)});

    addToValue.set_id("a.b.container1.0.container");
    addToValue.mutable_addtocontainer()->set_key("1");
    *reqMsg.mutable_valuechanged()->add_value() = addToValue;

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.container.1", 0));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.container.1.eui48", 1));
    EXPECT_TRUE(Validate_ValueAddToContainer(rspMsg, "a.b.container1.0.container", 2));

    ASSERT_TRUE(msgQueue_.empty());
    ASSERT_TRUE(coutWrapper_.empty());

    EXPECT_EQ(addedValues, container1_0_1);

    // start next insert
    addedValues.clear();
    reqMsg.Clear();

    IdValueTypeVector containerFail;

    addToValue.set_id("a.b.container1.2.container");
    addToValue.mutable_addtocontainer()->set_key("1");
    *reqMsg.mutable_valuechanged()->add_value() = addToValue;

    handler_->doMessage(reqMsg);
    ASSERT_TRUE(msgQueue_.empty());

    EXPECT_EQ(
            coutWrapper_.pop(),
            "server.protocol.handler: Error: doAddToContainer - Not a container: a.b.container1.2.container\n");

    EXPECT_EQ(addedValues, containerFail);
}

TEST_F(ServerProtocolHandlerTest, AddToSubscribedContainer)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    EXPECT_TRUE(ws_.has(containerId1_));

    *reqMsg.mutable_vssubscribe()->add_ids() = containerId1_;
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();
    Validate_VsResultSingle(rspMsg, containerId1_);

    ASSERT_TRUE(msgQueue_.empty());

    IdValueTypeVector addedValues;
    ws_.connectValueAddedListener([&addedValues](VariantValue::Ptr const value) {
        addedValues.push_back(std::make_pair(value->id(), value->type()));
    });

    ws_.get(textId_)->set("something_else");

    IdValueTypeVector container1_0(
            {std::make_pair("a.b.container1.0", CompanEdgeProtocol::Struct),
             std::make_pair("a.b.container1.0.interval", CompanEdgeProtocol::Interval),
             std::make_pair("a.b.container1.0.text", CompanEdgeProtocol::Text),
             std::make_pair("a.b.container1.0.dinterval", CompanEdgeProtocol::DInterval),
             std::make_pair("a.b.container1.0.struct", CompanEdgeProtocol::Struct),
             std::make_pair("a.b.container1.0.struct.bool", CompanEdgeProtocol::Bool),
             std::make_pair("a.b.container1.0.container", CompanEdgeProtocol::Container)});

    reqMsg.Clear();

    CompanEdgeProtocol::Value addToValue1(containerValue1_);
    addToValue1.mutable_addtocontainer()->set_key("0");
    *reqMsg.mutable_valuechanged()->add_value() = addToValue1;

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0", 0));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.container", 1));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.dinterval", 2));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.interval", 3));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.struct", 4));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.struct.bool", 5));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.text", 6));
    EXPECT_TRUE(Validate_ValueAddToContainer(rspMsg, "a.b.container1", 7));

    ASSERT_TRUE(msgQueue_.empty());
    ASSERT_TRUE(coutWrapper_.empty());
    EXPECT_EQ(addedValues, container1_0);

    // start next insert
    addedValues.clear();
    reqMsg.Clear();

    IdValueTypeVector container1_0_1(
            {std::make_pair("a.b.container1.0.container.1", CompanEdgeProtocol::Struct),
             std::make_pair("a.b.container1.0.container.1.eui48", CompanEdgeProtocol::EUI48)});

    CompanEdgeProtocol::Value addToValue2;

    addToValue2.set_id("a.b.container1.0.container");
    addToValue2.mutable_addtocontainer()->set_key("1");
    *reqMsg.mutable_valuechanged()->add_value() = addToValue2;
    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.container.1", 0));
    EXPECT_TRUE(Validate_ValueChangedOffset(rspMsg, "a.b.container1.0.container.1.eui48", 1));
    EXPECT_TRUE(Validate_ValueAddToContainer(rspMsg, "a.b.container1.0.container", 2));

    ASSERT_TRUE(msgQueue_.empty());
    ASSERT_TRUE(coutWrapper_.empty());
    EXPECT_EQ(addedValues, container1_0_1);

    // start next insert
    addedValues.clear();
    reqMsg.Clear();

    IdValueTypeVector containerFail;

    CompanEdgeProtocol::Value addToValue3;

    addToValue3.set_id("a.b.container1.2.container");
    addToValue3.mutable_addtocontainer()->set_key("1");
    *reqMsg.mutable_valuechanged()->add_value() = addToValue3;

    handler_->doMessage(reqMsg);

    EXPECT_EQ(
            coutWrapper_.pop(),
            "server.protocol.handler: Error: doAddToContainer - Not a container: a.b.container1.2.container\n");

    EXPECT_EQ(addedValues, containerFail);
}

TEST_F(ServerProtocolHandlerTest, RemoveFromContainer)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    reqMsg.mutable_vssubscribe();
    handler_->doMessage(reqMsg);
    queueGet();
    reqMsg.Clear();

    ASSERT_TRUE(msgQueue_.empty());
    ASSERT_TRUE(coutWrapper_.empty());

    {
        CompanEdgeProtocol::Value addToValue(containerValue1_);
        addToValue.mutable_addtocontainer()->set_key("0");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;

        handler_->doMessage(reqMsg);
        queueGet();

        ASSERT_TRUE(coutWrapper_.empty());
        ASSERT_TRUE(msgQueue_.empty());

        addToValue.Clear();
        reqMsg.Clear();

        addToValue.set_id("a.b.container1.0.container");
        addToValue.mutable_addtocontainer()->set_key("1");
        *reqMsg.mutable_valuechanged()->add_value() = addToValue;

        handler_->doMessage(reqMsg);
        queueGet();

        ASSERT_TRUE(coutWrapper_.empty());
        ASSERT_TRUE(msgQueue_.empty());

        addToValue.Clear();
        reqMsg.Clear();
    }

    IdValueTypeVector removedValues;
    ws_.connectValueRemovedListener([&removedValues](VariantValue::Ptr const value) {
        removedValues.push_back(std::make_pair(value->id(), value->type()));
    });

    IdValueTypeVector sortedRemovedValues({
            std::make_pair("a.b.container1.0.container.1.eui48", CompanEdgeProtocol::EUI48),
            std::make_pair("a.b.container1.0.container.1", CompanEdgeProtocol::Struct),
            std::make_pair("a.b.container1.0.container", CompanEdgeProtocol::Container),
            std::make_pair("a.b.container1.0.dinterval", CompanEdgeProtocol::DInterval),
            std::make_pair("a.b.container1.0.interval", CompanEdgeProtocol::Interval),
            std::make_pair("a.b.container1.0.struct.bool", CompanEdgeProtocol::Bool),
            std::make_pair("a.b.container1.0.struct", CompanEdgeProtocol::Struct),
            std::make_pair("a.b.container1.0.text", CompanEdgeProtocol::Text),
            std::make_pair("a.b.container1.0", CompanEdgeProtocol::Struct),
    });

    // capture the change notification
    int changeCount(0);
    SignalScopedConnection connection =
            ws_.connectValueRemoveFromContainerListener([this, &changeCount](VariantValue::Ptr const v) {
                if (v->id() == containerValue1_.id()) {
                    EXPECT_TRUE(v->get().has_removefromcontainer());
                    EXPECT_EQ(v->get().removefromcontainer().key(), "0");
                    ++changeCount;
                }
            });

    CompanEdgeProtocol::Value removeFromValue(containerValue1_);
    removeFromValue.mutable_removefromcontainer()->set_key("0");
    *reqMsg.mutable_valuechanged()->add_value() = removeFromValue;

    handler_->doMessage(reqMsg);
    run();

    EXPECT_TRUE(Validate_ValueRemovedOffset(queueGet(), "a.b.container1.0.container.1.eui48", 0));
    EXPECT_TRUE(Validate_ValueRemovedOffset(queueGet(), "a.b.container1.0.container.1", 0));
    EXPECT_TRUE(Validate_ValueRemoveFromContainer(queueGet(), "a.b.container1.0.container.1", 0));
    EXPECT_TRUE(Validate_ValueRemovedOffset(queueGet(), "a.b.container1.0.dinterval", 0));
    EXPECT_TRUE(Validate_ValueRemovedOffset(queueGet(), "a.b.container1.0.interval", 0));
    EXPECT_TRUE(Validate_ValueRemovedOffset(queueGet(), "a.b.container1.0.struct.bool", 0));
    EXPECT_TRUE(Validate_ValueRemovedOffset(queueGet(), "a.b.container1.0.struct", 0));
    EXPECT_TRUE(Validate_ValueRemovedOffset(queueGet(), "a.b.container1.0.text", 0));
    EXPECT_TRUE(Validate_ValueRemovedOffset(queueGet(), "a.b.container1.0", 0));
    EXPECT_TRUE(Validate_ValueRemoveFromContainer(queueGet(), "a.b.container1.0", 0));
    EXPECT_TRUE(Validate_ValueChangedOffset(queueGet(), "a.b.container1", 0));
    EXPECT_TRUE(Validate_ValueChangedOffset(queueGet(), "a.b.container1", 0));
    EXPECT_TRUE(Validate_ValueChangedOffset(queueGet(), "a.b", 0));
    EXPECT_TRUE(Validate_ValueChangedOffset(queueGet(), "a", 0));

    ASSERT_TRUE(coutWrapper_.empty());
    EXPECT_EQ(removedValues, sortedRemovedValues);
    EXPECT_EQ(changeCount, 1);
}
