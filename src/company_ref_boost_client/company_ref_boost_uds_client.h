/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_uds_client.cpp
  @brief CompanEdge Boost - UDS client
*/

#ifndef __company_ref_BOOST_CLIENT_company_ref_BOOST_UDS_CLIENT_H__
#define __company_ref_BOOST_CLIENT_company_ref_BOOST_UDS_CLIENT_H__

#include "company_ref_boost_client.h"

#include <boost/asio/local/stream_protocol.hpp>

namespace Compan{
namespace Edge {

extern template class CompanEdgeBoostClient<boost::asio::local::stream_protocol>;

/*!
 * @brief Boost.ASIO UDS Client
 */
class CompanEdgeBoostUdsClient : public CompanEdgeBoostClient<boost::asio::local::stream_protocol> {
public:
    /*!
     * @brief Constructs a TCP client connection
     *
     * @param   Boost.ASIO io_context object
     * @param   UDS socket path
     */
    CompanEdgeBoostUdsClient(boost::asio::io_context&, std::string const&);
    virtual ~CompanEdgeBoostUdsClient();
};
} // namespace Edge
} // namespace Compan

#endif /*__company_ref_BOOST_CLIENT_company_ref_BOOST_UDS_CLIENT_H__*/
