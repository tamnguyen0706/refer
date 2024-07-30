/**
 Copyright © 2024 COMPAN REF
 @file company_ref_ws_main_app.h
 @brief Main application class
 */
#ifndef __company_ref_WS_MAIN_APP_H__
#define __company_ref_WS_MAIN_APP_H__

#include <company_ref_asio/company_ref_asio_tcp_server.h>
#include <company_ref_asio/company_ref_asio_uds_server.h>
#include <company_ref_asio_protocol_server/company_ref_asio_server_msg_handler_factory.h>
#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_dynamic_dmo/company_ref_dynamic_dmo_controller.h>
#include <company_ref_main_apps/company_ref_main_io_context.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

namespace Compan{
namespace Edge {

/*!
 *
 */
class MainApp {
public:
    MainApp();
    virtual ~MainApp() = default;

    int run(int argc, char* argv[]);

protected:
    bool parseCmdLine(int argc, char* argv[]);
    int usage(int const ret);

    bool loadDmoFromCmdLine();

    void startUdsServer();
    void startTcpServer();

private:
    MainIoContext mainIo_;
    VariantValueStore ws_;
    DmoContainer dmo_;

    ServerMsgHandlerFactory handlerFactory_;
    std::unique_ptr<AsioUdsServer> udsServer_;
    std::unique_ptr<AsioTcpServer> tcpServer_;

    std::unique_ptr<DynamicDmoController> controller_;

    std::string appName_;
    std::string udsPath_;       // -u
    std::string loadPath_;      // -l
    std::string restorePath_;   // -r
    std::string dmoPath_;       // -d
    std::string persistedPath_; // -p
    bool enableTcp_;            // -t
    int threads_;               // -c
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_WS_MAIN_APP_H__
