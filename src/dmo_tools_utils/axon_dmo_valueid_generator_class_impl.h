/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_valueid_generator_class_impl.h
 @brief ValueId code generator - class implementations
 */
#ifndef __Compan_DMO_VALUEID_GENERATOR_CLASS_IMPL_H__
#define __Compan_DMO_VALUEID_GENERATOR_CLASS_IMPL_H__

#include "Compan_dmo_valueid_generator.h"

namespace Compan{
namespace Edge {

class ValueIdGeneratorClassImpl : public ValueIdGenerator {
public:
    ValueIdGeneratorClassImpl(DmoContainer& dmo);

    virtual ~ValueIdGeneratorClassImpl() = default;

    /// Require to include headers from 'company_ref_variant_valuestore'
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
            ValueId const&,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth);

    void generateBaseValueIds(std::ostream& strm);

    void generatePointerValidate(
            ValueId const&,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const depth);

    void generateIsValidSignature(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            std::ostream& strm,
            int const);

protected:
    std::string instantiationIdName_;
    std::string loggerName_;

    static std::string const ValueIdTerminal;
    static std::string const InstanceValueIdTerminal;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_DMO_VALUEID_GENERATOR_CLASS_IMPL_H__
