/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_main.cpp
  @brief Entry point
*/

#include <company_ref_cli_bl.h>
#include <company_ref_cli_command.h>
#include <company_ref_cli_container.h>
#include <company_ref_cli_invoke.h>
#include <company_ref_cli_output.h>

#include <company_ref_boost_client/company_ref_boost_client.h>
#include <company_ref_boost_client/company_ref_boost_tcp_client.h>
#include <company_ref_boost_client/company_ref_boost_uds_client.h>
#include <company_ref_utils/company_ref_string_utils.h>
#include <Compan_logger/Compan_logger_sink_cout.h>
#include <Compan_logger/Compan_logger_sink_syslog.h>

#include <iostream>
#include <memory>
#include <vector>

#include <boost/filesystem.hpp>
#include <getopt.h>

using namespace Compan::Edge;

namespace {
const std::string szDefaultUdsPath("/tmp/company_ref_ws.socket");
struct option long_options[] = {
        {"addtocontainer", required_argument, 0, 'A'},
        {"format", required_argument, 0, 'f'},
        {"get", required_argument, 0, 'g'},
        {"getall", no_argument, 0, 'G'},
        {"getobject", required_argument, 0, 'o'},
        {"monitor", no_argument, 0, 'm'},
        {"multiget", required_argument, 0, 'O'},
        {"multiset", required_argument, 0, 'S'},
        {"removefromcontainer", required_argument, 0, 'R'},
        {"set", required_argument, 0, 's'},
        {"invoke", required_argument, 0, 'i'},
        {"trigger", required_argument, 0, 't'},
        {"uds", required_argument, 0, 'u'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

int usage(char const* appname, int ret)
{
    std::cout << "Usage: " << appname << " [options] [<ip> <port>]" << std::endl;
    std::cout << "default: uds=" << szDefaultUdsPath << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -A, --addtocontainer      Add a container to value store (ex: "
                 "-A <containerId>,<keyId>)"
              << std::endl;
    std::cout << "  -f, --format              Enable get and set value format. Supported format:"
                 " [\"tr069\", \"Compan\"]. Default is \"Compan\""
              << std::endl;
    std::cout << "  -g, --get                 Get value (ex: --get=hardware.cpu)" << std::endl;
    std::cout << "  -O, --multiget            Getting multiple values within the same execution (ex: "
                 "-O system.deviceInfo.status.serialNumber,system.time.status.uptime)"
              << std::endl;
    std::cout << "  -G, --getall              Get all values" << std::endl;
    std::cout << "  -o, --getobject           Get values with all the children (ex: "
                 "--getobject=system.deviceInfo.status.*)"
              << std::endl;
    std::cout << "  -m, --monitor             Monitor value changes" << std::endl;
    std::cout << "  -R, --removefromcontainer Remove an item from a container (ex: "
                 "--removefromcontainer=<containerId>,<keyId>)"
              << std::endl;
    std::cout << "  -s, --set                 Set value (ex: --set=logger.System=Debug)" << std::endl;
    std::cout << "  -S, --multiset            Setting multiple values at the same execution (ex: "
                 "-S logger.WsServer.appmain=Debug,system.description=\"\\\"Compan: name, desc\\\"\")"
              << std::endl;
    std::cout << "  -i, --invoke              Run executable on init and when value changes (ex: "
                 "--invoke=system.reboot,/sbin/reboot_system.sh)"
              << std::endl;
    std::cout << "  -t, --trigger             Like invoke except it doesn't exec script on init (ex: "
                 "--trigger=system.reboot,/sbin/reboot_system.sh)"
              << std::endl;
    std::cout << "  -u, --uds                 Path to uds socket (optional; default to "
                 "--uds=/tmp/company_ref_ws.socket)"
              << std::endl;
    std::cout << "  -v, --verbose             Display human informative output" << std::endl;
    std::cout << "  -h, --help                Display this message and exit" << std::endl;
    std::cout << std::endl;
    return ret;
}
} // namespace

void parseInputCmdString(const std::string& csvCommand, std::vector<std::pair<std::string, std::string>>& cmdList)
{
    std::vector<std::string> tokenList;
    StringUtils::variableParse(tokenList, csvCommand);
    // std::cout << "Input: [" << csvCommand << "]" << std::endl; size_t count(0);
    for (auto& item : tokenList) {
        auto equal = item.find_first_of('=');
        bool isSetCmd = !(equal == std::string::npos);

        std::string id = item.substr(0, equal);
        std::string value("");
        if (isSetCmd) { value = item.substr(equal + 1); }
        cmdList.push_back(std::make_pair(id, value));
        // std::cout << "cmd " << ++count << " id:" << id << ":" << value << std::endl;
    }
}

int main(int argc, char* argv[])
{
    CompanLoggerSinkSyslog sinkSysLog(false, 0);
    CompanLoggerSinkCout sinkCout(false, 0);

    char const* appName = [&argv]() {
        auto p = strrchr(argv[0], '/');
        return (p ? ++p : argv[0]);
    }();

    if (argc < 2) return usage(appName, 1);

    std::vector<std::unique_ptr<CompanEdgeCliCommand>> commandQueue;

    boost::asio::io_context ioContext;

    bool monitorValues(false);
    bool verboseOutput(false);
    FormatOption format(FORMAT_Compan);
    ProtocolValueMap savedValues;

    std::string udsPath(szDefaultUdsPath);
    int c = 0, option_index = 0;
    std::string const argOptions("A:f:g:O:Go:hR:s:S:i:t:u:mv");
    while ((c = getopt_long(argc, argv, argOptions.c_str(), long_options, &option_index)) != -1) {
        switch (c) {
        case 'A': {
            std::string v(optarg);
            auto p = v.find_first_of(',');
            if (p == std::string::npos || (p + 1) >= v.size()) return usage(appName, 1);
            commandQueue.emplace_back(std::make_unique<CompanEdgeCliContainer>(
                    ioContext, v.substr(0, p), v.substr(p + 1), true, savedValues));
        } break;
        case 'f': {
            std::string v(optarg);
            if ("tr069" == v) {
                format = FORMAT_TR069;
            } else {
                if ("Compan" != v) { std::cout << "Invalid format option, default to \"Compan\"" << std::endl; }
            }
        } break;
        case 'g': commandQueue.emplace_back(std::make_unique<Get>(optarg, savedValues)); break;
        case 'O': {
            std::string v(optarg);
            std::vector<std::pair<std::string, std::string>> ids;
            parseInputCmdString(v, ids);
            if ((ids.size() <= 0)) return usage(appName, 1);
            commandQueue.emplace_back(std::make_unique<MultiGet>(ids, savedValues));
        } break;
        case 'G': commandQueue.emplace_back(std::make_unique<GetAll>(savedValues)); break;
        case 'o': commandQueue.emplace_back(std::make_unique<GetObject>(optarg, savedValues)); break;
        case 'h': return usage(appName, 0);
        case 'R': {
            std::string v(optarg);
            auto p = v.find_first_of(',');
            if (p == std::string::npos || (p + 1) >= v.size()) return usage(appName, 1);
            const std::string fqvid(v.substr(0, p) + "." + v.substr(p + 1));
            commandQueue.emplace_back(std::make_unique<Get>(fqvid, savedValues));
            commandQueue.emplace_back(std::make_unique<CompanEdgeCliContainer>(
                    ioContext, v.substr(0, p), v.substr(p + 1), false, savedValues));
        } break;
        case 's': {
            if (!optarg) return usage(appName, 1); // option used but no argument is provided
            std::string v(optarg); // if len == 0, option provided an explicit empty string, else full argument
            auto p = v.find_first_of('=');
            if (p == std::string::npos) return usage(appName, 1);
            std::vector<std::pair<std::string, std::string>> ids;
            ids.push_back(std::pair<std::string, std::string>(v.substr(0, p), v.substr(p + 1)));
            commandQueue.emplace_back(std::make_unique<GetValues>(ids, savedValues));
            commandQueue.emplace_back(std::make_unique<Set>(ids.front().first, ids.front().second, savedValues));
        } break;
        case 'S': {
            std::string v(optarg);
            std::vector<std::pair<std::string, std::string>> ids;
            parseInputCmdString(v, ids);
            if ((ids.size() <= 0)) return usage(appName, 1);
            commandQueue.emplace_back(std::make_unique<GetValues>(ids, savedValues));
            commandQueue.emplace_back(std::make_unique<MultiSet>(ids, savedValues));
        } break;
        case 'i': {
            std::string v(optarg);
            const bool onChangedOnly(false);
            auto p = v.find_first_of(',');
            if (p == std::string::npos || (p + 1) >= v.size()) return usage(appName, 1);
            commandQueue.emplace_back(std::make_unique<CompanEdgeCliInvoke>(
                    ioContext, v.substr(0, p), v.substr(p + 1), onChangedOnly, savedValues));
        } break;
        case 't': {
            const bool onChangedOnly(true);
            std::string v(optarg);
            auto p = v.find_first_of(',');
            if (p == std::string::npos || (p + 1) >= v.size()) return usage(appName, 1);
            commandQueue.emplace_back(std::make_unique<CompanEdgeCliInvoke>(
                    ioContext, v.substr(0, p), v.substr(p + 1), onChangedOnly, savedValues));
        } break;
        case 'u': {
            udsPath = optarg;
            if (!boost::filesystem::exists(boost::filesystem::path(udsPath))) return usage(appName, 1);
        } break;
        case 'm': monitorValues = true; break;
        case 'v': verboseOutput = true; break;
        case '?':
        default: return usage(appName, 1);
        }
    }

    if (monitorValues) commandQueue.emplace_back(std::make_unique<EnableMonitor>(savedValues));

    if (FORMAT_TR069 == format) {
        // note: this should probably be a simple vector of strings, but GetValues cmd is expecting a vector of pairs
        std::vector<std::pair<std::string, std::string>> cliGetIds = {{Compan_SYSTEM_CONFIG_TIMEZONE, ""}};
        commandQueue.insert(commandQueue.begin(), std::make_unique<GetValues>(cliGetIds, savedValues));
    }

    std::shared_ptr<CompanEdgeCliOutput> output;
    if (verboseOutput)
        output = std::make_shared<CompanEdgeCliVerboseOutput>();
    else
        output = std::make_shared<CompanEdgeCliSimpleOutput>();

    for (auto& cmd : commandQueue) {
        cmd->setPrinter(output);
        cmd->setFormatOption(format);
    }

    const std::string szHost = (argc - optind >= 1) ? argv[optind++] : "";
    const std::string szPort = (argc - optind >= 1) ? argv[optind++] : "9998";

    int exitCode = 0;
    try {
        std::shared_ptr<CompanEdgeBoostClientBase> client(
                ((szHost.size() && szPort.size()) ?
                         static_cast<CompanEdgeBoostClientBase*>(new CompanEdgeBoostTcpClient(ioContext, szHost, szPort)) :
                         static_cast<CompanEdgeBoostClientBase*>(new CompanEdgeBoostUdsClient(ioContext, udsPath))));

        CompanEdgeCliBl bl(ioContext, client, std::move(commandQueue));

        if (monitorValues) { bl.forceRead(true); }

        ioContext.run();

        exitCode = bl.exitStatus();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        exitCode = -1;
    }

    return exitCode;
}
