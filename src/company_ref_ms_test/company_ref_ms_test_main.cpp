/**
  Copyright © 2024 COMPAN REF
*/

#include <company_ref_main_apps/company_ref_app_options.h>
#include <company_ref_main_apps/company_ref_main_io_context.h>

#include <Compan_logger/Compan_logger_sink_cout.h>
#include <Compan_logger/Compan_logger_sink_syslog.h>

#include "ms_test_microservice.h"
#include <thread>

using namespace Compan::Edge;

CompanLogger StressorLog("appmain", LogLevel::Information);

namespace {

int usage(char const* appname, int ret)
{
    std::cout << "Usage: " << appname << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -u, --uds <path>         Path to uds socket (ex: --uds=/tmp/company_ref_ws.socket)" << std::endl;
    std::cout << "  -i, --ip addr<:port>   IP Address and port " << std::endl;
    std::cout << "  -t, --timeout <seconds>  Timeout for test to complete (default 2 seconds)" << std::endl;
    std::cout << " === " << std::endl;
    std::cout << " Select only ONE test type" << std::endl;
    std::cout << " === " << std::endl;
    std::cout << "  -s, --subcribe <valueid> Subscribe to a ValueId" << std::endl;
    std::cout << "  -c, --container <valueid>  Monitor add/remove container" << std::endl;
    std::cout << "  -h, --help               Display this message and exit" << std::endl;
    std::cout << std::endl;
    return ret;
}

} // namespace

#include <unistd.h>

int main(int argc, char* argv[])
{
    CompanLoggerSinkSyslog sinkSysLog(false, 0);
    CompanLoggerSinkCout sinkCout(false, 0);

    char const* appName = [&argv]() {
        auto p = strrchr(argv[0], '/');
        return (p ? ++p : argv[0]);
    }();

    std::string udsPath("/tmp/company_ref_ws.socket");
    int timeOut = 1;

    AppOptionsParser appOptionsParser({
            {'u', "uds", true, false},
            {'i', "ip", true, false},
            {'t', "timeout", true, false},
            {'s', "subscribe", true, false},
            {'c', "container", true, false},
            {'h', "help", false, false},
    });

    if (!appOptionsParser.parse(argc, argv)) return usage(appName, 1);

    if (appOptionsParser.has('h')) return usage(appName, 1);

    if (appOptionsParser.has('u')) udsPath = appOptionsParser.single('u');
    if (appOptionsParser.has('t')) timeOut = std::stoul(appOptionsParser.single('t'));

    if (appOptionsParser.has('i') && appOptionsParser.has('u')) return usage(appName, 2);

    if (appOptionsParser.has('s') && appOptionsParser.has('c')) return usage(appName, 2);

    int exitCode = 0;

    try {
        MainIoContext mainIoContext(StressorLog);

        MsTestMicroSerivce::Ptr msTest;

        if (appOptionsParser.has('i')) {
            std::string ip = appOptionsParser.single('i');

            std::string addr = ip;
            std::string port = "9998";

            size_t pos = ip.find(':');
            if (pos != std::string::npos) {
                addr = ip.substr(0, pos);
                port = ip.substr(pos + 1);
            }

            std::cout << "Connecting " << addr << ":" << port << std::endl;
            msTest = std::make_unique<MsTestMicroSerivce>(mainIoContext.getIoContext(), "MsTest", timeOut, addr, port);
        } else {
            std::cout << "Connecting " << udsPath << std::endl;

            msTest = std::make_unique<MsTestMicroSerivce>(mainIoContext.getIoContext(), "MsTest", timeOut, udsPath);
        }

        if (appOptionsParser.has('s')) msTest->doSubscribe(appOptionsParser.single('s'));

        if (appOptionsParser.has('c')) msTest->doContainer(appOptionsParser.single('c'));

        mainIoContext.run(1);

        exitCode = 0;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        exitCode = -1;
    }

    return exitCode;

    return 0;
}
