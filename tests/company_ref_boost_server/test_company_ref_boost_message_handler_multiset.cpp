/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_boost_message_handler_multiset.cpp
  @brief Testing VsMultiSet
*/

#include "company_ref_boost_message_handler_mock.h"
#include <gtest/gtest.h>

TEST_F(CompanEdgeBoostMessageHandlerTest, VsMultiSet_Success)
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

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultSetResultSuccess(rspMsg, textId_, 0);
    Validate_VsMultSetResultSuccess(rspMsg, rangedId_, 1);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsMultiSet_UnknownError)
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

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultSetResultFailure(rspMsg, boolId_, 0);
    Validate_VsMultSetResultFailure(rspMsg, rangedId_, 1);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsMultiSet_ValueNotFound)
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

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultSetResultNotFound(rspMsg, nonExistId_, 0);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsMultiSet_Mixed)
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

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultSetResultSuccess(rspMsg, textId_, 0);
    Validate_VsMultSetResultFailure(rspMsg, rangedId_, 1);
    Validate_VsMultSetResultNotFound(rspMsg, nonExistId_, 2);
}
