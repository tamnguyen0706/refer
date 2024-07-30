/**
  Copyright © 2023 COMPAN REF
  @file dmo_from_ini_main.cpp
  @brief Entry point
*/

#include <company_ref_dmo/company_ref_dmo_file.h>
#include <company_ref_main_apps/company_ref_app_options.h>
#include <company_ref_utils/company_ref_ini_config_file.h>
#include <company_ref_utils/company_ref_path_parser.h>
#include <Compan_logger/Compan_logger_sink_cout.h>
#include <dmo_tools_utils/Compan_dmo_ini_converter.h>

#include <iostream>
#include <regex>
#include <string.h>

#include <functional>

namespace {
int usage(char const* appname, int ret)
{
    std::cout << "Usage: " << appname << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                Display this message and exit" << std::endl;
    std::cout << "  -s, --show                Print the tree" << std::endl;
    std::cout << "  -i, --ini                 Ini.cfg file path" << std::endl;
    std::cout << "                            Path can be wildcard and comma separated" << std::endl;
    std::cout << "  -d, --dmo                 Path to dmo output" << std::endl;

    std::cout << std::endl;
    return ret;
}

bool iniImport(COMPAN::REF::DmoContainer& dmo, std::string const& iniPath)
{
    COMPAN::REF::IniConfigFile cfg;

    if (!cfg.read(iniPath)) return false;

    COMPAN::REF::DmoIniConverter converter(dmo, cfg);

    converter.doImport();

    return true;
}

} // namespace

using namespace Compan::Edge;

int main(int argc, char* argv[])
{
    CompanLoggerSinkCout sinkCout(false, 0);

    char const* appName = [&argv]() {
        auto p = strrchr(argv[0], '/');
        return (p ? ++p : argv[0]);
    }();

    DmoContainer dmoContainer;

    std::vector<std::string> iniPaths;
    std::string dmoPath;
    bool showTree(false);

    AppOptionsParser appOptionsParser({
            {'s', "show", false, false},
            {'i', "ini", true, true},
            {'d', "dmo", true, true},
            {'h', "help", false, false},
    });

    if (!appOptionsParser.parse(argc, argv)) return usage(appName, 1);

    if (appOptionsParser.has('h')) return usage(appName, 1);

    if (appOptionsParser.has('i')) iniPaths = appOptionsParser.multi('i');
    if (appOptionsParser.has('d')) dmoPath = appOptionsParser.single('d');
    if (appOptionsParser.has('s')) showTree = true;

    if (iniPaths.empty() || dmoPath.empty()) return usage(appName, 1);

    std::cout << dmoPath << std::endl;
    for (auto& iniPath : iniPaths) {

        // if comma separated - we have to parse them apart
        if (iniPath.find(',') != std::string::npos) {

            PathParser::pathSplitString(iniPath, [&dmoContainer](std::string const& subPath) {
                std::cout << "Importing: " << subPath << std::endl;

                if (!iniImport(dmoContainer, subPath)) std::cout << subPath << " not found" << std::endl;
            });

            continue;
        }

        std::cout << "Importing: " << iniPath << std::endl;
        if (!iniImport(dmoContainer, iniPath)) std::cout << iniPath << " not found" << std::endl;
    }

    std::cout << "Exporting: " << dmoPath << std::endl;
    if (!DmoFile::write(dmoPath, dmoContainer)) std::cout << "Error Writing: " << dmoPath << std::endl;

    if (showTree) dmoContainer.print(std::cout);

    return 0;
}
