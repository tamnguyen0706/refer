/**
 Copyright © 2023 COMPAN REF
 @file company_ref_app_options.h
 @brief Options parser for applications
 */
#ifndef __company_ref_APP_OPTIONS_H__
#define __company_ref_APP_OPTIONS_H__

#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index_container.hpp>

#include <map>
#include <string>
#include <vector>

namespace Compan{
namespace Edge {

/*!
 *
 * Options parser for applications
 *
 * Replacement for get_opt and get_opt_long
 */
class AppOptionsParser {
public:
    /// Option definition
    struct Option {
        char argNameShort;
        std::string argNameLong;
        bool expectParams;
        bool required;
    };
    using Options = std::vector<Option>;

    AppOptionsParser();
    AppOptionsParser(Options const& options);
    virtual ~AppOptionsParser() = default;

    /// Add Options to the parser
    void add(Options const& options);

    /// Add a single Option to the parser
    void add(Option const& option);

    /// Get an Option by it's short name
    Option get(char const& argName);

    /// Get an Option by it's long name
    Option get(std::string const& argName);

    /// Parses the argc/argv from a main
    /// Returns false on failure
    bool parse(int const argc, char const* argv[]);
    bool parse(int const argc, char* argv[]);

    /// Checks if a argument was parsed
    bool has(char const& argName);

    /// Checks if a argument was parsed
    bool has(std::string const argName);

    /// Returns a single argument's parsed string
    std::string single(char const& argName);
    /// Returns a single argument's parsed string
    std::string single(std::string const& argName);

    /// Returns a single argument's parsed vector of strings
    std::vector<std::string> multi(char const& argName);
    /// Returns a single argument's parsed vector of strings
    std::vector<std::string> multi(std::string const argName);

protected:
    AppOptionsParser(AppOptionsParser const&) = delete;
    AppOptionsParser& operator=(AppOptionsParser const&) = delete;

    /// Adds to the arglist, returns false if options requirements weren't met
    bool addToArgList(char&& shortName, std::vector<std::string>&& params);

private:
    struct NameShortTag {};
    struct NameLongTag {};

    typedef boost::multi_index::multi_index_container<
            Option,
            boost::multi_index::indexed_by<
                    boost::multi_index::ordered_unique<
                            boost::multi_index::tag<NameShortTag>,
                            boost::multi_index::member<Option, char, &Option::argNameShort>>,
                    boost::multi_index::ordered_unique<
                            boost::multi_index::tag<NameLongTag>,
                            boost::multi_index::member<Option, std::string, &Option::argNameLong>>>>
            OptionsListType;

    OptionsListType optionsList_;
    std::map<char, std::vector<std::string>> argList_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_APP_OPTIONS_H__
