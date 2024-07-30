/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_ini_converter.h
 @brief Creates a DMO container from an IniConfigFile
 */
#ifndef __Compan_DMO_INI_CONVERTER_H__
#define __Compan_DMO_INI_CONVERTER_H__

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_utils/company_ref_ini_config_file.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

namespace Compan{
namespace Edge {

/*!
 * Converts a dmo ini configuration file into a DMO container
 */
class DmoIniConverter {
public:
    DmoIniConverter(DmoContainer& dmo, IniConfigFile& iniCfg);

    virtual ~DmoIniConverter() = default;

    /// Parses the IniConfigFile and creates the entries in the DMO container
    void doImport();

    // post processing values that can be consumed by dmoc
    ValueId baseId() const;
    std::string className() const;
    std::string microserviceName() const;

protected:
    enum { dmoId, dmoType1, dmoType2, dmoEnumType };

    using DmoDataTypeDef =
            std::tuple<std::string, CompanEdgeProtocol::Value_Type, CompanEdgeProtocol::Value_Type, std::string>;

    void createConfigInformation();
    void createEnumDefinitions();
    void createDataModelObjects();
    void createValues();

    DmoDataTypeDef parseIdType(std::string const& arg);

private:
    DmoContainer& dmo_;
    IniConfigFile& cfg_;
    boost::asio::io_context ctx_;
    boost::asio::io_context::strand strand_;
    VariantValueStore ws_;

    ValueId baseId_;
    std::string className_;
    std::string microserviceName_;

    std::map<std::string, CompanValueTypes::EnumValue> enumDefinitions_;

    static std::regex const trimPattern_;

    static std::string const ConfigDmocSection;
    static std::string const EnumSection;
    static std::string const DataModelSection;
    static std::string const ValuesSection;
};

inline ValueId DmoIniConverter::baseId() const
{
    return baseId_;
}

inline std::string DmoIniConverter::className() const
{
    return className_;
}

inline std::string DmoIniConverter::microserviceName() const
{
    return microserviceName_;
}

} // namespace Edge
} // namespace Compan

#endif // __Compan_DMO_INI_CONVERTER_H__
