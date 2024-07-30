/**
  Copyright © 2023 COMPAN REF
  @file dmo_from_ini_main.cpp
  @brief Entry point
*/

#include <company_ref_dmo/company_ref_dmo_file.h>
#include <company_ref_utils/company_ref_path_parser.h>
#include <Compan_logger/Compan_logger_sink_cout.h>

#include <iostream>
#include <regex>
#include <getopt.h>
#include <string.h>

namespace {
struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"show", no_argument, 0, 's'},
        {"files", required_argument, 0, 'f'},
        {"dmo", required_argument, 0, 'd'},
        {0, 0, 0, 0}};

int usage(char const* appname, int ret)
{
    std::cout << "Usage: " << appname << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                Display this message and exit" << std::endl;
    std::cout << "  -s, --show                Print the tree" << std::endl;
    std::cout << "  -f, --files               Path to files for merge" << std::endl;
    std::cout << "                            Comma seperated" << std::endl;
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

    std::string filesPath;
    std::string dmoPath;
    bool showTree(false);

    int c = 0, option_index = 0;
    std::string const argOptions("hsf:d:");
    while ((c = getopt_long(argc, argv, argOptions.c_str(), long_options, &option_index)) != -1) {
        switch (c) {
        case 'h': return usage(appName, 0);
        case 's': showTree = true; break;

        case 'f': {
            filesPath = optarg;
            break;
        }
        case 'd': {
            dmoPath = optarg;
            break;
        }

        default: return usage(appName, 1);
        }
    }

    if (filesPath.empty()) {
        std::cout << "No input files specified" << std::endl;
        return usage(appName, 1);
    }

    if (dmoPath.empty()) {
        std::cout << "No output path specified" << std::endl;
        return usage(appName, 1);
    }

    PathParser::pathSplitString(
            filesPath, [&dmoContainer](std::string const& mergeFile) { DmoFile::read(mergeFile, dmoContainer); });

    if (!DmoFile::write(dmoPath, dmoContainer)) std::cout << "Error Writing: " << dmoPath << std::endl;

    if (showTree) dmoContainer.print(std::cout);

    return 0;
}
