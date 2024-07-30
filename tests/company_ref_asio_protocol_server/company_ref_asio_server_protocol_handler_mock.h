/**
 Copyright © 2023 COMPAN REF
 @file company_ref_asio_server_protocol_handler_mock.h
 @brief Mock Message Handler
 */
#ifndef __company_ref_ASIO_SERVER_PROTOCOL_HANDLER__MOCK_H__
#define __company_ref_ASIO_SERVER_PROTOCOL_HANDLER__MOCK_H__

#include <gmock/gmock.h>

#include "Compan_logger_sink_buffered.h"

#include "company_ref_variant_valuestore/company_ref_variant_valuestore.h"
#include <company_ref_asio_protocol_server/company_ref_asio_server_protocol_handler.h>
#include <company_ref_dmo/company_ref_dmo_container.h>

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_stream.h>

#include <Compan_logger/Compan_logger.h>

#include <queue>

using namespace Compan::Edge;

// use this for getting notifications in change events
extern CompanLogger ServerProtocolHandlerLog;

class ServerProtocolHandlerTest : public testing::Test {
public:
    typedef std::vector<std::pair<std::string, CompanEdgeProtocol::Value_Type>> IdValueTypeVector;

    ServerProtocolHandlerTest();

    virtual ~ServerProtocolHandlerTest();

    void messageCompleted(std::shared_ptr<CompanEdgeProtocol::ClientMessage> msg);

    void populateValueStore();

    void showRspMsg(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void run();

    uint64_t getHashToken(std::string const& valueId);

    CompanEdgeProtocol::ClientMessage queueGet();

    void Validate_ClientMessageHasNoVsResult(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void Validate_VsResultSuccess(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void Validate_VsResultNotFound(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void Validate_VsResultWrongValueType(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void Validate_VsResultRangeError(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void Validate_VsResultEnumError(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void Validate_VsResultAccessError(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void Validate_VsResultAll(CompanEdgeProtocol::ClientMessage const& rspMsg);

    void Validate_VsResultSingle(CompanEdgeProtocol::ClientMessage const& rspMsg, std::string const& valueId);

    void Validate_VsResultOffset(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx);

    void Validate_ValueChangedAll(CompanEdgeProtocol::ClientMessage const& rspMsg);

    bool Validate_ValueChangedSingle(CompanEdgeProtocol::ClientMessage const& rspMsg, std::string const& valueId);

    bool Validate_ValueAddToContainer(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx = 0);

    bool Validate_ValueRemoveFromContainer(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx = 0);

    bool Validate_ValueChangedOffset(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx);

    bool Validate_ValueTransientChangedOffset(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx);

    void Validate_ValueRemoved(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            uint64_t const hashToken);

    bool Validate_ValueRemovedOffset(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx);

    void Validate_VsMultGetResultSuccess(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx);
    void Validate_VsMultGetResultNotFound(CompanEdgeProtocol::ClientMessage const& rspMsg, int const idx);

    void Validate_VsMultSetResultSuccess(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx);
    void Validate_VsMultSetResultNotFound(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx);
    void Validate_VsMultSetResultFailure(
            CompanEdgeProtocol::ClientMessage const& rspMsg,
            std::string const& valueId,
            int const idx);

    boost::asio::io_context ctx_;
    boost::asio::io_context::strand strand_;

    VariantValueStore ws_;
    DmoContainer dmo_;
    std::mutex wsContainerMutex_;

    std::queue<CompanEdgeProtocol::ClientMessage> msgQueue_;

    ServerProtocolHandler::Ptr handler_;

    std::string const boolId_;
    std::string const enumId_;
    std::string const rangedId_;
    std::string const textId_;
    std::string const structId_;
    std::string const containerId1_;
    std::string const containerId2_;

    std::string const nonExistId_;

    /// sample of 4 "types" of values
    CompanEdgeProtocol::Value boolValue_;
    CompanEdgeProtocol::Value enumValue_;
    CompanEdgeProtocol::Value intervalValue_;
    CompanEdgeProtocol::Value textValue_;
    CompanEdgeProtocol::Value structValue_;
    CompanEdgeProtocol::Value containerValue1_;
    CompanEdgeProtocol::Value containerValue2_;

    CompanLoggerSinkBuffered coutWrapper_;
};

class ValueIdListener {
public:
    ValueIdListener(std::string const& valueId);
    ValueIdListener(std::string const& valueId, int const expected, int const other);
    virtual ~ValueIdListener();
    void doNotification(VariantValue::Ptr const valuePtr);

    std::string const valueId_;
    int const max_expected_;
    int count_expected_;
    int const max_other_;
    int count_other_;
};

#endif // __company_ref_ASIO_SERVER_PROTOCOL_HANDLER__MOCK_H__
