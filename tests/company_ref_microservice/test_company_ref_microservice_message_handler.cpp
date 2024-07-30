/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_microservice_message_handler.cpp
  @brief MicroService handler test
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Compan_logger_sink_buffered.h"
#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_variant_valuestore/company_ref_variant_factory.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <company_ref_asio_protocol_server/company_ref_asio_server_protocol_handler.h>
#include <company_ref_microservice/company_ref_microservice_message_handler.h>

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_stream.h>

#include <fstream>

using namespace Compan::Edge;

class MicroServiceMessageHandlerTest : public testing::Test {
public:
    typedef std::vector<std::pair<std::string, CompanEdgeProtocol::Value_Type>> IdValueTypeVector;

    MicroServiceMessageHandlerTest()
        : ctx_()
        , strand_(ctx_)
        , wsServer_(ctx_)
        , wsClient_(ctx_)
        , seqNo_(0)
        , serverHandler_(std::make_shared<ServerProtocolHandler>(wsServer_, dmo_, 0))
        , clientHandler_(wsClient_, seqNo_)
        , textId_("test.text_")
        , boolId_("test.bool")
        , structId_("test")
        , containerId_("test.container")
        , nonExistId_("test.notfound")
        , metaDefs_({
                  std::make_pair("test.container.+.bool", CompanEdgeProtocol::Bool),
                  std::make_pair("test.container.+.interval", CompanEdgeProtocol::Interval),
                  std::make_pair("test.container.+.text", CompanEdgeProtocol::Text),
          })
    {
        textValue_.set_id(textId_);
        textValue_.set_type(CompanEdgeProtocol::Text);
        textValue_.set_access(CompanEdgeProtocol::Value_Access_ReadWrite);
        textValue_.mutable_textvalue()->set_value("Compan");
        wsServer_.add(VariantFactory::make(strand_, textValue_));

        boolValue_.set_id(boolId_);
        boolValue_.set_type(CompanEdgeProtocol::Bool);
        boolValue_.set_access(CompanEdgeProtocol::Value_Access_ReadOnly);
        boolValue_.mutable_boolvalue()->set_value(true);
        wsServer_.add(VariantFactory::make(strand_, boolValue_));

        structValue_.set_id(structId_);
        structValue_.set_type(CompanEdgeProtocol::Struct);
        wsServer_.add(VariantFactory::make(strand_, structValue_));

        containerValue_.set_id(containerId_);
        containerValue_.set_type(CompanEdgeProtocol::Container);
        wsServer_.add(VariantFactory::make(strand_, containerValue_));

        dmo_.insertMetaContainer(containerId_, CompanEdgeProtocol::Container);

        for (auto& child : metaDefs_) {

            CompanEdgeProtocol::Value vsValue;
            vsValue.set_id(child.first);
            vsValue.set_type(child.second);
            dmo_.insertMetaData(ValueId(child.first), vsValue);
        }

        serverHandler_->connectSendCallback(
                std::bind(&MicroServiceMessageHandlerTest::serverMessageHandler, this, std::placeholders::_1));

        clientHandler_.setSendServerMessageCb(
                std::bind(&MicroServiceMessageHandlerTest::clientMessageHandler, this, std::placeholders::_1));
    }

    virtual ~MicroServiceMessageHandlerTest()
    {
        EXPECT_TRUE(coutWrapper_.empty());
        if (!coutWrapper_.empty()) std::cout << coutWrapper_.pop() << std::endl;
    }

    void serverMessageHandler(std::shared_ptr<CompanEdgeProtocol::ClientMessage> msgPtr)
    {
        if (!msgPtr) return;
        clientHandler_.handleMessage(*msgPtr);
    }

    void clientMessageHandler(CompanEdgeProtocol::ServerMessage&& msg)
    {
        CompanEdgeProtocol::ServerMessage moved = std::move(msg);
        serverHandler_->doMessage(moved);
    }

    void subscribeCompleteHandler(uint32_t seqNo)
    {
        EXPECT_EQ(seqNo_, seqNo);
    }

    uint64_t getHashToken(std::string const& valueId)
    {
        VariantValue::Ptr valuePtr = wsServer_.get(valueId);
        return valuePtr->hashToken();
    }

