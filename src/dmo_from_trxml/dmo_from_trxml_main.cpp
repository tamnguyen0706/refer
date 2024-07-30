/**
  Copyright © 2023 COMPAN REF
  @file dmo_from_trxml_main.cpp
  @brief Entry point for BroadBand Forum's TR XML format
*/

#include <company_ref_dmo/company_ref_dmo_file.h>
#include <Compan_logger/Compan_logger_sink_cout.h>

#include <iostream>
#include <string.h>

#include <dmo_tools_utils/Compan_dmo_from_trxml_dmdocument.h>
#include <getopt.h>

namespace {
struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"show", no_argument, 0, 's'},
        {"xml", required_argument, 0, 'x'},
        {"dmo", required_argument, 0, 'd'},
        {0, 0, 0, 0}};

int usage(char const* appname, int ret)
{
    std::cout << "Usage: " << appname << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                Display this message and exit" << std::endl;
    std::cout << "  -s, --show                Print the tree" << std::endl;
    std::cout << "  -x, --xml                 Xml file path" << std::endl;
    std::cout << "  -d, --dmo                 Path to dmo output" << std::endl;

    std::cout << std::endl;
    return ret;
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
    DmDocument dmDocument(dmoContainer);

    std::string xmlPath;
    std::string dmoPath;
    bool showTree(false);

    int c = 0, option_index = 0;
    std::string const argOptions("hsx:d:");
    while ((c = getopt_long(argc, argv, argOptions.c_str(), long_options, &option_index)) != -1) {
        switch (c) {
        case 'h': return usage(appName, 0);
        case 's': showTree = true; break;

        case 'x': {
            xmlPath = optarg;
            break;
        }
        case 'd': {
            dmoPath = optarg;
            break;
        }

        default: return usage(appName, 1);
        }
    }

    if (xmlPath.empty() || dmoPath.empty()) return usage(appName, 1);

    if (!dmDocument.parse(xmlPath)) return usage(appName, 1);

    if (!DmoFile::write(dmoPath, dmoContainer)) std::cout << "Error Writing: " << dmoPath << std::endl;

    if (showTree) dmoContainer.print(std::cout);

    return 0;
}
