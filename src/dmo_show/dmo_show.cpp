/**
  Copyright © 2023 COMPAN REF
  @file dmo_show.cpp
  @brief Entry point
*/

#include <company_ref_dmo/company_ref_dmo_file.h>
#include <company_ref_main_apps/company_ref_app_options.h>
#include <company_ref_utils/company_ref_path_parser.h>
#include <Compan_logger/Compan_logger_sink_cout.h>

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
    std::cout << "  -d, --dmo                 Path to dmo show" << std::endl;

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

    std::string dmoPath;

    AppOptionsParser appOptionsParser({
            {'d', "dmo", true, true},
            {'h', "help", false, false},
    });

    if (!appOptionsParser.parse(argc, argv)) return usage(appName, 1);

    if (appOptionsParser.has('h')) return usage(appName, 1);

    if (appOptionsParser.has('d')) dmoPath = appOptionsParser.single('d');

    if (!DmoFile::read(dmoPath, dmoContainer)) std::cout << "Error Reading: " << dmoPath << std::endl;

    dmoContainer.print(std::cout);

    return 0;
}
