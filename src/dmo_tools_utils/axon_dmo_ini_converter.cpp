/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_ini_converter.cpp
 @brief Creates a DMO container from an IniConfigFile
 */
#include "Compan_dmo_ini_converter.h"

#include <company_ref_dmo/company_ref_dmo_helper.h>

#include <company_ref_protocol_utils/company_ref_pb_enum.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_stream.h>

#include <Compan_logger/Compan_logger.h>

#include <iostream>
#include <regex>
#include <sstream>

using namespace Compan::Edge;

CompanLogger DmoIniConverterLog("DmoIniConverter", LogLevel::Error);

std::string const DmoIniConverter::ConfigDmocSection("ConfigDmoc");
std::string const DmoIniConverter::EnumSection("Enums");
std::string const DmoIniConverter::DataModelSection("DataModel");
std::string const DmoIniConverter::ValuesSection("Values");

std::regex const DmoIniConverter::trimPattern_("^\\s+|\\s+$");

/*
 * [ConfigDmoc] # Optional section
 * BaseId=system.something          # base id for dmoc
 * ClassName=SystemSomethingElse    # base class name for dmoc value_ids
 * MicroServiceName=Something       # microservice name for dmoc
 *
 * [Enums]
 * enumName=(index,string),(index,string),(index,string)
 *
 * [DataModel]
 * valueid [TypeName]=default value
 *
 * valueId [Enum enumName]=default value
 * valueid [Text]=string value
 * valueid [Container Text] # Creates a container with Text as the value type
 * valueid [Container Enum enumName] # Creates a container with Enum as the value type
 *
 * [Values]
 * Uses the DataModel to create the value
 * valueId (PersisterName,PersisterName)=value
 *
 * Creates value by they type definition, outside of the DMO container, following the
 * same syntax rules as DMO Definition
 * valueid [TypeName]=default value
 *
 */

DmoIniConverter::DmoIniConverter(DmoContainer& dmo, IniConfigFile& iniCfg)
    : dmo_(dmo)
    , cfg_(iniCfg)
    , ctx_()
    , strand_(ctx_)
    , ws_(ctx_)
    , enumDefinitions_()
{
    FunctionLog(DmoIniConverterLog);
}

void DmoIniConverter::doImport()
{
    FunctionLog(DmoIniConverterLog);

    if (cfg_.has(ConfigDmocSection)) createConfigInformation();

    if (cfg_.has(EnumSection)) createEnumDefinitions();

    if (cfg_.has(DataModelSection)) createDataModelObjects();

    if (cfg_.has(ValuesSection)) createValues();

    ws_.visitValues([this](VariantValue::Ptr const& valuePtr) {
        if (!dmo_.insertValue(valuePtr->get(), false))
            DebugLog(DmoIniConverterLog) << "Warning: failed to insert: " << valuePtr->get() << std::endl;
    });
}

void DmoIniConverter::createConfigInformation()
{
    FunctionLog(DmoIniConverterLog);

    baseId_ = cfg_.getValue(ConfigDmocSection, "BaseId");
    className_ = cfg_.getValue(ConfigDmocSection, "ClassName");
    microserviceName_ = cfg_.getValue(ConfigDmocSection, "MicroServiceName");
}

void DmoIniConverter::createEnumDefinitions()
{
    FunctionLog(DmoIniConverterLog);

    for (auto& enumDefs : cfg_.get(EnumSection)) {
        CompanValueTypes::EnumValue enumValue;

        std::regex regexEnumerators("(\\W+,\\W+)");

        std::sregex_token_iterator begin(enumDefs.second.begin(), enumDefs.second.end(), regexEnumerators, -1);
        std::sregex_token_iterator end;

        for (; begin != end; ++begin) {
            std::string enumStr = *begin;

            size_t posComma = enumStr.find(",");
            if (posComma == std::string::npos) continue;

            // because I can't figure out the regex statement to NOT include the opening
            //  paren
            size_t posBegin = 0;
            if (!isalnum(enumStr[0])) ++posBegin;

            // because I can't figure out the regex statement to NOT include the opening
            //  paren
            size_t posEnd = enumStr.length();
            if (!isalnum(enumStr[posEnd - 1])) { --posEnd; }

            CompanValueTypes::EnumValue::Enumerator enumerator;
            int v = std::stoi(enumStr.substr(posBegin, posComma - posBegin));

            enumerator.set_value(v);
            enumerator.set_text(
                    std::regex_replace(enumStr.substr(posComma + 1, posEnd - (posComma + 1)), trimPattern_, ""));

            *enumValue.add_enumerators() = std::move(enumerator);
        }

        enumDefinitions_.emplace(enumDefs.first, enumValue);
    }
}

