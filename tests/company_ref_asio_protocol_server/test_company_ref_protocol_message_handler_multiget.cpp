/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_protocol_message_handler_multiget.cpp
  @brief Testing VsMultiGet
*/

#include "company_ref_asio_server_protocol_handler_mock.h"

TEST_F(ServerProtocolHandlerTest, VsMultiGet_Success)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::VsMultiGet* multiGet = reqMsg.mutable_vsmultiget();
    multiGet->set_sequenceno(1);
    *multiGet->add_ids() = textId_;
    *multiGet->add_ids() = boolId_;

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultGetResultSuccess(rspMsg, textId_, 0);
    Validate_VsMultGetResultSuccess(rspMsg, boolId_, 1);
}

TEST_F(ServerProtocolHandlerTest, VsMultiGet_ValueNotFound)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::VsMultiGet* multiGet = reqMsg.mutable_vsmultiget();
    multiGet->set_sequenceno(1);
    *multiGet->add_ids() = nonExistId_;

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultGetResultNotFound(rspMsg, 0);
}

TEST_F(ServerProtocolHandlerTest, VsMultiGet_Mixed)
{
    CompanEdgeProtocol::ClientMessage rspMsg;
    CompanEdgeProtocol::ServerMessage reqMsg;

    populateValueStore();

    CompanEdgeProtocol::VsMultiGet* multiGet = reqMsg.mutable_vsmultiget();
    multiGet->set_sequenceno(1);
    *multiGet->add_ids() = textId_;
    *multiGet->add_ids() = nonExistId_;
    *multiGet->add_ids() = boolId_;

    handler_->doMessage(reqMsg);

    rspMsg = queueGet();

    Validate_VsMultGetResultSuccess(rspMsg, textId_, 0);
    Validate_VsMultGetResultNotFound(rspMsg, 1);
    Validate_VsMultGetResultSuccess(rspMsg, boolId_, 2);
}
