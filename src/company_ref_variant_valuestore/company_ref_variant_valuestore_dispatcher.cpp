/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_dispatcher.cpp
 @brief VariantValue Data dispatcher
 */

#include "company_ref_variant_valuestore_dispatcher.h"
#include <boost/asio/post.hpp>

#include <iostream>
using namespace Compan::Edge;

VariantValueDispatcher::VariantValueDispatcher()
    : workGuard_(boost::asio::make_work_guard(ctx_))
{
}

void VariantValueDispatcher::start()
{
    thread_ = std::thread([&] { ctx_.run(); });
}

void VariantValueDispatcher::stop()
{
    workGuard_.reset();
    thread_.join();
}

void VariantValueDispatcher::getCompanEdgeProtocolValue(ProtocolValueGetPtr taskPtr)
{
    boost::asio::post(ctx_, std::bind(&ProtocolValueGet::operator(), taskPtr));
}

void VariantValueDispatcher::setCompanEdgeProtocolValue(ProtocolValueSetPtr taskPtr, CompanEdgeProtocol::Value const& data)
{
    boost::asio::post(ctx_, std::bind(&ProtocolValueSet::operator(), taskPtr, data));
}

void VariantValueDispatcher::hasDataCompanEdgeProtocolValue(ProtocolValueHasDataPtr taskPtr)
{
    boost::asio::post(ctx_, std::bind(&ProtocolValueHasData::operator(), taskPtr));
}
