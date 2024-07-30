/**
 Copyright © 2023 COMPAN REF
 @file company_ref_asio_server_protocol_handler_mock.cpp
 @brief Mock Message Handler
 */

#include "company_ref_asio_server_protocol_handler_mock.h"

#include <company_ref_variant_valuestore/company_ref_variant_factory.h>

ServerProtocolHandlerTest::ServerProtocolHandlerTest()
    : ctx_()
    , strand_(ctx_)
    , ws_(ctx_)
    , msgQueue_()
    , handler_(std::make_shared<ServerProtocolHandler>(ws_, dmo_, wsContainerMutex_, 0))
    , boolId_("test.bool")
    , enumId_("test.enum")
    , rangedId_("test.ranged")
    , textId_("test.string")
    , structId_("a.b")
    , containerId1_("a.b.container1")
    , containerId2_("a.b.container2")
    , nonExistId_("test.notfound")
{
    ServerProtocolHandlerLog.logLevel(LogLevel::Information);

    boolValue_.set_id(boolId_);
    boolValue_.set_type(CompanEdgeProtocol::Bool);
    boolValue_.set_access(CompanEdgeProtocol::Value_Access_ReadOnly);
    boolValue_.mutable_boolvalue()->set_value(true);

    textValue_.set_id(textId_);
    textValue_.set_type(CompanEdgeProtocol::Text);
    textValue_.set_access(CompanEdgeProtocol::Value_Access_ReadWrite);
    textValue_.mutable_textvalue()->set_value("Compan-networks.com");

    intervalValue_.set_id(rangedId_);
    intervalValue_.set_type(CompanEdgeProtocol::Interval);
    intervalValue_.set_access(CompanEdgeProtocol::Value_Access_ReadWrite);

    CompanValueTypes::IntervalValue rangeValue;
    rangeValue.set_value(15);
    rangeValue.set_min(10);
    rangeValue.set_max(20);
    *intervalValue_.mutable_intervalvalue() = std::move(rangeValue);

    std::set<std::pair<uint32_t, std::string>> enumStrings({{0, "No"}, {1, "Yes"}, {2, "Maybe"}});

    enumValue_.set_id(enumId_);
    enumValue_.set_type(CompanEdgeProtocol::Enum);
    enumValue_.set_access(CompanEdgeProtocol::Value_Access_ReadWrite);
    for (auto& it : enumStrings) {
        CompanValueTypes::EnumValue::Enumerator enumerator;
        enumerator.set_value(it.first);
        enumerator.set_text(it.second);
        *enumValue_.mutable_enumvalue()->add_enumerators() = std::move(enumerator);
    }

    structValue_.set_id(structId_);
    structValue_.set_type(CompanEdgeProtocol::Struct);

    containerValue1_.set_id(containerId1_);
    containerValue1_.set_type(CompanEdgeProtocol::Container);
    containerValue1_.set_access(CompanEdgeProtocol::Value_Access_ReadWrite);

    containerValue2_.set_id(containerId2_);
    containerValue2_.set_type(CompanEdgeProtocol::Container);
    containerValue2_.set_access(CompanEdgeProtocol::Value_Access_ReadWrite);

    handler_->connectSendCallback(std::bind(&ServerProtocolHandlerTest::messageCompleted, this, std::placeholders::_1));
}

ServerProtocolHandlerTest::~ServerProtocolHandlerTest()
{
    run();

    EXPECT_TRUE(coutWrapper_.empty());
    if (!coutWrapper_.empty()) std::cout << coutWrapper_.pop() << std::endl;

    EXPECT_TRUE(msgQueue_.empty());
    while (!msgQueue_.empty()) {
        showRspMsg(msgQueue_.front());
        msgQueue_.pop();
    }
}

void ServerProtocolHandlerTest::showRspMsg(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    if (rspMsg.has_valuechanged()) std::cout << rspMsg.valuechanged();
    if (rspMsg.has_valueremoved()) std::cout << rspMsg.valueremoved();
    if (rspMsg.has_vsresult()) std::cout << rspMsg.vsresult();
}

