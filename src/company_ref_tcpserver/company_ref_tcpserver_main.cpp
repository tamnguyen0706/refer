/**
  Copyright © 2023 COMPAN REF
  @file company_ref_ms_manager_main.cpp
  @brief CompanEdge MicroServer Manager
*/

#include "company_ref_tcpserver.h"

#include <company_ref_main_apps/company_ref_app_options.h>
#include <company_ref_main_apps/company_ref_main_io_context.h>
#include <Compan_logger/Compan_logger_sink_cout.h>
#include <Compan_logger/Compan_logger_sink_syslog.h>

#include <iostream>
#include <getopt.h>

using namespace Compan::Edge;

CompanLogger TcpServerMainLog("appmain", LogLevel::Information);

namespace {

int usage(char const* appname, int ret)
{
    std::cout << "Usage: " << appname << " [options] <ip> <port>" << std::endl;
    std::cout << "      ip default: 0.0.0.0" << std::endl;
    std::cout << "      port default: 9998" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -u, --uds               Path to uds socket (ex: "
                 "--uds=/tmp/company_ref_ws.socket)"
              << std::endl;
    std::cout << "  -h, --help              Display this message and exit" << std::endl;
    std::cout << std::endl;
    return ret;
}
} // namespace

int main(int argc, char* argv[])
{
    CompanLoggerSinkSyslog sinkSysLog(false, 0);
    CompanLoggerSinkCout sinkCout(false, 0);

    char const* appName = [&argv]() {
        auto p = strrchr(argv[0], '/');
        return (p ? ++p : argv[0]);
    }();

    std::string udsPath("/tmp/company_ref_ws.socket");

    AppOptionsParser appOptionsParser({
            {'u', "uds", true, false},
            {'h', "help", false, false},
    });

    if (!appOptionsParser.parse(argc, argv)) return usage(appName, 1);

    if (appOptionsParser.has('h')) return usage(appName, 1);

    if (appOptionsParser.has('u')) udsPath = appOptionsParser.single('u');

    int option_index = 0;
    static struct option long_options[] = {{"help", no_argument, 0, 0}, {"uds", required_argument, 0, 0}, {0, 0, 0, 0}};

    while (getopt_long(argc, argv, "hu:", long_options, &option_index) != -1)
        ;

    const std::string szHost = (argc - optind >= 1) ? argv[optind++] : "0.0.0.0";
    const std::string szPort = (argc - optind >= 1) ? argv[optind++] : "9998";

    if (udsPath.empty()) return usage(appName, 1);

    int exitCode = 0;

    try {
        MainIoContext mainIoContext(TcpServerMainLog);

        TcpServer tcpServer(mainIoContext.getIoContext(), udsPath, szHost, szPort);

        mainIoContext.run(1);

        exitCode = 0;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        exitCode = -1;
    }

    return exitCode;
}