    void run()
    {
        ctx_.restart();
        ctx_.run();
    }

    boost::asio::io_context ctx_;
    boost::asio::io_context::strand strand_;

    VariantValueStore wsServer_;
    VariantValueStore wsClient_;

    uint32_t seqNo_;

    DmoContainer dmo_;

    ServerProtocolHandler::Ptr serverHandler_;
    MicroServiceMessageHandler clientHandler_;

    std::string const textId_;
    std::string const boolId_;
    std::string const structId_;
    std::string const containerId_;

    std::string const nonExistId_;

    CompanEdgeProtocol::Value textValue_;
    CompanEdgeProtocol::Value boolValue_;
    CompanEdgeProtocol::Value structValue_;
    CompanEdgeProtocol::Value containerValue_;

    typedef std::vector<std::pair<std::string, CompanEdgeProtocol::Value_Type>> ValueDefType;
    ValueDefType metaDefs_;

    CompanLoggerSinkBuffered coutWrapper_;
};

TEST_F(MicroServiceMessageHandlerTest, BlindRegistration)
{
    wsClient_.add(std::make_shared<VariantValue>(strand_, textId_, CompanEdgeProtocol::Unset));

    run();

    VariantValue::Ptr validate = wsClient_.get(textId_);

    EXPECT_EQ(validate->type(), textValue_.type());
}

TEST_F(MicroServiceMessageHandlerTest, FailedRegistration)
{
    wsClient_.add(std::make_shared<VariantValue>(strand_, nonExistId_, CompanEdgeProtocol::Unset));

    run();

    EXPECT_FALSE(wsClient_.has(nonExistId_));
}

TEST_F(MicroServiceMessageHandlerTest, ReadOnlyTest)
{
    wsClient_.add(std::make_shared<VariantValue>(strand_, boolId_, CompanEdgeProtocol::Unset));

    run();

    VariantValue::Ptr validate = wsClient_.get(boolId_);

    EXPECT_EQ(validate->type(), boolValue_.type());
    EXPECT_EQ(validate->str(), "true");

    EXPECT_EQ(validate->set("false"), ValueDataSet::AccessError);

    EXPECT_EQ(validate->str(), "true");
}

TEST_F(MicroServiceMessageHandlerTest, Remove)
{
    wsServer_.del(boolId_);
    EXPECT_FALSE(wsClient_.has(boolId_));

    wsClient_.add(std::make_shared<VariantValue>(strand_, boolId_, CompanEdgeProtocol::Unset));
    run();

    EXPECT_FALSE(wsClient_.has(boolId_));
}

TEST_F(MicroServiceMessageHandlerTest, VsSync)
{
    CompanEdgeProtocol::ServerMessage reqMsg;

    EXPECT_TRUE(wsClient_.size() == 0);

    reqMsg.mutable_vssync();
    serverHandler_->doMessage(reqMsg);
    run();

    wsServer_.visitValues([this](VariantValue::Ptr const& valuePtr) { EXPECT_TRUE(wsClient_.has(valuePtr->id())); });
}