void ServerProtocolHandlerTest::messageCompleted(std::shared_ptr<CompanEdgeProtocol::ClientMessage> msgPtr)
{
    if (!msgPtr) return;

    if (msgPtr->has_valuechanged() || msgPtr->has_valueremoved() || msgPtr->has_vsresult()
        || msgPtr->has_vsmultigetresult() || msgPtr->has_vsmultisetresult() || msgPtr->has_vssynccompleted())
        msgQueue_.push(*msgPtr);
}

void ServerProtocolHandlerTest::populateValueStore()
{
    ws_.add(VariantFactory::make(strand_, boolValue_));
    ws_.add(VariantFactory::make(strand_, textValue_));
    ws_.add(VariantFactory::make(strand_, intervalValue_));
    ws_.add(VariantFactory::make(strand_, enumValue_));
    ws_.add(VariantFactory::make(strand_, structValue_));
    ws_.add(VariantFactory::make(strand_, containerValue1_));
    ws_.add(VariantFactory::make(strand_, containerValue2_));

    IdValueTypeVector metaDefs({
            std::make_pair("a.b.container1.+.interval", CompanEdgeProtocol::Interval),
            std::make_pair("a.b.container1.+.text", CompanEdgeProtocol::Text),
            std::make_pair("a.b.container1.+.dinterval", CompanEdgeProtocol::DInterval),
            std::make_pair("a.b.container1.+.struct", CompanEdgeProtocol::Struct),
            std::make_pair("a.b.container1.+.struct.bool", CompanEdgeProtocol::Bool),
            std::make_pair("a.b.container1.+.container", CompanEdgeProtocol::Container),
            std::make_pair("a.b.container1.+.container.+.eui48", CompanEdgeProtocol::EUI48),
            std::make_pair("a.b.container2.+.bool", CompanEdgeProtocol::Bool),
    });

    EXPECT_TRUE(dmo_.insertValue(containerValue1_));
    EXPECT_TRUE(dmo_.insertValue(containerValue2_));

    for (auto& child : metaDefs) {

        VariantValue::Ptr vsValue = std::make_shared<VariantValue>(strand_, child.first, child.second);

        switch (child.second) {
        case CompanEdgeProtocol::Interval: vsValue->set("42"); break;

        case CompanEdgeProtocol::Text: vsValue->set("Hello"); break;

        case CompanEdgeProtocol::DInterval: vsValue->set("4.2"); break;

        case CompanEdgeProtocol::Bool: vsValue->set("true"); break;

        case CompanEdgeProtocol::EUI48: vsValue->set("0abeef"); break;

        default: break;
        }

        EXPECT_TRUE(dmo_.insertMetaData(ValueId(child.first), vsValue->get()));
    }
}

uint64_t ServerProtocolHandlerTest::getHashToken(std::string const& valueId)
{
    VariantValue::Ptr valuePtr = ws_.get(valueId);
    if (valuePtr) return valuePtr->hashToken();

    return 0;
}

CompanEdgeProtocol::ClientMessage ServerProtocolHandlerTest::queueGet()
{
    CompanEdgeProtocol::ClientMessage rspMsg;

    run();

    if (!msgQueue_.empty()) {
        rspMsg = msgQueue_.front();
        msgQueue_.pop();
    }

    return rspMsg;
}

void ServerProtocolHandlerTest::Validate_ClientMessageHasNoVsResult(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    EXPECT_FALSE(rspMsg.has_vsresult());
}

void ServerProtocolHandlerTest::Validate_VsResultSuccess(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    EXPECT_EQ(rspMsg.vsresult().values().size(), 0);
    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_success);
}

void ServerProtocolHandlerTest::Validate_VsResultNotFound(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    EXPECT_EQ(rspMsg.vsresult().values().size(), 0);
    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_error_not_found);
}

void ServerProtocolHandlerTest::Validate_VsResultWrongValueType(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    EXPECT_EQ(rspMsg.vsresult().values().size(), 1);
    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_wrong_value_type);
}

