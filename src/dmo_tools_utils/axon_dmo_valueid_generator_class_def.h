/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_valueid_generator_class_def.h
 @brief ValueId code generator - class definitions
 */
#ifndef __Compan_DMO_VALUEID_GENERATOR_CLASS_DEF_H__
#define __Compan_DMO_VALUEID_GENERATOR_CLASS_DEF_H__

#include "Compan_dmo_valueid_generator.h"

namespace Compan{
namespace Edge {

class ValueIdGeneratorClassDef : public ValueIdGenerator {
public:
    ValueIdGeneratorClassDef(DmoContainer& dmo);

    virtual ~ValueIdGeneratorClassDef() = default;

    /// Required to include headers shared_ptr headers
    virtual bool doPreGeneration();

    virtual void generateHead(std::ostream& strm);
    virtual void generateBody(std::ostream& strm);
    virtual void generateFoot(std::ostream& strm);

    virtual void generateClass(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth);

    virtual void generateClassInstance(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth);

    virtual void generateClassMembers(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth);

protected:
    void generateEnums(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth);

    void generateChildren(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth);

    void generateIsValidSignature(std::ostream& strm, int const depth);
    void generateIsValidDefinition(std::ostream& strm, int const depth);

private:
    std::string ifndefStr_;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_DMO_VALUEID_GENERATOR_CLASS_DEF_H__
