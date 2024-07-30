/**
 Copyright © 2023 COMPAN REF
 @file company_ref_app_options.cpp
 @brief Options parser for applications
 */
#include "company_ref_app_options.h"

using namespace Compan::Edge;

#include <iostream>

AppOptionsParser::AppOptionsParser()
{
}

AppOptionsParser::AppOptionsParser(Options const& options)
{
    add(options);
}

void AppOptionsParser::add(Options const& options)
{
    for (auto& option : options) add(option);
}

void AppOptionsParser::add(Option const& option)
{
    optionsList_.insert(option);
}

AppOptionsParser::Option AppOptionsParser::get(char const& argName)
{
    auto iter = optionsList_.get<NameShortTag>().find(argName);
    if (iter == optionsList_.get<NameShortTag>().end()) return Option();

    return *iter;
}

AppOptionsParser::Option AppOptionsParser::get(std::string const& argName)
{
    auto iter = optionsList_.get<NameLongTag>().find(argName);
    if (iter == optionsList_.get<NameLongTag>().end()) return Option();

    return *iter;
}

bool AppOptionsParser::addToArgList(char&& shortName, std::vector<std::string>&& params)
{
    if (get(shortName).expectParams && params.empty()) return false;

    argList_.emplace(shortName, params);
    return true;
}

bool AppOptionsParser::parse(int const argc, char const* argv[])
{
    return parse(argc, const_cast<char**>(argv));
}

bool AppOptionsParser::parse(int const argc, char* argv[])
{
    char shortName(0x00);
    std::vector<std::string> params;

    for (int idx = 1; idx < argc; ++idx) {
        std::string const element(argv[idx]);

        if (element[0] == '-') {
            if (shortName) {

                if (!addToArgList(std::move(shortName), std::move(params))) return false;
                params.clear();
            }

            Option option;

            std::string elementParam;

            if (element[1] == '-') {
                // --arg=param
                size_t assignPos = element.find('=');
                if (assignPos != std::string::npos) elementParam = element.substr(assignPos + 1);

                option = get(element.substr(2, assignPos - 2));
            } else
                option = get(element[1]);

            shortName = option.argNameShort;

            if (!shortName) return false;

            if (!elementParam.empty()) params.push_back(elementParam);

            continue;
        }

        params.push_back(element);
    }

    if (!addToArgList(std::move(shortName), std::move(params))) return false;

    // Make sure we aren't missing any requrements
    for (auto iter = optionsList_.get<NameShortTag>().begin(); iter != optionsList_.get<NameShortTag>().end(); ++iter) {

        if (iter->required && argList_.count(iter->argNameShort) == 0) return false;
    }

    return true;
}

bool AppOptionsParser::has(char const& argName)
{
    return (argList_.count(argName) != 0);
}

bool AppOptionsParser::has(std::string const argName)
{
    return has(get(argName).argNameShort);
}

std::string AppOptionsParser::single(char const& argName)
{
    if (!has(argName)) return std::string();

    std::vector<std::string> args(multi(argName));
    if (args.empty()) return std::string();

    return args[0];
}

std::string AppOptionsParser::single(std::string const& argName)
{
    return single(get(argName).argNameShort);
}

std::vector<std::string> AppOptionsParser::multi(char const& argName)
{
    if (!has(argName)) return std::vector<std::string>();

    return argList_[argName];
}

std::vector<std::string> AppOptionsParser::multi(std::string const argName)
{
    return multi(get(argName).argNameShort);
}
