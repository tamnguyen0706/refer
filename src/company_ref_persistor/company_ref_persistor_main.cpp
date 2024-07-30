/**
  Copyright © 2023 COMPAN REF
  @file company_ref_persistor_main.cpp
  @brief CompanEdge Persistor microservice
*/

#include "company_ref_persistor_bl.h"

#include <company_ref_main_apps/company_ref_app_options.h>
#include <company_ref_main_apps/company_ref_main_io_context.h>

#include <Compan_logger/Compan_logger_sink_cout.h>
#include <Compan_logger/Compan_logger_sink_syslog.h>

#include <fstream>
#include <iostream>

using namespace Compan::Edge;

namespace Compan{
namespace Edge {
CompanLogger PersistorMainLog("appmain", LogLevel::Information);
} // namespace Edge
} // namespace Compan

namespace {
int usage(char const* appname, int ret)
{
    std::cout << "Usage: " << appname << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c, --cfg               Path to cfg file (ex: --cfg=/tmp/cfg.ini)" << std::endl;
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
    std::string cfgPath("");

    AppOptionsParser appOptionsParser({
            {'u', "uds", true, false},
            {'c', "cfg", true, true},
            {'h', "help", false, false},
    });

    if (!appOptionsParser.parse(argc, argv)) return usage(appName, 1);

    if (appOptionsParser.has('h')) return usage(appName, 1);

    if (appOptionsParser.has('u')) udsPath = appOptionsParser.single('u');
    if (appOptionsParser.has('c')) cfgPath = appOptionsParser.single('c');

    if (udsPath.empty()) {
        std::cout << "Missing uds path" << std::endl;
        return usage(appName, 1);
    }
    if (cfgPath.empty()) {
        std::cout << "Missing cfg path" << std::endl;
        return usage(appName, 1);
    }

    {
        std::ifstream file(cfgPath);
        if (!file.is_open()) {
            ErrorLog(PersistorMainLog) << "failed to open cfg file: [" << cfgPath << "]" << std::endl;
            return usage(appName, 1);
        }
    }

    int exitCode = 0;

    try {
        MainIoContext mainIoContext(PersistorMainLog);

        PersistorBl persistor(mainIoContext.getIoContext(), udsPath, cfgPath);

        // low consuming application doesn't require multiple threads
        mainIoContext.run(1);

        exitCode = 0;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        exitCode = -1;
    }

    return exitCode;
}