void ServerProtocolHandlerTest::Validate_VsResultRangeError(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    EXPECT_EQ(rspMsg.vsresult().values().size(), 1);
    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_range_error);
}

void ServerProtocolHandlerTest::Validate_VsResultEnumError(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    EXPECT_EQ(rspMsg.vsresult().values().size(), 1);
    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_enum_error);
}

void ServerProtocolHandlerTest::Validate_VsResultAccessError(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    EXPECT_EQ(rspMsg.vsresult().values().size(), 1);
    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_access_error);
}

void ServerProtocolHandlerTest::Validate_VsResultAll(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    ASSERT_EQ(rspMsg.vsresult().values().size(), ws_.size());
    for (int i = 0; i < rspMsg.vsresult().values().size(); ++i) {
        VariantValue::Ptr valuePtr = ws_.get(rspMsg.vsresult().values(i).id());
        ASSERT_TRUE(valuePtr);

        VariantValue result(strand_, rspMsg.vsresult().values(i));

        EXPECT_EQ(valuePtr->id(), result.id());
        EXPECT_EQ(valuePtr->type(), result.type());
        EXPECT_EQ(valuePtr->hashToken(), result.hashToken());
        EXPECT_EQ(valuePtr->access(), result.access());
        EXPECT_EQ(valuePtr->str(), result.str());
    }

    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_success);
}

void ServerProtocolHandlerTest::Validate_VsResultSingle(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId)
{
    ASSERT_EQ(rspMsg.vsresult().values().size(), 1);

    VariantValue::Ptr valuePtr = ws_.get(valueId);
    ASSERT_TRUE(valuePtr);

    VariantValue result(strand_, rspMsg.vsresult().values(0));

    EXPECT_EQ(valuePtr->id(), result.id());
    EXPECT_EQ(valuePtr->type(), result.type());
    EXPECT_EQ(valuePtr->hashToken(), result.hashToken());
    EXPECT_EQ(valuePtr->access(), result.access());
    EXPECT_EQ(valuePtr->str(), result.str());

    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_success);
}

void ServerProtocolHandlerTest::Validate_VsResultOffset(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    ASSERT_TRUE(idx < rspMsg.vsresult().values().size());

    VariantValue::Ptr valuePtr = ws_.get(valueId);
    ASSERT_TRUE(valuePtr);

    VariantValue result(strand_, rspMsg.vsresult().values(idx));

    EXPECT_EQ(valuePtr->id(), result.id());
    EXPECT_EQ(valuePtr->type(), result.type());
    EXPECT_EQ(valuePtr->hashToken(), result.hashToken());
    EXPECT_EQ(valuePtr->access(), result.access());
    EXPECT_EQ(valuePtr->str(), result.str());

    EXPECT_EQ(rspMsg.vsresult().status(), CompanEdgeProtocol::VsResult_Status_success);
}

void ServerProtocolHandlerTest::Validate_ValueChangedAll(CompanEdgeProtocol::ClientMessage const& rspMsg)
{
    ASSERT_EQ(rspMsg.valuechanged().value().size(), ws_.size());
    for (int i = 0; i < rspMsg.valuechanged().value().size(); ++i) {
        VariantValue::Ptr valuePtr = ws_.get(rspMsg.valuechanged().value(i).id());

        ASSERT_TRUE(valuePtr);

        VariantValue result(strand_, rspMsg.valuechanged().value(i));

        EXPECT_EQ(valuePtr->id(), result.id());
        EXPECT_EQ(valuePtr->type(), result.type());
        EXPECT_EQ(valuePtr->hashToken(), result.hashToken());
        EXPECT_EQ(valuePtr->access(), result.access());
        EXPECT_EQ(valuePtr->str(), result.str());
    }
}

