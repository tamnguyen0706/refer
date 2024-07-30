/**
 Copyright © 2023 COMPAN REF
 @file company_ref_dmo_container.h
 @brief DataModel Metadata container
 */
#ifndef __company_ref_DMO_company_ref_DMO_CONTAINER_H__
#define __company_ref_DMO_company_ref_DMO_CONTAINER_H__

#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h>

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <boost/property_tree/ptree.hpp>

namespace Compan{
namespace Edge {

/*
 * @brief
 */
class DmoContainer {
public:
    typedef boost::property_tree::basic_ptree<std::string, CompanEdgeProtocol::Value> TreeType;
    typedef boost::optional<DmoContainer::TreeType const&> OptionalTreeType;
    typedef std::function<void(CompanEdgeProtocol::Value const&)> VisitFunction;

    static std::string const MetaContainerDelim;
    static ValueId const MetaContainerDelimId;

public:
    DmoContainer();
    virtual ~DmoContainer();

    /*!
     * Inserts a concrete value
     *
     * Inserts a value into the tree, qualifying each name element
     * on insertion. Every element requires value Value_Type
     *
     * Does not allow insertion of values that are meta-data definitions, unless
     * metaCheck is false
     *
     * @param value
     * @param metaCheck - False simply inserts the value into the container
     * @return True if success
     */
    bool insertValue(CompanEdgeProtocol::Value const& value, bool const metaCheck = true);

    /*!
     * Removes non-MetaData values fron the container
     */
    void pruneValues();

    /*!
     * Inserts a container type meta element, and creates the leaf '+' struct value for type definitions.
     *
     * @param valuePath Path to place the Container
     * @param containerType Container type to insert
     * @return True on success
     */
    bool insertMetaContainer(ValueId const& valuePath, CompanEdgeProtocol::Value_Type const containerType);

    /*!
     * Inserts a container type meta element, and creates the leaf '+' with a single value meta type definition
     *
     * @param valuePath Path to place the Container
     * @param containerType Container type to insert
     * @param metaType Value to use
     * @return True on success
     */
    bool insertMetaContainer(
            ValueId const& valuePath,
            CompanEdgeProtocol::Value_Type const containerType,
            CompanEdgeProtocol::Value_Type const metaType);

    /*!
     * Inserts a container type meta element, and creates the leaf '+' with a single value meta type definition
     *
     * @param valuePath Path to place the Container
     * @param containerType Container type to insert
     * @param metaValue Value to use
     * @return True on success
     */
    bool insertMetaContainer(
            ValueId const& valuePath,
            CompanEdgeProtocol::Value_Type const containerType,
            CompanEdgeProtocol::Value const metaValue);

    /*!
     * Inserts a container type meta element, and creates the leaf '+' with a single value meta type definition
     * containing an EnumValue
     *
     * @param valuePath Path to place the Container
     * @param containerType Container type to insert
     * @param metaEnumValue
     * @return True on success
     */
    bool insertMetaContainer(
            ValueId const& valuePath,
            CompanEdgeProtocol::Value_Type const containerType,
            CompanValueTypes::EnumValue const metaEnumValue);

    /*!
     * Inserts a Meta Data value at a parent path location
     *
     * @param parent    Location to insert
     * @param vsValue   Meta Data value to insert
     * @return True on success
     */
    bool insertMetaData(ValueId const& parent, CompanEdgeProtocol::Value const& vsValue);

    /*!
     * Removes a value from the container
     *
     * @param valuePath Location to remove
     * @return  True on success
     */
    bool removeValue(ValueId const& valuePath);

    /*!
     * Sets a value type, over riding the initial type
     *
     * @param valuePath Location to set
     * @return  True on success
     */
    bool setMetaDataType(ValueId const& valuePath, CompanEdgeProtocol::Value_Type const metaType);

    /*!
     * Gets a value type
     *
     * @param valuePath Location to set
     * @return  Valid type or CompanEdgeProtocol::Unset if not found
     */
    CompanEdgeProtocol::Value_Type getMetaDataType(ValueId const& valuePath);

    /*!
     * Retrieves a snippet of the tree that container the meta data for a path.
     *
     * If the path is an embedded map value, passed as:
     *  a.b.c.key.x.y
     *
     * It will internally translate to:
     *  a.b.c.+.x.y
     *
     * @param valuePath Path to retrieve the meta data
     * @return  Valie sub tree - or a boost::null
     */
    OptionalTreeType getMetaData(ValueId const& valuePath) const;

    /*!
     * Retrieves the Value Information from a meta data path
     * @param valueId
     * @return Value
     */
    CompanEdgeProtocol::Value getValueDefinition(ValueId const& valueId) const;

    /*!
     * Function to export the ValueContainer's tree elements with their Value.id()
     * correctly set
     *
     * @param cb    Receiver callback
     */
    void visitValues(VisitFunction cb) const;

    /*!
     * Function to export the ValueContainer's tree elements with their Value.id()
     * correctly set
     *
     * @param cb    Receiver callback
     */
    void visitMetaData(VisitFunction cb) const;

    /*!
     * Checks if a value path exists
     *
     * @param valuePath Path to search for
     * @return  True if found
     */
    bool has(ValueId const& valuePath) const;

    /*!
     * Checks if the value id is an instance value in a container
     * @return True if instance
     */
    bool isInstance(ValueId const& valuePath) const;

    /*!
     * Checks if the value id is an meta data value in a container
     * @return True if instance
     */
    bool isMetaData(ValueId const& valuePath) const;

    /*!
     * Retrieves the MetaData path of a key
     *
     * If the path is an embedded map value, passed as:
     *  a.b.c.key.x.y
     *
     * It will internally translate to the following MetaData key:
     *  a.b.c.+.x.y
     *
     * @param valuePath Path to retrieve the meta data
     * @return  ValueId containing the full MetaData key
     */
    ValueId getMetaDataPath(ValueId const& valuePath) const;

    /// Returns the root TreeType object contained in this DmoContainer
    TreeType const& root() const;

    /// Returns true if empty
    bool empty() const;

    /// Sort the container keys
    void sort();

    void print(std::ostream& os);

protected:
    /*!
     * Inserts or replaces the value.
     *
     * If the value is a Struct or Container, inserts the correct UnknownValue strings
     *
     * @param parent    Parent path to insert
     * @param value     Value to insert
     * @return True on success
     */
    bool emplaceNormalized(ValueId const& parent, CompanEdgeProtocol::Value const& value);

    /*!
     * Inserts a value into the container
     * @param value Value to add
     * @param containerCheck Whether to check if we're adding a container key
     * @return True on success
     */
    bool insertElement(CompanEdgeProtocol::Value const& value, bool const containerCheck);

    // Recursive branch visitor for values
    void visitValues(ValueId const& parentId, TreeType const& branch, VisitFunction cb) const;

    // Recursive branch visitor for meta data containers
    void visitMetaData(ValueId const& parentId, TreeType const& branch, VisitFunction cb, bool const inMetaData = false)
            const;

    // Recursively sorts branches
    void sort(TreeType& branch);

    // Recursive stream out with indentation
    void print(std::ostream& os, TreeType& pt, int level);

private:
    TreeType dmoTree_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_DMO_company_ref_DMO_CONTAINER_H__