DmoIniConverter::DmoDataTypeDef DmoIniConverter::parseIdType(std::string const& arg)
{
    FunctionLog(DmoIniConverterLog);

    DmoDataTypeDef dmoTypeDef;

    // The string possibility is as such
    // valueId [ValueType1 (enumName) ValueType2 (enumName)]
    // myValue                              - Case 1
    // myValue [Container]                  - Case 2
    // myValue [Enum enumName]              - Case 3
    // myValue [Container Enum enumName]    - Case 4

    size_t posBracketBegin = arg.find('[');

    std::get<dmoId>(dmoTypeDef) = std::regex_replace(arg.substr(0, posBracketBegin), trimPattern_, "");

    // No bracket - Case 1
    if (posBracketBegin == std::string::npos) return dmoTypeDef;

    // Now - parse what's inside the brackets
    size_t posBracketEnd = arg.find(']');
    if (posBracketEnd == std::string::npos) posBracketEnd = arg.length();

    ++posBracketBegin;

    std::string bracketStr = arg.substr(posBracketBegin, posBracketEnd - posBracketBegin);

    std::regex typeDefRegex("(\\W+)");
    std::sregex_token_iterator begin(bracketStr.begin(), bracketStr.end(), typeDefRegex, -1);
    std::sregex_token_iterator end;

    // Empty brackets - while not an error,
    if (!std::distance(begin, end)) {
        ErrorLog(DmoIniConverterLog) << "Empty ValueType definitions " << arg << std::endl;

        return dmoTypeDef;
    }

    CompanEdgeProtocol::Value_Type metaType(CompanEdgeProtocol::Unset);

    // Start identifying the data defs
    if (CompanEdgeProtocol::Value_Type_Parse(std::string(*begin), &metaType))
        std::get<dmoType1>(dmoTypeDef) = metaType;
    else {
        ErrorLog(DmoIniConverterLog) << "Missing Value Type: " << arg << std::endl;
        return dmoTypeDef;
    }

    ++begin;
    if (!std::distance(begin, end)) // Case 2
        return dmoTypeDef;

    if (metaType == CompanEdgeProtocol::Enum)
        std::get<dmoEnumType>(dmoTypeDef) = *begin; // Case 3
    else if (CompanEdgeProtocol::Value_Type_Parse(std::string(*begin), &metaType))
        std::get<dmoType2>(dmoTypeDef) = metaType; // Case 4
    else {
        ErrorLog(DmoIniConverterLog) << "Missing Value Type: " << arg << std::endl;
        return dmoTypeDef;
    }

    ++begin;
    if (!std::distance(begin, end)) return dmoTypeDef;

    if (metaType == CompanEdgeProtocol::Enum) std::get<dmoEnumType>(dmoTypeDef) = *begin;

    return dmoTypeDef;
}

