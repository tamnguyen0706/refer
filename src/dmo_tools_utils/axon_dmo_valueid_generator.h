/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_valueid_generator.h
 @brief ValueId code generator - base class
 */
#ifndef __Compan_DMO_VALUEID_GENERATOR_H__
#define __Compan_DMO_VALUEID_GENERATOR_H__

#include <company_ref_dmo/company_ref_dmo_container.h>

#include <boost/filesystem.hpp>

#include <map>
#include <set>
#include <vector>

// CompanEdge Protocol fwd declarations
namespace CompanEdgeProtocol {
enum Value_Type : int;
class Value;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

/*!
 *
 */
class ValueIdGenerator {
public:
    using IncludePaths = std::vector<boost::filesystem::path>;
    using ValueTypeSet = std::set<CompanEdgeProtocol::Value_Type>;

public:
    ValueIdGenerator(DmoContainer& dmo, std::string const postfix);
    virtual ~ValueIdGenerator() = default;

    /// Generates the contents
    bool generate(std::ostream& strm);

    /// Validate the DMO container and gather preliminary information
    virtual bool doPreGeneration();

    virtual void generateHead(std::ostream& strm) = 0;
    virtual void generateBody(std::ostream& strm) = 0;
    virtual void generateFoot(std::ostream& strm) = 0;

    virtual void generateClass(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth) = 0;

    virtual void generateClassMembers(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth) = 0;

    virtual void generateClassInstance(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth) = 0;

    void generateIncludes(std::ostream& strm);

    /// walks the DMO container for CompanEdgeProtocol::types
    void findValueTypes(DmoContainer::TreeType const branch);

    /// Sets the base id
    void baseId(ValueId const& arg);

    /// Gets the base id
    ValueId baseId() const;

    /// Sets the microservice name
    void microserviceName(std::string const& arg);

    /// Gets the microservice name
    std::string microserviceName() const;

    /// Sets the class name
    void className(std::string const& arg);

    /// Get's the class name being exported
    std::string className() const;

    /// Set's the file name (minus extension)
    void fileName(std::string const& arg);

    /// Get's the file name (minus extension)
    std::string fileName() const;

    /// Gets the auto-generation for enum types
    bool generateEnumTypes() const;

    /// Sets the auto-generation for enum types
    void generateEnumTypes(bool const arg);

    /// Makes sure the member value name isn't a keyword, and if it is
    /// replace it with a appropriate name
    std::string keywordReplace(std::string const&);

    /// returns the over-arching class name
    static ValueId getOuterClassName(ValueId const&, DmoContainer::TreeType const);

    /// Returns camel case spelling: name -> Name or my_name -> My_Name
    static std::string camelCase(std::string const& arg);

    static std::string makeValueTypeClassName(CompanEdgeProtocol::Value_Type const arg);

    /// Converts a.b to A::B::
    static std::string convertParent(ValueId const& parentId);

    /// Returns a Derived Variant type name, or the Class name
    static std::string getMemberType(CompanEdgeProtocol::Value const& value);

    /// Returns a member variable name
    static std::string getMemberName(CompanEdgeProtocol::Value const& value);

    /// Returns a derived VariantPtr or Class::Ptr
    static std::string getClassPtrType(CompanEdgeProtocol::Value const& value);

    /// Replaces '-' with '_'
    static std::string makeNormalizedName(std::string const& arg);

    static std::string createValueIdDecl(std::string const& first, std::string const& second = "");

    /// Returns arg with " around it
    static std::string quoteString(std::string const& arg);

protected:
    void makeClassName();
    void makeFileName();

protected:
    DmoContainer& dmo_;

    ValueId fileClass_;        //!< Outer class parent
    std::string className_;    //!< Outer class name for the module
    std::string fileNameBase_; //!< File name base
    std::string microserviceName_;
    std::string postfix_;
    ValueId baseId_;
    bool generateEnumTypes_;

    ValueTypeSet valueTypes_; //!< Value types found in the DMO - used for fwd/include

    IncludePaths includePaths_;

    std::map<std::string, std::string> keywords_; // common keywords that need replacement

protected:
    static constexpr int TabSpace = 4;
    static std::string const Indent;

    static std::string const ContainerValueTypeName; //!< Used when there is a container of Type
};

inline void ValueIdGenerator::baseId(ValueId const& arg)
{
    baseId_ = ValueId(arg);
}

inline ValueId ValueIdGenerator::baseId() const
{
    return baseId_;
}

inline void ValueIdGenerator::className(std::string const& arg)
{
    className_ = arg;
    fileNameBase_ = arg;
}

inline std::string ValueIdGenerator::className() const
{
    return className_;
}

inline void ValueIdGenerator::fileName(std::string const& arg)
{
    fileNameBase_ = arg;
}

inline std::string ValueIdGenerator::fileName() const
{
    return fileNameBase_;
}

inline bool ValueIdGenerator::generateEnumTypes() const
{
    return generateEnumTypes_;
}

inline void ValueIdGenerator::generateEnumTypes(bool const arg)
{
    generateEnumTypes_ = arg;
}

inline void ValueIdGenerator::microserviceName(std::string const& arg)
{
    microserviceName_ = arg;
}

inline std::string ValueIdGenerator::microserviceName() const
{
    return microserviceName_;
}

} // namespace Edge
} // namespace Compan

#endif // __Compan_DMO_VALUEID_GENERATOR_H__
