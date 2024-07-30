/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_uds_client.cpp
  @brief CompanEdge Boost - UDS client
*/

#include "company_ref_boost_uds_client.h"

#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {
CompanLogger AebUdsClientLog("client.connection.uds", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

CompanEdgeBoostUdsClient::CompanEdgeBoostUdsClient(boost::asio::io_context& io_context, std::string const& path)
    : CompanEdgeBoostClient<boost::asio::local::stream_protocol>(io_context)
{
    endPoint_ = EndPointType(path);

    boost::asio::post(ioContext_, std::bind(&CompanEdgeBoostUdsClient::doConnect, this));
}

CompanEdgeBoostUdsClient::~CompanEdgeBoostUdsClient() = default;
