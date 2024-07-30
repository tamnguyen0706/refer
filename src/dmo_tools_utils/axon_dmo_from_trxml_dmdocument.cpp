/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_from_trxml_dmdocument.cpp
 @brief XML Parser for dm:document root
 */
#include "Compan_dmo_from_trxml_dmdocument.h"

#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_utils/company_ref_regex_utils.h>
#include <Compan_logger/Compan_logger.h>

#include <boost/property_tree/xml_parser.hpp>
#include <fstream>
#include <regex>

namespace Compan{
namespace Edge {
CompanLogger DmDocumentLog("dmdocument", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

// I know - it's not in "namespace" - this is developer code only. Will be removed
//  when this is done
void print_tree(std::ostream& os, boost::property_tree::ptree const& pt, int level)
{
    std::string indent(level * 2, ' ');
    for (auto& pos : pt) {

        if (pos.first == std::string("description")) continue;

        boost::optional<boost::property_tree::ptree const&> attributes = pos.second.get_child_optional("<xmlattr>");
        if (attributes) {
            os << indent << pos.first << std::endl;
            for (auto& attrib : attributes.get()) {
                os << indent << "-> " << attrib.first.data() << " = " << attrib.second.data() << std::endl;
            }
        } else
            os << indent << "-> " << pos.first.data() << ": " << pos.second.data() << std::endl;

        print_tree(os, pos.second, ++level);
    }
}

namespace {
std::string getAttribute(boost::property_tree::ptree const& xmlNode, std::string const& attribName)
{
    auto xmlAttribIter = xmlNode.find(DmDocument::TermXmlAttrib);
    if (xmlAttribIter == xmlNode.not_found()) {
        ErrorLog(DmDocumentLog) << "attributes not found: " << DmDocument::TermXmlAttrib << std::endl;
        return std::string();
    }

    auto attribIter = xmlAttribIter->second.find(attribName);
    if (attribIter == xmlAttribIter->second.not_found()) { return std::string(); }

    return attribIter->second.data();
}

bool hasAttribute(boost::property_tree::ptree const& xmlNode, std::string const& attribName)
{
    auto xmlAttribIter = xmlNode.find(DmDocument::TermXmlAttrib);
    if (xmlAttribIter == xmlNode.not_found()) { return false; }

    auto attribIter = xmlAttribIter->second.find(attribName);
    if (attribIter == xmlAttribIter->second.not_found()) return false;

    return true;
}

template <typename S, typename T>
void setIntervalMin(S* interval, T const arg)
{
    interval->set_min(arg);
}

template <typename S, typename T>
void setIntervalMax(S* interval, T const arg)
{
    interval->set_max(arg);
}

template <typename S, typename T>
void setDefaultMinMax(S* interval, T const)
{
    setIntervalMin(interval, std::numeric_limits<T>::min());
    setIntervalMax(interval, std::numeric_limits<T>::max());
}

template <typename S, typename T>
void setRange(boost::property_tree::ptree const& xmlNode, S* interval, T const arg)
{
    setDefaultMinMax(interval, arg);

    auto rangeIter = xmlNode.find(DmDocument::TermRange);
    if (rangeIter != xmlNode.not_found()) {
        std::string minString(getAttribute(rangeIter->second, DmDocument::TermAttribMinInclusive));
        if (!minString.empty()) {
            T minValue(0);
            std::istringstream(minString) >> minValue;
            setIntervalMin(interval, minValue);
        }

        std::string maxString(getAttribute(rangeIter->second, DmDocument::TermAttribMaxInclusive));
        if (!maxString.empty()) {
            T maxValue(0);
            std::istringstream(maxString) >> maxValue;
            setIntervalMax(interval, maxValue);
        }
    }
}
} // namespace

std::string const DmDocument::TermXmlAttrib("<xmlattr>");
std::string const DmDocument::TermAttribName("name");
std::string const DmDocument::TermAttribBase("base");
std::string const DmDocument::TermAttribAccess("access");
std::string const DmDocument::TermAttribValue("value");
std::string const DmDocument::TermAttribStatus("status");
std::string const DmDocument::TermAttribRef("ref");
std::string const DmDocument::TermAttrib_ValueDeprecated("deprecated");

std::string const DmDocument::TermDataType("dataType");
std::string const DmDocument::TermModel("model");
std::string const DmDocument::TermObject("object");
std::string const DmDocument::TermParameter("parameter");
std::string const DmDocument::TermSyntax("syntax");
std::string const DmDocument::TermPathRef("pathRef");

std::string const DmDocument::TermString("string");
std::string const DmDocument::TermSize("size");
std::string const DmDocument::TermEnumeration("enumeration");
std::string const DmDocument::TermEnumerationRef("enumerationRef");
std::string const DmDocument::TermPattern("pattern");

std::string const DmDocument::TermBoolean("boolean");
std::string const DmDocument::TermList("list");
std::string const DmDocument::TermDateTime("dateTime");
std::string const DmDocument::TermHexBinary("hexBinary");
std::string const DmDocument::TermBase64("base64");
std::string const DmDocument::TermDecimal("decimal");

std::string const DmDocument::TermInt("int");
std::string const DmDocument::TermUnsignedInt("unsignedInt");
std::string const DmDocument::TermLong("long");
std::string const DmDocument::TermUnsignedLong("unsignedLong");
std::string const DmDocument::TermRange("range");
std::string const DmDocument::TermAttribMinInclusive("minInclusive");
std::string const DmDocument::TermAttribMaxInclusive("maxInclusive");

std::string const DmDocument::TermDefault("default");

DmDocument::DmDocument(DmoContainer& dmo)
    : dmo_(dmo)
    , knownDataTypeDeclarations_({
              {"IPv6Prefix", CompanEdgeProtocol::IPv6},
              {"IPv4Prefix", CompanEdgeProtocol::IPv6},
              {"IPPrefix", CompanEdgeProtocol::IPv4},
              {"IPv6Address", CompanEdgeProtocol::IPv6},
              {"IPv4Address", CompanEdgeProtocol::IPv4},
              {"IPAddress", CompanEdgeProtocol::IPv4},
              {"IEEE_EUI64", CompanEdgeProtocol::ULLInterval},
              {"UUID", CompanEdgeProtocol::Udid},
              {"MACAddress", CompanEdgeProtocol::EUI48},
              {"ZigBeeNetworkAddress", CompanEdgeProtocol::USInterval},
      })
{

    dataTypeParsers_[TermDataType] =
            std::bind(&DmDocument::parseDataType, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermString] =
            std::bind(&DmDocument::parseDataTypeString, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermBoolean] =
            std::bind(&DmDocument::parseDataTypeBoolean, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermList] =
            std::bind(&DmDocument::parseDataTypeList, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermDateTime] =
            std::bind(&DmDocument::parseDataTypeDateTime, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermInt] =
            std::bind(&DmDocument::parseDataTypeInt, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermUnsignedInt] =
            std::bind(&DmDocument::parseDataTypeUnsignedInt, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermLong] =
            std::bind(&DmDocument::parseDataTypeLong, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermUnsignedLong] =
            std::bind(&DmDocument::parseDataTypeUnsignedLong, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermDecimal] =
            std::bind(&DmDocument::parseDataTypeDecimal, this, std::placeholders::_1, std::placeholders::_2);

    dataTypeParsers_[TermHexBinary] =
            std::bind(&DmDocument::parseDataTypeMiscTextType, this, std::placeholders::_1, std::placeholders::_2);
    dataTypeParsers_[TermBase64] =
            std::bind(&DmDocument::parseDataTypeMiscTextType, this, std::placeholders::_1, std::placeholders::_2);

    dataTypeParsers_[TermDefault] =
            std::bind(&DmDocument::parseDataTypeDefault, this, std::placeholders::_1, std::placeholders::_2);
}

DmDocument::~DmDocument()
{
}

bool DmDocument::parse(std::string const& xmlPath)
{
    std::ifstream xmlStream(xmlPath);

    if (!xmlStream.is_open()) {
        ErrorLog(DmDocumentLog) << "failed to open file: [" << xmlPath << "]" << std::endl;
        return false;
    }

    boost::property_tree::ptree xmlTree;
    boost::property_tree::ptree dmTree;
    boost::property_tree::read_xml(xmlStream, xmlTree);

    try {
        dmTree = xmlTree.get_child("dm:document");
    } catch (boost::property_tree::ptree_bad_path& pbp) {
        ErrorLog(DmDocumentLog) << "Not a TR-XML document: [" << xmlPath << "]" << std::endl;
        return false;
    }

    for (auto& pos : dmTree) {

        if (pos.first == TermDataType) {
            CompanEdgeProtocol::Value dataType;

            if (parseDataType(pos.second, dataType)) dataTypeDeclarations_.insert({dataType.id(), dataType});

        } else if (pos.first == TermModel) {

            for (auto& modelElement : pos.second) {

                if (modelElement.first == TermObject) parseObject(modelElement.second);
            }
        }

        // everything else, we skip
    }

    if (0) {
        std::cout << "DataType Declarations" << std::endl;
        for (auto& decls : dataTypeDeclarations_)
            std::cout << decls.second << "\t ( " << CompanEdgeProtocol::Value_Type_Name(decls.second.type()) << " ) "
                      << std::endl;
    }

    return true;
}

bool DmDocument::parseEnumValue(boost::property_tree::ptree const& xmlNode, CompanValueTypes::EnumValue& enumValue)
{
    // in the event it is an extension definition
    auto enumIter = xmlNode.find(TermEnumeration);

    if (enumIter != xmlNode.not_found()) {

        enumValue.clear_enumerators();
        int i = 0;
        for (auto& enumStr : xmlNode) {

            if (enumStr.first == TermXmlAttrib) continue;

            CompanValueTypes::EnumValue::Enumerator enumerator;
            enumerator.set_value(i++);

            std::string enumText = getAttribute(enumStr.second, TermAttribValue);

            if (enumText.empty())
                enumText = "_EMPTY_";
            else if (enumText == "*")
                enumText = "Wildcard";

            enumerator.set_text(enumText);
            *enumValue.add_enumerators() = std::move(enumerator);
        }

        return true;
    }

    return false;
}

bool DmDocument::parseDataType(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    std::string dataTypeName;
    dataTypeName = getAttribute(xmlNode, TermAttribName);

    if (dataTypeName.empty()) {

        //  This might be a reference data type
        std::string refName;
        refName = getAttribute(xmlNode, TermAttribRef);
        if (refName.empty()) {
            DebugLog(DmDocumentLog) << "no name or ref attributes" << std::endl;
            return false;
        }

        if (knownDataTypeDeclarations_.count(dataTypeName)) {

            valueInit(value, knownDataTypeDeclarations_[dataTypeName]);
            return true;

        } else if (dataTypeDeclarations_.count(refName)) {

            CompanEdgeProtocol::Value refValue = dataTypeDeclarations_[refName];

            valueInit(value, refValue.type());

            if (!parseEnumValue(xmlNode, *value.mutable_enumvalue())) *value.mutable_enumvalue() = refValue.enumvalue();

            return true;
        }

        DebugLog(DmDocumentLog) << "no name or ref attributes" << std::endl;
        return false;
    }

    value.set_id(dataTypeName);
    if (knownDataTypeDeclarations_.count(dataTypeName)) {
        value.set_type(knownDataTypeDeclarations_[dataTypeName]);
        return true;
    }

    std::string baseName(getAttribute(xmlNode, TermAttribBase));
    if (!baseName.empty()) {

        if (!dataTypeDeclarations_.count(baseName)) {
            ErrorLog(DmDocumentLog) << "base name not found: " << baseName << std::endl;
            return false;
        }

        value = dataTypeDeclarations_[baseName];
        value.set_id(dataTypeName);

        return true;
    }

    for (auto& child : xmlNode) {

        if (dataTypeParsers_.count(child.first)) return dataTypeParsers_[child.first](child.second, value);
    }

    return false;
}

bool DmDocument::parseDataTypeString(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    // This is okay, the set contains strings
    if (value.type() == CompanEdgeProtocol::UnorderedSet) return true;

    if (parseEnumValue(xmlNode, *value.mutable_enumvalue())) {
        valueInit(value, CompanEdgeProtocol::Enum);
        return true;
    }

    value.set_type(CompanEdgeProtocol::Text);

    if (xmlNode.find(TermSize) != xmlNode.not_found()) { return true; }

    if (xmlNode.find(TermPathRef) != xmlNode.not_found()) { return true; }

    if (xmlNode.find(TermEnumerationRef) != xmlNode.not_found()) { return true; }

    if (xmlNode.find(TermPattern) != xmlNode.not_found()) { return true; }

    return true;
}

bool DmDocument::parseDataTypeBoolean(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    (void)xmlNode;
    value.set_type(CompanEdgeProtocol::Bool);
    return true;
}

bool DmDocument::parseDataTypeList(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    (void)xmlNode;
    value.set_type(CompanEdgeProtocol::UnorderedSet);
    return true;
}

bool DmDocument::parseDataTypeDateTime(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    (void)xmlNode;
    value.set_type(CompanEdgeProtocol::TimeVal);
    return true;
}

bool DmDocument::parseDataTypeInt(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    value.set_type(CompanEdgeProtocol::Interval);
    setRange(xmlNode, value.mutable_intervalvalue(), int32_t());
    return true;
}

bool DmDocument::parseDataTypeUnsignedInt(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    value.set_type(CompanEdgeProtocol::UInterval);
    setRange(xmlNode, value.mutable_uintervalvalue(), uint32_t());
    return true;
}

bool DmDocument::parseDataTypeLong(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    value.set_type(CompanEdgeProtocol::LLInterval);
    setRange(xmlNode, value.mutable_llintervalvalue(), int64_t());
    return true;
}

bool DmDocument::parseDataTypeUnsignedLong(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    value.set_type(CompanEdgeProtocol::ULLInterval);
    setRange(xmlNode, value.mutable_llintervalvalue(), uint64_t());
    return true;
}

bool DmDocument::parseDataTypeDecimal(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    value.set_type(CompanEdgeProtocol::DInterval);
    setRange(xmlNode, value.mutable_dintervalvalue(), double());
    return true;
}

bool DmDocument::parseDataTypeMiscTextType(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    (void)xmlNode;
    value.set_type(CompanEdgeProtocol::Text);
    return true;
}

bool DmDocument::parseDataTypeDefault(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value)
{
    std::istringstream valueStrm(getAttribute(xmlNode, DmDocument::TermAttribValue));

    switch (value.type()) {
    case CompanEdgeProtocol::Bool: {
        bool arg;
        valueStrm >> std::boolalpha >> arg;
        value.mutable_boolvalue()->set_value(arg);
    } break;

    case CompanEdgeProtocol::Text: value.mutable_textvalue()->set_value(valueStrm.str()); break;

    case CompanEdgeProtocol::IPv4: value.mutable_ipv4value()->set_value(valueStrm.str()); break;

    case CompanEdgeProtocol::IPv6: value.mutable_ipv6value()->set_value(valueStrm.str()); break;

    case CompanEdgeProtocol::Enum: {
        bool found(false);

        for (int i = 0; i < value.enumvalue().enumerators_size(); ++i) {
            if (value.enumvalue().enumerators(i).text() == valueStrm.str()) {
                value.mutable_enumvalue()->set_value(value.enumvalue().enumerators(i).value());
                found = true;
            }
        }

        if (!found) {
            ErrorLog(DmDocumentLog) << "Failed to set default enum value: " << value.id() << " = " << valueStrm.str()
                                    << std::endl;
        }

    } break;

    case CompanEdgeProtocol::Interval: {
        int32_t arg;
        valueStrm >> arg;
        value.mutable_intervalvalue()->set_value(arg);
    } break;
    case CompanEdgeProtocol::UInterval: {
        uint32_t arg;
        valueStrm >> arg;
        value.mutable_uintervalvalue()->set_value(arg);
    } break;
    case CompanEdgeProtocol::LLInterval: {
        int64_t arg;
        valueStrm >> arg;
        value.mutable_llintervalvalue()->set_value(arg);
    } break;
    case CompanEdgeProtocol::ULLInterval: {
        uint64_t arg;
        valueStrm >> arg;
        value.mutable_ullintervalvalue()->set_value(arg);
    } break;

    case CompanEdgeProtocol::UnorderedSet:
    case CompanEdgeProtocol::EUI48:
    case CompanEdgeProtocol::TimeVal:
        // nothing to default
        break;

    default:
        ErrorLog(DmDocumentLog) << __FUNCTION__ << ": unhandled - " << value.id() << " = " << valueStrm.str() << " ("
                                << CompanEdgeProtocol::Value_Type_Name(value.type()) << ") " << std::endl;
        break;
    }
    return true;
}

bool DmDocument::checkDuplicate(ValueId const& valueId)
{
    HashToken hashToken = valudIdBucket_.make(valueId);

    if (tokenSet_.has(hashToken)) return true;

    if (!tokenSet_.insert(hashToken)) ErrorLog(DmDocumentLog) << "Failed to add hash token: " << valueId << std::endl;

    return false;
}

void DmDocument::parseObject(boost::property_tree::ptree const& xmlNode)
{
    std::string baseAttribute = getAttribute(xmlNode, TermAttribBase);
    if (!baseAttribute.empty()) return;

    std::string ojectName = getAttribute(xmlNode, TermAttribName);

    // We want to replace the instance {i} with DMO's concept '+' for containers
    std::regex instanceStr(RegexUtils::escapeString("{i}"));

    ValueId valueId(std::regex_replace(ojectName, instanceStr, "+"));

    if (checkDuplicate(valueId)) {
        ErrorLog(DmDocumentLog) << "Duplicate object: " << valueId << std::endl;
        return;
    }

    if (valueId.leaf() == DmoContainer::MetaContainerDelimId) {

        if (!dmo_.insertMetaContainer(valueId.parent(), CompanEdgeProtocol::Container)) {
            ErrorLog(DmDocumentLog) << "Failed to add meta data container: " << valueId << std::endl;
            return;
        }
    }

    for (auto& element : xmlNode) {
        if (element.first == TermParameter) {
            if (!parseParameter(valueId, element.second)) {

                std::string paramIdentity;
                if (hasAttribute(element.second, TermAttribBase))
                    paramIdentity = "base=" + getAttribute(element.second, TermAttribBase);
                else if (hasAttribute(element.second, TermAttribName))
                    paramIdentity = "name=" + getAttribute(element.second, TermAttribName);
                else
                    ErrorLog(DmDocumentLog) << "unhandled: " << ojectName << " " << paramIdentity << std::endl;
            }
        }
    }
}

bool DmDocument::parseParameter(ValueId const& parent, boost::property_tree::ptree const& xmlNode)
{
    std::string baseAttribute = getAttribute(xmlNode, TermAttribBase);
    if (!baseAttribute.empty()) return false;

    std::string statusAttribute = getAttribute(xmlNode, TermAttribStatus);
    if (!statusAttribute.empty()) {
        if (statusAttribute == TermAttrib_ValueDeprecated) {
            DebugLog(DmDocumentLog) << "Parameter deprecated: " << getAttribute(xmlNode, TermAttribName) << std::endl;
            return false;
        }
    }

    ValueId paramName(parent, getAttribute(xmlNode, TermAttribName));

    if (checkDuplicate(paramName)) {
        ErrorLog(DmDocumentLog) << "Duplicate parameter: " << paramName << std::endl;
        return false;
    }

    CompanEdgeProtocol::Value value;

    auto syntaxIter = xmlNode.find(TermSyntax);
    if (syntaxIter == xmlNode.not_found()) {
        ErrorLog(DmDocumentLog) << "No syntax meta data: " << paramName << std::endl;

        return false;
    }

    value.set_id(paramName);

    for (auto& syntaxChild : syntaxIter->second) {

        if (syntaxChild.first == TermXmlAttrib) {
            // deal with parsing this later
            //  These are the attributes that define readonly, password .. etc
            //  Hints to what to do with the data type
            continue;
        }

        if (!dataTypeParsers_.count(syntaxChild.first)) {
            ErrorLog(DmDocumentLog) << "Unknown data type: " << paramName << " - " << syntaxChild.first << std::endl;
            continue;
        }

        if (value.type() == CompanEdgeProtocol::UnorderedSet) continue;

        if (!dataTypeParsers_[syntaxChild.first](syntaxChild.second, value)) return false;
    }

    if (parent.name().find(DmoContainer::MetaContainerDelim) != std::string::npos) {

        if (!dmo_.insertMetaData(paramName, value)) {
            ErrorLog(DmDocumentLog) << "Failed to insert MetaData: " << paramName << std::endl;
            return false;
        }

        return true;
    }

    if (!dmo_.insertValue(value, true)) {
        ErrorLog(DmDocumentLog) << "Failed to value: " << paramName << std::endl;
        return false;
    }

    return true;
}