TEST_F(MicroServiceMessageHandlerTest, AddToContainer)
{
    CompanEdgeProtocol::ServerMessage reqMsg;

    CompanEdgeProtocol::VsSubscribe* pVsSubscribe = reqMsg.mutable_vssubscribe();
    pVsSubscribe->set_sequenceno(1);
    pVsSubscribe->add_ids(containerId_);

    serverHandler_->doMessage(reqMsg);
    run();

    VariantMapValue::Ptr mapPtr = wsClient_.get<VariantMapValue>(containerId_);
    ASSERT_TRUE(mapPtr);

    ValueDefType updatedValues;

    bool added(false);
    mapPtr->connectValueAddToContainerListener([&added, &updatedValues](VariantValue::Ptr const valuePtr) {
        if (valuePtr->type() != CompanEdgeProtocol::ContainerAddTo) return;

        if (valuePtr->setUpdateType() != VariantValue::Remote) return;

        updatedValues.push_back(std::make_pair(valuePtr->id(), valuePtr->type()));
        added = true;
    });
    wsClient_.connectValueAddedListener([&updatedValues](VariantValue::Ptr const valuePtr) {
        if (valuePtr->setUpdateType() != VariantValue::Remote) return;
        updatedValues.push_back(std::make_pair(valuePtr->id(), valuePtr->type()));
    });

    mapPtr->insert("1");

    EXPECT_FALSE(wsClient_.has("test.container.1"));
    EXPECT_FALSE(wsClient_.has("test.container.1.bool"));
    EXPECT_FALSE(wsClient_.has("test.container.1.interval"));
    EXPECT_FALSE(wsClient_.has("test.container.1.text"));

    run();

    EXPECT_TRUE(wsClient_.has("test.container.1"));
    EXPECT_TRUE(wsClient_.has("test.container.1.bool"));
    EXPECT_TRUE(wsClient_.has("test.container.1.interval"));
    EXPECT_TRUE(wsClient_.has("test.container.1.text"));

    EXPECT_TRUE(added);

    ValueDefType expectedValues(
            {std::make_pair("test.container.1", CompanEdgeProtocol::Struct),
             std::make_pair("test.container.1.bool", CompanEdgeProtocol::Bool),
             std::make_pair("test.container.1.interval", CompanEdgeProtocol::Interval),
             std::make_pair("test.container.1.text", CompanEdgeProtocol::Text),
             std::make_pair("test.container", CompanEdgeProtocol::ContainerAddTo)});

    EXPECT_EQ(updatedValues, expectedValues);
}

TEST_F(MicroServiceMessageHandlerTest, RemoveFromContainer)
{
    CompanEdgeProtocol::ServerMessage reqMsg;

    CompanEdgeProtocol::VsSubscribe* pVsSubscribe = reqMsg.mutable_vssubscribe();
    pVsSubscribe->set_sequenceno(1);
    pVsSubscribe->add_ids(containerId_);

    serverHandler_->doMessage(reqMsg);
    run();

    VariantMapValue::Ptr mapPtr = wsClient_.get<VariantMapValue>(containerId_);
    ASSERT_TRUE(mapPtr);

    ValueDefType removedValues;

    bool removed(false);
    mapPtr->connectValueRemoveFromContainerListener([&removed, &removedValues](VariantValue::Ptr const valuePtr) {
        if (valuePtr->type() != CompanEdgeProtocol::ContainerRemoveFrom) return;

        if (valuePtr->setUpdateType() != VariantValue::Remote) return;

        removedValues.push_back(std::make_pair(valuePtr->id(), valuePtr->type()));

        removed = true;
    });

    wsClient_.connectValueRemovedListener([&removedValues](VariantValue::Ptr const valuePtr) {
        removedValues.push_back(std::make_pair(valuePtr->id(), valuePtr->type()));
    });

    mapPtr->insert("1");
    run();

    EXPECT_TRUE(wsClient_.has("test.container.1"));
    EXPECT_TRUE(wsClient_.has("test.container.1.bool"));
    EXPECT_TRUE(wsClient_.has("test.container.1.interval"));
    EXPECT_TRUE(wsClient_.has("test.container.1.text"));

    mapPtr->erase("1");
    run();

    EXPECT_TRUE(wsClient_.has("test"));
    EXPECT_TRUE(wsClient_.has("test.container"));

    EXPECT_FALSE(wsClient_.has("test.container.1"));
    EXPECT_FALSE(wsClient_.has("test.container.1.bool"));
    EXPECT_FALSE(wsClient_.has("test.container.1.interval"));
    EXPECT_FALSE(wsClient_.has("test.container.1.text"));

    EXPECT_TRUE(removed);

    ValueDefType expectedValues(
            {std::make_pair("test.container.1.bool", CompanEdgeProtocol::Bool),
             std::make_pair("test.container.1.interval", CompanEdgeProtocol::Interval),
             std::make_pair("test.container.1.text", CompanEdgeProtocol::Text),
             std::make_pair("test.container.1", CompanEdgeProtocol::Struct),
             std::make_pair("test.container", CompanEdgeProtocol::ContainerRemoveFrom)});

    EXPECT_EQ(removedValues, expectedValues);
}
