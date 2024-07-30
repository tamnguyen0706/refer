/**
 Copyright © 2024 COMPAN REF
 @file test_base.cpp
 @brief -*-*-*-*-*-
 */
#include "test_base.h"

using namespace Compan::Edge;

TestBase::TestBase(boost::asio::io_context& ctx, MicroServiceClient& msClient)
    : ctx_(ctx)
    , msClient_(msClient)
{
}

TestBase::~TestBase()
{
}
