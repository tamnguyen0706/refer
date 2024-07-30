/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_protocol_message_handler_multiset.cpp
  @brief Testing VsMultiSet
*/

#include "company_ref_asio_server_protocol_handler_mock.h"

TEST_F(ServerProtocolHandlerTest, VsMultiSet_Success)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    ValueIdListener listenerTextId(textId_);
    ValueIdListener listenerRangedId(rangedId_);

    populateValueStore();

    // when using trackable - you must use boost::placeholders
    ws_.get(textId_)->connectChangedListener(
            std::bind(&ValueIdListener::doNotification, &listenerTextId, std::placeholders::_1));
    ws_.get(rangedId_)->connectChangedListener(
            std::bind(&ValueIdListener::doNotification, &listenerRangedId, std::placeholders::_1));

    CompanEdgeProtocol::VsMultiSet* multiSet = reqMsg.mutable_vsmultiset();
    multiSet->set_sequenceno(1);

    CompanEdgeProtocol::VsMultiSet::Value* pValue;
    pValue = multiSet->add_values();
    pValue->set_id(textId_);
    pValue->set_value("hello-there");

    pValue = multiSet->add_values();
    pValue->set_id(rangedId_);
    pValue->set_value("18");

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultSetResultSuccess(rspMsg, textId_, 0);
    Validate_VsMultSetResultSuccess(rspMsg, rangedId_, 1);
}

TEST_F(ServerProtocolHandlerTest, VsMultiSet_UnknownError)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::VsMultiSet* multiSet = reqMsg.mutable_vsmultiset();
    multiSet->set_sequenceno(1);

    CompanEdgeProtocol::VsMultiSet::Value* pValue;
    pValue = multiSet->add_values();
    pValue->set_id(boolId_);
    pValue->set_value("false");

    pValue = multiSet->add_values();
    pValue->set_id(rangedId_);
    pValue->set_value("42");

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultSetResultFailure(rspMsg, boolId_, 0);
    Validate_VsMultSetResultFailure(rspMsg, rangedId_, 1);
}

TEST_F(ServerProtocolHandlerTest, VsMultiSet_ValueNotFound)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::VsMultiSet* multiSet = reqMsg.mutable_vsmultiset();
    multiSet->set_sequenceno(1);

    CompanEdgeProtocol::VsMultiSet::Value* pValue;
    pValue = multiSet->add_values();
    pValue->set_id(nonExistId_);
    pValue->set_value("nope");

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultSetResultNotFound(rspMsg, nonExistId_, 0);
}

TEST_F(ServerProtocolHandlerTest, VsMultiSet_Mixed)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    ValueIdListener listenerTextId(textId_);

    populateValueStore();

    // when using trackable - you must use boost::placeholders
    ws_.get(textId_)->connectChangedListener(
            std::bind(&ValueIdListener::doNotification, &listenerTextId, std::placeholders::_1));

    CompanEdgeProtocol::VsMultiSet* multiSet = reqMsg.mutable_vsmultiset();
    multiSet->set_sequenceno(1);

    CompanEdgeProtocol::VsMultiSet::Value* pValue;
    pValue = multiSet->add_values();
    pValue->set_id(textId_);
    pValue->set_value("hello-there");

    pValue = multiSet->add_values();
    pValue->set_id(rangedId_);
    pValue->set_value("42");

    pValue = multiSet->add_values();
    pValue->set_id(nonExistId_);
    pValue->set_value("nope");

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultSetResultSuccess(rspMsg, textId_, 0);
    Validate_VsMultSetResultFailure(rspMsg, rangedId_, 1);
    Validate_VsMultSetResultNotFound(rspMsg, nonExistId_, 2);
}
