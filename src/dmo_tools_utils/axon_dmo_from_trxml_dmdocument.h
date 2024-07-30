/**
 Copyright © 2023 COMPAN REF
 @file dmo_from_trxml_dmdocument.h
 @brief XML Parser for dm:document root
 */
#ifndef __DMO_FROM_TRXML_DMDOCUMENT_H__
#define __DMO_FROM_TRXML_DMDOCUMENT_H__

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <boost/property_tree/ptree.hpp>

#include <company_ref_variant_valuestore/company_ref_variant_valuestore_hashtoken_set.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_value_id_bucketizer.h>

#include <map>

namespace CompanValueTypes {
class EnumValue;
}

namespace Compan{
namespace Edge {

/*
 * @brief
 */
class DmDocument {
public:
    using DeclaredDataTypes = std::map<std::string, CompanEdgeProtocol::Value>;

    using DataTypeParser = std::function<bool(boost::property_tree::ptree const&, CompanEdgeProtocol::Value&)>;
    DmDocument(DmoContainer& dmo);

    virtual ~DmDocument();

    bool parse(std::string const& xmlPath);

    DeclaredDataTypes getDeclaredDataTypes() const;

protected:
    bool parseEnumValue(boost::property_tree::ptree const& xmlNode, CompanValueTypes::EnumValue& enumValue);

    bool parseDataType(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeString(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeBoolean(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeList(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeDateTime(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeInt(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeUnsignedInt(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeLong(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeUnsignedLong(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);
    bool parseDataTypeDecimal(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);

    // for tr-xml types that just equate to a CompanEdgeProtocol::Text type
    bool parseDataTypeMiscTextType(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);

    bool parseDataTypeDefault(boost::property_tree::ptree const& xmlNode, CompanEdgeProtocol::Value& value);

    void parseObject(boost::property_tree::ptree const& xmlNode);
    bool parseParameter(ValueId const& parent, boost::property_tree::ptree const& xmlNode);

    bool insertParent(ValueId const& parent);

    bool checkDuplicate(ValueId const& valueId);

public:
    static std::string const TermXmlAttrib;

    static std::string const TermDataType;
    static std::string const TermModel;
    static std::string const TermObject;
    static std::string const TermParameter;
    static std::string const TermSyntax;

    static std::string const TermPathRef;
    static std::string const TermAttribRef;

    static std::string const TermString;
    static std::string const TermSize;
    static std::string const TermEnumeration;
    static std::string const TermEnumerationRef;
    static std::string const TermPattern;

    static std::string const TermList;
    static std::string const TermBoolean;
    static std::string const TermDateTime;
    static std::string const TermHexBinary;
    static std::string const TermBase64;
    static std::string const TermDecimal;

    static std::string const TermInt;
    static std::string const TermUnsignedInt;
    static std::string const TermLong;
    static std::string const TermUnsignedLong;
    static std::string const TermRange;
    static std::string const TermAttribMinInclusive;
    static std::string const TermAttribMaxInclusive;

    static std::string const TermDefault;

    static std::string const TermAttribName;
    static std::string const TermAttribBase;
    static std::string const TermAttribAccess;
    static std::string const TermAttribValue;
    static std::string const TermAttribStatus;

    // status="deprecated"
    static std::string const TermAttrib_ValueDeprecated;

private:
    DmoContainer& dmo_;

    DeclaredDataTypes dataTypeDeclarations_;

    std::map<std::string, CompanEdgeProtocol::Value_Type> knownDataTypeDeclarations_;

    std::map<std::string, DataTypeParser> dataTypeParsers_;

    // for identifying duplicates
    ValueIdBucketizer valudIdBucket_;
    HashTokenSet tokenSet_;
};

inline DmDocument::DeclaredDataTypes DmDocument::getDeclaredDataTypes() const
{
    return dataTypeDeclarations_;
}

} // namespace Edge
} // namespace Compan

#endif // __DMO_FROM_TRXML_DMDOCUMENT_H__
