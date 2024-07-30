/**
  Copyright © 2023 COMPAN REF
  @file company_ref_ws_command.h
  @brief CompanEdge Variant ValueStore (WS) - command
*/

#ifndef __company_ref_WS_COMMAND_H__
#define __company_ref_WS_COMMAND_H__

#include <boost/asio/io_context.hpp>

#include <fstream>
#include <functional>
#include <string>

namespace Compan{
namespace Edge {

class CompanEdgeWsCommand {
public:
    using CommandCompleteCb = std::function<void(bool)>;
    CompanEdgeWsCommand();
    virtual ~CompanEdgeWsCommand();
    virtual void process(boost::asio::io_context&) = 0;

    // allow command to notify it is finishded
    void setCommandCb(CommandCompleteCb const&);

protected:
    CommandCompleteCb cb_;
};

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_WS_COMMAND_H__*/
