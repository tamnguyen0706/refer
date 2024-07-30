/**
  Copyright © 2024 COMPAN REF
  @file test_company_ref_microservice_client.cpp
  @brief -*-*-*-*-*-
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <company_ref_sdk_value_ids/dynamic_dmo_meta_data.h>
#include <company_ref_sdk_value_ids/logger_meta_data.h>
#include <company_ref_sdk_value_ids/microservices_meta_data.h>

#include <company_ref_dmo/company_ref_dmo_file.h>
#include <company_ref_dmo/company_ref_dmo_helper.h>
#include <company_ref_dynamic_dmo/company_ref_dynamic_dmo_controller.h>
#include <company_ref_main_apps/Compan_logger_factory.h>
#include <company_ref_microservice/company_ref_microservice_client.h>
#include <company_ref_microservice/company_ref_microservice_dynamic_dmo.h>

#include <company_ref_variant_valuestore/company_ref_variant_bool_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>

#include <Compan_logger/Compan_logger.h>
#include <Compan_logger/Compan_logger_controller.h>
#include <Compan_logger/Compan_logger_sink_buffered.h>

#include <company_ref_protocol_utils/company_ref_stream.h>

#include "mock_asio_client.h"

#include <Compan_logger/Compan_logger_sink_cout.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace Compan::Edge;

class MicroServiceClientMockable : public MicroServiceClient {
public:
    MicroServiceClientMockable(
            boost::asio::io_context& ctx,
            std::string const& appName,
            ServerProtocolHandler::Ptr server)
        : MicroServiceClient(ctx, appName)
    {
        MockAsioClient::Ptr client(std::make_shared<MockAsioClient>(ctx, messageHandler(), server));

        clientConnection(client);

        if (client->isConnected()) onClientConnected();
    }

    virtual ~MicroServiceClientMockable() = default;
};

class MicroServiceClientTest : public testing::Test {
public:
    using ValueDefType = std::vector<std::pair<std::string, CompanEdgeProtocol::Value_Type>>;

    MicroServiceClientTest()
        : ctx_()
        , ws_(ctx_)
        , dmo_()
        , connId_(100)
        , system_a_id("system.a")
        , system_b_id("system.b")
        , dmoFileName_("a.dmo")
        , testLog_(__FUNCTION__, LogLevel::Debug)
    {
        // The test mock is behaving like a WS Server
        //  It has the VariantValueStore and Meta Data container

        // Connections are simulated via the MockBoostClient, which contains
        //  a ServerProtocolHandler object, which is what the WS Server uses
        //  to communicate via a UDS or TCP socket
        LoggerMetaData::create(ws_, dmo_);
        DynamicDmoMetaData::create(ws_, dmo_);
        MicroservicesMetaData::create(ws_, dmo_);

        LoggerFactory::createBinders("WsServer", ws_, false);

        controller_ = std::make_unique<DynamicDmoController>(ctx_, ws_, dmo_);
    }

    virtual ~MicroServiceClientTest()
    {
        unlink(dmoFileName_.c_str());
        EXPECT_TRUE(coutWrapper_.empty());
        if (!coutWrapper_.empty()) std::cout << coutWrapper_.pop() << std::endl;

        CompanLoggerController::get()->clearBinders();
    }

    void createMockDmo(DmoContainer& dmo)
    {
        ValueDefType const dmoData(
                {std::make_pair("system.a.i", CompanEdgeProtocol::UInterval),
                 std::make_pair("system.b.c.+.i", CompanEdgeProtocol::UInterval)});

        for (auto& child : dmoData) {
            CompanEdgeProtocol::Value vsValue;
            vsValue.set_id(child.first);
            vsValue.set_type(child.second);
            dmo.insertMetaData(ValueId(child.first), vsValue);
        }

        dmo.sort();
    }

    bool createMockDmoStream(std::ostream& os)
    {
        DmoContainer dmo;

        createMockDmo(dmo);
        return DmoFile::write(os, dmo);
    }

    bool createMockDmoFile(std::string const filePath)
    {
        boost::filesystem::ofstream oFile(boost::filesystem::path(filePath), std::ios::binary);
        if (!oFile.is_open()) return false;

        return createMockDmoStream(oFile);
    }

    void run()
    {
        ctx_.restart();
        ctx_.run();
    }

    boost::asio::io_context ctx_;

    VariantValueStore ws_;
    DmoContainer dmo_;
    std::mutex wsContainerMutex_;
    uint32_t connId_;

    std::unique_ptr<DynamicDmoController> controller_;

    ValueId const system_a_id;
    ValueId const system_b_id;
    std::string const dmoFileName_;

    CompanLogger testLog_;

    CompanLoggerSinkBuffered coutWrapper_;
};

TEST_F(MicroServiceClientTest, SingleClientLogger)
{
    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);
    run();

    // we expect that the microservice's logger functionality has been loaded correctly
    EXPECT_TRUE(clientA.ws().has("logger.A.variantvalue"));
    EXPECT_TRUE(ws_.has("logger.A.variantvalue"));
}

TEST_F(MicroServiceClientTest, MultiClientLogger)
{
    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);
    run();

    auto connB = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientB(ctx_, "B", connB);
    run();

    // checking to make sure that we do NOT have cross polination of
    //  CompanLogger elements
    EXPECT_FALSE(clientA.ws().has("logger.WsServer"));
    EXPECT_TRUE(clientA.ws().has("logger.A.variantvalue"));
    EXPECT_FALSE(clientB.ws().has("logger.A.variantvalue"));

    EXPECT_FALSE(clientB.ws().has("logger.WsServer"));
    EXPECT_FALSE(clientA.ws().has("logger.B.variantvalue"));
    EXPECT_TRUE(clientB.ws().has("logger.B.variantvalue"));
}

TEST_F(MicroServiceClientTest, ClientSubscribeAll)
{
    std::stringstream strm;
    createMockDmoStream(strm);
    DmoFile::read(strm, dmo_, ws_);

    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);
    run();

    auto connB = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientB(ctx_, "B", connB);
    run();

    bool subscribed(false);

    // int subscribeWithCompletion(std::string const& valueId, SubscribeCompleteCb const& cb);
    clientA.subscribeAll([&subscribed](bool const complete) { subscribed = complete; });

    run();

    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(clientA.ws().has(system_a_id));
    EXPECT_TRUE(clientA.ws().has(system_b_id));
    EXPECT_TRUE(clientA.ws().has("logger.A.variantvalue"));
    EXPECT_TRUE(clientA.ws().has("logger.B.variantvalue"));
}

TEST_F(MicroServiceClientTest, ClientSingleSubscriptionPass)
{
    std::stringstream strm;
    createMockDmoStream(strm);
    DmoFile::read(strm, dmo_, ws_);

    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);

    run();

    bool subscribed(false);

    // int subscribeWithCompletion(std::string const& valueId, SubscribeCompleteCb const& cb);
    clientA.subscribeWithCompletion(system_a_id, [&subscribed](bool const complete) { subscribed = complete; });

    run();

    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(clientA.ws().has(system_a_id));
    EXPECT_FALSE(clientA.ws().has(system_b_id));
}

TEST_F(MicroServiceClientTest, ClientSingleSubscriptionFail)
{
    /// subscription notification needs to change - it should pass
    /// a param via the callback
}

TEST_F(MicroServiceClientTest, ClientMultiSubscriptionPass)
{
    std::stringstream strm;
    createMockDmoStream(strm);
    DmoFile::read(strm, dmo_, ws_);

    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);
    run();

    bool subscribed(false);

    // int subscribeWithCompletion(std::vector<std::string> const& valueIds, SubscribeCompleteCb const& cb);
    clientA.subscribeWithCompletion(
            {system_a_id, system_b_id}, [&subscribed](bool const complete) { subscribed = complete; });

    run();

    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(clientA.ws().has(system_a_id));
    EXPECT_TRUE(clientA.ws().has(system_b_id));
}

TEST_F(MicroServiceClientTest, ClientAutoReconnect)
{
    std::stringstream strm;
    createMockDmoStream(strm);
    DmoFile::read(strm, dmo_, ws_);

    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);
    run();

    bool subscribed(false);

    // int subscribeWithCompletion(std::string const& valueId, SubscribeCompleteCb const& cb);
    clientA.subscribeWithCompletion(system_a_id, [&subscribed](bool const complete) { subscribed = complete; });

    run();

    // disconnect
    clientA.clientConnection()->close();

    // update a local value
    VariantUIntervalValue::Ptr localPtr = clientA.ws().get<VariantUIntervalValue>(ValueId(system_a_id, "i"));
    ASSERT_NE(localPtr, nullptr);
    localPtr->set(1);

    // check the remote value
    VariantUIntervalValue::Ptr remotePtr = ws_.get<VariantUIntervalValue>(ValueId(system_a_id, "i"));
    ASSERT_NE(remotePtr, nullptr);

    EXPECT_NE(localPtr->get(), remotePtr->get());

    run();

    EXPECT_EQ(localPtr->get(), remotePtr->get());
}

TEST_F(MicroServiceClientTest, MultiListeners)
{
    using Map = std::map<ValueId, int>;
    int updateCount(0);

    std::stringstream strm;
    createMockDmoStream(strm);
    DmoFile::read(strm, dmo_, ws_);

    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);
    run();

    auto connB = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientB(ctx_, "B", connB);
    run();

    auto connC = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientC(ctx_, "C", connC);
    run();

    bool subscribed(false);

    // int subscribeWithCompletion(std::string const& valueId, SubscribeCompleteCb const& cb);
    clientA.subscribeAll([&subscribed](bool const complete) { subscribed = complete; });
    clientB.subscribeAll([&subscribed](bool const complete) { subscribed = complete; });
    clientC.subscribeAll([&subscribed](bool const complete) { subscribed = complete; });

    run();

    // Grab all the instances of system.a.i across all microservices
    VariantUIntervalValue::Ptr localA = clientA.ws().get<VariantUIntervalValue>(ValueId(system_a_id, "i"));
    VariantUIntervalValue::Ptr localB = clientB.ws().get<VariantUIntervalValue>(ValueId(system_a_id, "i"));
    VariantUIntervalValue::Ptr localC = clientC.ws().get<VariantUIntervalValue>(ValueId(system_a_id, "i"));

    // Listen for the changes from the various value stores

    Map changeWs;
    ws_.connectValueChangedListener([&changeWs](VariantValue::Ptr const valuePtr) { ++changeWs[valuePtr->id()]; });

    Map changeA;
    clientA.ws().connectValueChangedListener(
            [&changeA](VariantValue::Ptr const valuePtr) { ++changeA[valuePtr->id()]; });

    Map changeB;
    clientB.ws().connectValueChangedListener(
            [&changeB](VariantValue::Ptr const valuePtr) { ++changeB[valuePtr->id()]; });

    Map changeC;
    clientC.ws().connectValueChangedListener(
            [&changeC](VariantValue::Ptr const valuePtr) { ++changeC[valuePtr->id()]; });

    int localA_count = 0;
    localA->connectChangedListener([&localA_count](VariantValue::Ptr const valuePtr) {
        if (valuePtr->setUpdateType() == VariantValue::Local) return;
        ++localA_count;
    });

    int localB_count = 0;
    localB->connectChangedListener([&localB_count](VariantValue::Ptr const valuePtr) {
        if (valuePtr->setUpdateType() == VariantValue::Local) return;
        ++localB_count;
    });

    int localC_count = 0;
    localC->connectChangedListener([&localC_count](VariantValue::Ptr const valuePtr) {
        if (valuePtr->setUpdateType() == VariantValue::Local) return;
        ++localC_count;
    });

    run();

    localA->set(1);

    run();

    updateCount = 0;
    for (auto& element : changeWs) {
        ASSERT_EQ(element.second, 1);
        ++updateCount;
    }
    EXPECT_EQ(updateCount, 3);

    updateCount = 0;
    for (auto& element : changeA) {
        ASSERT_EQ(element.second, 1);
        ++updateCount;
    }
    EXPECT_EQ(updateCount, 3);

    updateCount = 0;
    for (auto& element : changeB) {
        ASSERT_EQ(element.second, 1);
        ++updateCount;
    }
    EXPECT_EQ(updateCount, 3);

    updateCount = 0;
    for (auto& element : changeC) {
        ASSERT_EQ(element.second, 1);
        ++updateCount;
    }
    EXPECT_EQ(updateCount, 3);

    EXPECT_EQ(localA_count, 1);
    EXPECT_EQ(localB_count, 1);
    EXPECT_EQ(localC_count, 1);

    ASSERT_EQ(clientA.ws().get(ValueId(system_a_id, "i"))->str(), "1");
    ASSERT_EQ(clientB.ws().get(ValueId(system_a_id, "i"))->str(), "1");
    ASSERT_EQ(clientC.ws().get(ValueId(system_a_id, "i"))->str(), "1");
}

TEST_F(MicroServiceClientTest, ClientPidCompletionUpdate)
{
    std::stringstream strm;
    createMockDmoStream(strm);
    DmoFile::read(strm, dmo_, ws_);

    /// Microservice Manager "adds" this information
    DmoValueStoreHelper(dmo_, ws_).insertChild("system.microservices", "A", VariantValue::Remote);

    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);
    run();

    bool subscribed(false);

    // int subscribeWithCompletion(std::string const& valueId, SubscribeCompleteCb const& cb);
    clientA.subscribeWithCompletion(system_a_id, [&subscribed](bool const complete) { subscribed = complete; });

    run();

    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(clientA.ws().has(system_a_id));
    EXPECT_FALSE(clientA.ws().has(system_b_id));

    run();

    ASSERT_TRUE(clientA.ws().has("system.microservices.A.status.pid"));
    EXPECT_EQ(clientA.ws().get<VariantUIntervalValue>("system.microservices.A.status.pid")->get(), getpid());

    clientA.setStartupCompleted();
    run();

    ASSERT_TRUE(clientA.ws().has("system.microservices.A.status.startupComplete"));
    EXPECT_TRUE(clientA.ws().get<VariantBoolValue>("system.microservices.A.status.startupComplete")->get());
}

TEST_F(MicroServiceClientTest, DynamicDmoLoader)
{
    auto connA = std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, connId_++);
    MicroServiceClientMockable clientA(ctx_, "A", connA);
    run();

    EXPECT_FALSE(ws_.has("system.ws.dmo.A.config.action"));
    EXPECT_FALSE(clientA.ws().has("system.ws.dmo.A.config.action"));

    auto dynamicDmo = clientA.getDynamicDmo();

    bool subscribed(false);
    dynamicDmo->init([&subscribed](bool const result) { subscribed = result; });
    run();
    EXPECT_TRUE(subscribed);

    EXPECT_TRUE(ws_.has("system.ws.dmo.A.config.action"));
    EXPECT_TRUE(clientA.ws().has("system.ws.dmo.A.config.action"));

    bool notFound(false);
    dynamicDmo->load(dmoFileName_, [&notFound](DynamicDmoValueIds::Dmo::Status::ActionEnum const result) {
        notFound = (result == DynamicDmoValueIds::Dmo::Status::FileNotFound);
    });
    run();
    EXPECT_TRUE(notFound);

    EXPECT_EQ(coutWrapper_.pop(), "dynamicdmo.loader: Warning: \"a.dmo\" doesn't exist\n");

    createMockDmoFile(dmoFileName_);

    bool successLoad(false);
    dynamicDmo->load(dmoFileName_, [&successLoad](DynamicDmoValueIds::Dmo::Status::ActionEnum const result) {
        successLoad = (result == DynamicDmoValueIds::Dmo::Status::Complete);
    });
    run();
    EXPECT_TRUE(successLoad);
}