void DmoIniConverter::createDataModelObjects()
{
    FunctionLog(DmoIniConverterLog);

    for (auto& metaData : cfg_.get(DataModelSection)) {
        DmoDataTypeDef dmoTypeDef = parseIdType(metaData.first);
        std::string valueStr = std::regex_replace(metaData.second, trimPattern_, "");

        if (std::get<dmoId>(dmoTypeDef).empty() || std::get<dmoType1>(dmoTypeDef) == CompanEdgeProtocol::Unset) {
            ErrorLog(DmoIniConverterLog) << "Missing DataType: " << metaData.first << std::endl;
            continue;
        }

        ValueId metaId(std::get<dmoId>(dmoTypeDef));
        CompanEdgeProtocol::Value_Type metaType(std::get<dmoType1>(dmoTypeDef));

        CompanEdgeProtocol::Value metaValue;

        // If this is a ValueType ValueType definition
        if (std::get<dmoType2>(dmoTypeDef) != CompanEdgeProtocol::Unset) {
            if (metaType != CompanEdgeProtocol::Container) {
                ErrorLog(DmoIniConverterLog) << "Missing Value Type for container: " << metaData.first << std::endl;
                continue;
            }

            // Create the base value
            metaValue.set_id(metaId);
            valueInit(metaValue, std::get<dmoType2>(dmoTypeDef));

            // If we have a enum definition attached
            if (std::get<dmoType2>(dmoTypeDef) == CompanEdgeProtocol::Enum) {

                std::string enumName = std::get<dmoEnumType>(dmoTypeDef);

                // does the enum definition exist?
                if (!enumDefinitions_.count(enumName)) {
                    ErrorLog(DmoIniConverterLog)
                            << "Missing Enum definition: " << metaData.first << " -> " << metaData.second << std::endl;
                    continue;
                }

                // Set everything for this entry
                *metaValue.mutable_enumvalue() = enumDefinitions_[enumName];
                if (!metaData.second.empty())
                    metaValue.mutable_enumvalue()->set_value(ValueEnumerators::fromString(metaValue, metaData.second));
            }

            // Insert the meta definition
            dmo_.insertMetaContainer(metaId, metaType, metaValue);

            continue;
        }

        // If we're container, do our magic
        if (metaType == CompanEdgeProtocol::Container) dmo_.insertMetaContainer(metaId, metaType);
        // If we have a enum definition attached
        else if (metaType == CompanEdgeProtocol::Enum) {

            std::string enumName = std::get<dmoEnumType>(dmoTypeDef);

            // does the enum definition exist?
            if (!enumDefinitions_.count(enumName)) {
                ErrorLog(DmoIniConverterLog)
                        << "Missing Enum definition: " << metaData.first << " -> " << metaData.second << std::endl;
                continue;
            }

            // Create the base value
            metaValue.set_id(metaId);
            valueInit(metaValue, metaType);

            *metaValue.mutable_enumvalue() = enumDefinitions_[enumName];
            if (!metaData.second.empty())
                metaValue.mutable_enumvalue()->set_value(ValueEnumerators::fromString(metaValue, metaData.second));

            // Insert the meta definition
            dmo_.insertMetaData(metaId, metaValue);

        } else {

            // regular old fashioned value - nothing fancy
            VariantValue::Ptr valuePtr(std::make_shared<VariantValue>(strand_, metaId, metaType));
            if (!valueStr.empty()) valuePtr->set(valueStr);

            dmo_.insertMetaData(metaId, valuePtr->get());
        }
    }
}

void DmoIniConverter::createValues()
{
    FunctionLog(DmoIniConverterLog);

    DmoValueStoreHelper helper(dmo_, ws_);

    for (auto& valueDef : cfg_.get(ValuesSection)) {

        DmoDataTypeDef dmoTypeDef = parseIdType(valueDef.first);
        std::string valueStr = std::regex_replace(valueDef.second, trimPattern_, "");

        if (std::get<dmoId>(dmoTypeDef).empty()) {
            ErrorLog(DmoIniConverterLog) << "Missing DataType: " << valueDef.first << std::endl;
            continue;
        }

        ValueId valueId(std::get<dmoId>(dmoTypeDef));

        // it's been created already via createContainerFromDmo
        if (ws_.has(valueId)) {
            ws_.get(valueId)->set(valueDef.second);
            continue;
        }

        CompanEdgeProtocol::Value_Type valueType(std::get<dmoType1>(dmoTypeDef));

        // If we are simply creating values, without a DMO requirement
        if (valueType != CompanEdgeProtocol::Unset) {

            CompanEdgeProtocol::Value value;

            value.set_id(valueId);
            valueInit(value, valueType);

            if (valueType == CompanEdgeProtocol::Enum) {

                std::string enumName = std::get<dmoEnumType>(dmoTypeDef);

                // does the enum definition exist?
                if (!enumDefinitions_.count(enumName)) {
                    ErrorLog(DmoIniConverterLog)
                            << "Missing Enum definition: " << valueId << " -> " << enumName << std::endl;
                    continue;
                }

                *value.mutable_enumvalue() = enumDefinitions_[enumName];
            }

            VariantValue::Ptr valuePtr(std::make_shared<VariantValue>(strand_, value));

            if (!valueStr.empty()) valuePtr->set(valueStr);

            dmo_.insertValue(valuePtr->get());
            continue;
        }

        // Insert the element into the valuestore according to the rules
        CompanEdgeProtocol::Value value = dmo_.getValueDefinition(valueId);
        value.set_id(valueId);

        if (helper.insertValue(value)) ws_.get(valueId)->set(valueStr);
    }
}
