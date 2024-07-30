/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_tcp_client.h
  @brief CompanEdge Boost - TCP client
*/

#ifndef __company_ref_BOOST_CLIENT_company_ref_BOOST_TCP_CLIENT_H__
#define __company_ref_BOOST_CLIENT_company_ref_BOOST_TCP_CLIENT_H__

#include <boost/asio/ip/tcp.hpp>

#include "company_ref_boost_client.h"

namespace Compan{
namespace Edge {
extern template class CompanEdgeBoostClient<boost::asio::ip::tcp>;

/*!
 * @brief Boost.ASIO TCP Client
 */
class CompanEdgeBoostTcpClient : public CompanEdgeBoostClient<boost::asio::ip::tcp> {
public:
    /*!
     * @brief Constructs a TCP client connection
     *
     * @param   Boost.ASIO io_context object
     * @param   Host name
     * @param   Port name
     */
    CompanEdgeBoostTcpClient(boost::asio::io_context&, std::string const&, std::string const&);
    virtual ~CompanEdgeBoostTcpClient();

private:
    void doResolve(boost::asio::ip::tcp::resolver::query const&);
    void onResolve(boost::system::error_code const&, boost::asio::ip::tcp::resolver::iterator);
    void doResolveConnect(boost::asio::ip::tcp::resolver::iterator);
    void onResolveConnect(boost::system::error_code const&, boost::asio::ip::tcp::resolver::iterator);

private:
    boost::asio::ip::tcp::resolver resolver_;
};
} // namespace Edge
} // namespace Compan

#endif /*__company_ref_BOOST_CLIENT_company_ref_BOOST_TCP_CLIENT_H__*/