bool ServerProtocolHandlerTest::Validate_ValueAddToContainer(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    if (!rspMsg.has_valuechanged()) return false;

    bool const assertSize = (idx < rspMsg.valuechanged().value().size());
    EXPECT_TRUE(assertSize);
    if (!assertSize) return assertSize;

    VariantValue::Ptr valuePtr = ws_.get(valueId);

    if (valuePtr == nullptr) return false;

    VariantValue result(strand_, rspMsg.valuechanged().value(idx));

    EXPECT_EQ(valuePtr->id(), result.id());
    EXPECT_EQ(result.type(), CompanEdgeProtocol::ContainerAddTo);
    EXPECT_EQ(valuePtr->hashToken(), result.hashToken());
    EXPECT_TRUE(result.get().has_addtocontainer());

    return (valuePtr->id() == result.id() && result.type() == CompanEdgeProtocol::ContainerAddTo
            && valuePtr->hashToken() == result.hashToken() && result.get().has_addtocontainer());
}

bool ServerProtocolHandlerTest::Validate_ValueRemoveFromContainer(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    EXPECT_TRUE(rspMsg.has_valuechanged());
    if (!rspMsg.has_valuechanged()) return false;

    bool const assertSize = (idx < rspMsg.valuechanged().value().size());
    EXPECT_TRUE(assertSize);
    if (!assertSize) return assertSize;

    VariantValue result(strand_, rspMsg.valuechanged().value(idx));

    ValueId removeId(valueId);

    EXPECT_EQ(rspMsg.valuechanged().value(idx).id(), removeId.parent().name());
    EXPECT_EQ(rspMsg.valuechanged().value(idx).type(), CompanEdgeProtocol::ContainerRemoveFrom);
    EXPECT_TRUE(rspMsg.valuechanged().value(idx).has_removefromcontainer());
    EXPECT_EQ(rspMsg.valuechanged().value(idx).removefromcontainer().key(), removeId.leaf().name());

    return (rspMsg.valuechanged().value(idx).id() == removeId.parent().name()
            && rspMsg.valuechanged().value(idx).type() == CompanEdgeProtocol::ContainerRemoveFrom
            && rspMsg.valuechanged().value(idx).has_removefromcontainer()
            && rspMsg.valuechanged().value(idx).removefromcontainer().key() == removeId.leaf().name());
}

bool ServerProtocolHandlerTest::Validate_ValueChangedSingle(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId)
{
    return Validate_ValueChangedOffset(rspMsg, valueId, 0);
}

bool ServerProtocolHandlerTest::Validate_ValueChangedOffset(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    if (!rspMsg.has_valuechanged()) return false;

    bool const assertSize = (idx < rspMsg.valuechanged().value().size());

    EXPECT_TRUE(assertSize);
    if (!assertSize) return assertSize;

    VariantValue::Ptr valuePtr = ws_.get(valueId);
    if (valuePtr == nullptr) return false;

    VariantValue result(strand_, rspMsg.valuechanged().value(idx));

    EXPECT_EQ(valuePtr->id(), result.id());
    EXPECT_EQ(valuePtr->type(), result.type());
    EXPECT_EQ(valuePtr->hashToken(), result.hashToken());
    EXPECT_EQ(valuePtr->access(), result.access());
    EXPECT_EQ(valuePtr->str(), result.str());

    return (valuePtr->id() == result.id() && valuePtr->type() == result.type()
            && valuePtr->hashToken() == result.hashToken() && valuePtr->access() == result.access()
            && valuePtr->str() == result.str());
}

bool ServerProtocolHandlerTest::Validate_ValueTransientChangedOffset(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    if (!rspMsg.has_valuechanged()) return false;

    bool const assertSize = (idx < rspMsg.valuechanged().value().size());

    EXPECT_TRUE(assertSize);
    if (!assertSize) return assertSize;

    EXPECT_EQ(rspMsg.valuechanged().value(idx).id(), valueId);

    return (rspMsg.valuechanged().value(idx).id() == valueId);
}

void ServerProtocolHandlerTest::Validate_ValueRemoved(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        uint64_t const hashToken)
{
    ASSERT_EQ(rspMsg.valueremoved().id().size(), 1);
    EXPECT_EQ(rspMsg.valueremoved().id(0), valueId);

    ASSERT_EQ(rspMsg.valueremoved().hashtoken().size(), 1);
    EXPECT_EQ(rspMsg.valueremoved().hashtoken(0), hashToken);
}

