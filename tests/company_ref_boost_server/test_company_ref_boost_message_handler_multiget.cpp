/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_boost_message_handler_multiget.cpp
  @brief Testing VsMultiGet
*/

#include "company_ref_boost_message_handler_mock.h"
#include <gtest/gtest.h>

TEST_F(CompanEdgeBoostMessageHandlerTest, VsMultiGet_Success)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::VsMultiGet* multiGet = reqMsg.mutable_vsmultiget();
    multiGet->set_sequenceno(1);
    *multiGet->add_ids() = textId_;
    *multiGet->add_ids() = boolId_;

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultGetResultSuccess(rspMsg, textId_, 0);
    Validate_VsMultGetResultSuccess(rspMsg, boolId_, 1);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsMultiGet_ValueNotFound)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::VsMultiGet* multiGet = reqMsg.mutable_vsmultiget();
    multiGet->set_sequenceno(1);
    *multiGet->add_ids() = nonExistId_;

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultGetResultNotFound(rspMsg, 0);
}

TEST_F(CompanEdgeBoostMessageHandlerTest, VsMultiGet_Mixed)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::VsMultiGet* multiGet = reqMsg.mutable_vsmultiget();
    multiGet->set_sequenceno(1);
    *multiGet->add_ids() = textId_;
    *multiGet->add_ids() = nonExistId_;
    *multiGet->add_ids() = boolId_;

    handler_->handleMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultGetResultSuccess(rspMsg, textId_, 0);
    Validate_VsMultGetResultNotFound(rspMsg, 1);
    Validate_VsMultGetResultSuccess(rspMsg, boolId_, 2);
}
