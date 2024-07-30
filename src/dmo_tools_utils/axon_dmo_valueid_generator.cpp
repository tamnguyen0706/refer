/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_valueid_generator.cpp
 @brief ValueId code generator - base class
 */
#include "Compan_dmo_valueid_generator.h"

#include <company_ref_utils/company_ref_regex_utils.h>
#include <Compan_logger/Compan_logger.h>

#include <regex>

using namespace Compan::Edge;

CompanLogger ValueIdGeneratorLog("generator");

std::string const ValueIdGenerator::Indent(TabSpace, ' ');
std::string const ValueIdGenerator::ContainerValueTypeName("keyValue");

ValueIdGenerator::ValueIdGenerator(DmoContainer& dmo, std::string const postfix)
    : dmo_(dmo)
    , postfix_(postfix)
    , generateEnumTypes_(true)
    , keywords_(
              {{"", "Empty_"},
               {"*", "Wildcard"},
               {"+", "plus"},
               {"bool", "bool_"},
               {"enum", "enum_"},
               {"operator", "operator_"}})
{
}

bool ValueIdGenerator::generate(std::ostream& strm)
{
    if (!doPreGeneration()) return false;

    generateHead(strm);
    generateBody(strm);
    generateFoot(strm);

    return true;
}

bool ValueIdGenerator::doPreGeneration()
{
    if (dmo_.empty()) return false;

    // scan the file for some information
    ValueId parentId;
    if (fileClass_.empty()) fileClass_ = getOuterClassName(parentId, dmo_.root());

    if (fileClass_.empty()) {
        ErrorLog(ValueIdGeneratorLog) << "Failed to get outer class name" << std::endl;
        return false;
    }

    makeClassName();
    makeFileName();

    if (baseId_.empty()) baseId_ = ValueId(fileClass_);

    bool haveUnknown(false);

    if (haveUnknown) {
        ErrorLog(ValueIdGeneratorLog) << "Unknown data types" << std::endl;
        return false;
    }

    // lastly find all the data types
    findValueTypes(dmo_.root());

    return true;
}

void ValueIdGenerator::generateIncludes(std::ostream& strm)
{
    if (includePaths_.empty()) return;

    for (auto& path : includePaths_) strm << "#include <" << path.generic_string() << ">" << std::endl;

    strm << std::endl;
}

void ValueIdGenerator::makeClassName()
{
    if (className_.empty()) {
        for (auto& valueId : fileClass_) className_ += camelCase(makeNormalizedName(valueId.name()));
    }

    className_ += postfix_;
}

void ValueIdGenerator::makeFileName()
{
    // if the filename isn't set, use the class name - important for #ifdef
    std::string name(std::move(fileNameBase_));
    if (name.empty())
        name = className_;
    else
        name += postfix_;

    bool first = true;
    for (auto& c : name) {

        if (std::isalpha(c) && c == std::toupper(c) && !first) fileNameBase_ += '_';

        fileNameBase_ += std::tolower(c);

        first = false;
    }
}

void ValueIdGenerator::findValueTypes(DmoContainer::TreeType const branch)
{
    for (auto& pos : branch) {

        CompanEdgeProtocol::Value value = pos.second.data();

        if (value.type() != CompanEdgeProtocol::Struct) {
            if (value.type() == CompanEdgeProtocol::Unset || value.type() == CompanEdgeProtocol::Unknown) {
                ErrorLog(ValueIdGeneratorLog) << "Unknown data type" << std::endl;
                return;
            }

            valueTypes_.insert(value.type());
        }

        if (value.type() != CompanEdgeProtocol::Struct && value.type() != CompanEdgeProtocol::Container) continue;

        findValueTypes(pos.second);
    }
}

std::string ValueIdGenerator::keywordReplace(std::string const& arg)
{
    if (keywords_.count(arg)) return keywords_[arg];

    return arg;
}

//
// * static functions
//

ValueId ValueIdGenerator::getOuterClassName(ValueId const& parentId, DmoContainer::TreeType const branch)
{
    //
    // do not change case or normalize this information
    //

    if (branch.data().type() == CompanEdgeProtocol::Container) return parentId;

    ValueId outerName(parentId, branch.data().id());

    if (std::distance(branch.begin(), branch.end()) == 1) {
        if (branch.begin()->second.data().type() != CompanEdgeProtocol::Struct) {

            if (!parentId.empty()) return parentId;

            if (branch.begin()->second.data().type() == CompanEdgeProtocol::Container && outerName.empty())
                return ValueId(branch.begin()->second.data().id());

            return ValueId(branch.data().id());
        }

        return getOuterClassName(outerName, branch.begin()->second);
    }

    return outerName;
}

std::string ValueIdGenerator::camelCase(std::string const& arg)
{
    std::string str = arg;
    str[0] = std::toupper(str[0]);

    size_t pos = 0;
    while ((pos = arg.find('_', pos)) != std::string::npos) {
        ++pos;
        if (pos > str.length()) break;

        str[pos] = std::toupper(str[pos]);
    }

    return std::regex_replace(str, RegexUtils::TrimPattern, "");
}

std::string ValueIdGenerator::makeValueTypeClassName(CompanEdgeProtocol::Value_Type const arg)
{
    if (arg == CompanEdgeProtocol::Container) return "VariantMapValue";

    return "Variant" + CompanEdgeProtocol::Value_Type_Name(arg) + "Value";
}

std::string ValueIdGenerator::convertParent(ValueId const& parentId)
{
    std::ostringstream classStr;

    for (auto& valueId : parentId) classStr << valueId << "::";

    return classStr.str();
}

std::string ValueIdGenerator::getMemberType(CompanEdgeProtocol::Value const& value)
{
    if (value.type() == CompanEdgeProtocol::Struct) return camelCase(getMemberName(value));

    return makeValueTypeClassName(value.type());
}

std::string ValueIdGenerator::getMemberName(CompanEdgeProtocol::Value const& value)
{
    std::string str = makeNormalizedName(value.id());
    str[0] = std::tolower(str[0]);

    return str;
}

std::string ValueIdGenerator::getClassPtrType(CompanEdgeProtocol::Value const& value)
{
    std::ostringstream classPtrStrm;

    classPtrStrm << getMemberType(value) << (value.type() == CompanEdgeProtocol::Struct ? "::" : "") << "Ptr";

    return classPtrStrm.str();
}

std::string ValueIdGenerator::makeNormalizedName(std::string const& arg)
{
    std::string normalized(arg);
    std::regex dashSpace("-| |[.]|[+]");

    if (!std::isalpha(normalized[0])) normalized.insert(normalized.begin(), '_');

    return std::regex_replace(std::regex_replace(normalized, RegexUtils::TrimPattern, ""), dashSpace, "_");
}

std::string ValueIdGenerator::createValueIdDecl(std::string const& first, std::string const& second)
{
    std::ostringstream strm;

    strm << "ValueId(" << first;
    if (!second.empty()) strm << ", " << second;
    strm << ")";

    return strm.str();
}

std::string ValueIdGenerator::quoteString(std::string const& arg)
{
    std::ostringstream strm;
    strm << '"' << arg << '"';
    return strm.str();
}