bool ServerProtocolHandlerTest::Validate_ValueRemovedOffset(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    if (!rspMsg.has_valueremoved()) return false;

    bool const assertSize = (idx < rspMsg.valueremoved().id().size());

    EXPECT_TRUE(assertSize);
    if (!assertSize) return assertSize;

    EXPECT_EQ(rspMsg.valueremoved().id(idx), valueId);

    return (rspMsg.valueremoved().id(idx) == valueId);
}

void ServerProtocolHandlerTest::Validate_VsMultGetResultSuccess(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    ASSERT_TRUE(idx < rspMsg.vsmultigetresult().results().size());

    VariantValue::Ptr valuePtr = ws_.get(valueId);

    ASSERT_TRUE(valuePtr);

    EXPECT_EQ(rspMsg.vsmultigetresult().results(idx).error(), CompanEdgeProtocol::VsMultiGetResult::Success);
    VariantValue result(strand_, rspMsg.vsmultigetresult().results(idx).values(0));

    EXPECT_EQ(valuePtr->id(), result.id());
    EXPECT_EQ(valuePtr->type(), result.type());
    EXPECT_EQ(valuePtr->hashToken(), result.hashToken());
    EXPECT_EQ(valuePtr->access(), result.access());
    EXPECT_EQ(valuePtr->str(), result.str());
}

void ServerProtocolHandlerTest::Validate_VsMultGetResultNotFound(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        int const idx)
{
    ASSERT_TRUE(idx < rspMsg.vsmultigetresult().results().size());

    EXPECT_EQ(rspMsg.vsmultigetresult().results(idx).error(), CompanEdgeProtocol::VsMultiGetResult::ValueNotFound);
}

void ServerProtocolHandlerTest::Validate_VsMultSetResultSuccess(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    ASSERT_TRUE(idx < rspMsg.vsmultisetresult().results().size());

    EXPECT_EQ(rspMsg.vsmultisetresult().results(idx).error(), CompanEdgeProtocol::VsMultiSetResult::Success);
    EXPECT_EQ(rspMsg.vsmultisetresult().results(idx).id(), valueId);
}

void ServerProtocolHandlerTest::Validate_VsMultSetResultNotFound(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    ASSERT_TRUE(idx < rspMsg.vsmultisetresult().results().size());

    EXPECT_EQ(rspMsg.vsmultisetresult().results(idx).error(), CompanEdgeProtocol::VsMultiSetResult::ValueNotFound);
    EXPECT_EQ(rspMsg.vsmultisetresult().results(idx).id(), valueId);
}

void ServerProtocolHandlerTest::Validate_VsMultSetResultFailure(
        CompanEdgeProtocol::ClientMessage const& rspMsg,
        std::string const& valueId,
        int const idx)
{
    ASSERT_TRUE(idx < rspMsg.vsmultisetresult().results().size());

    EXPECT_EQ(rspMsg.vsmultisetresult().results(idx).error(), CompanEdgeProtocol::VsMultiSetResult::UnknownError);
    EXPECT_EQ(rspMsg.vsmultisetresult().results(idx).id(), valueId);
}

void ServerProtocolHandlerTest::run()
{
    ctx_.restart();
    ctx_.run();
}

ValueIdListener::ValueIdListener(std::string const& valueId)
    : ValueIdListener(valueId, 1, 0)
{
}

ValueIdListener::ValueIdListener(std::string const& valueId, int const expected, int const other)
    : valueId_(valueId)
    , max_expected_(expected)
    , count_expected_(0)
    , max_other_(other)
    , count_other_(0)
{
}

ValueIdListener::~ValueIdListener()
{
    EXPECT_EQ(count_expected_, max_expected_);
    EXPECT_EQ(count_other_, max_other_);
}

void ValueIdListener::doNotification(VariantValue::Ptr const valuePtr)
{
    if (valuePtr->id() == valueId_)
        ++count_expected_;
    else
        ++count_other_;
}
