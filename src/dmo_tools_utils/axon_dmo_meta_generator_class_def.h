/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_meta_generator_class_def.h
 @brief -*-*-*-*-*-
 */
#ifndef __Compan_DMO_META_GENERATOR_CLASS_DEF_H__
#define __Compan_DMO_META_GENERATOR_CLASS_DEF_H__

#include "Compan_dmo_valueid_generator.h"

namespace Compan{
namespace Edge {

class MetaGeneratorClassDef : public ValueIdGenerator {
public:
    MetaGeneratorClassDef(DmoContainer& dmo);

    virtual ~MetaGeneratorClassDef() = default;

    /// Required to include headers shared_ptr headers
    virtual bool doPreGeneration();

    virtual void generateHead(std::ostream& strm);
    virtual void generateBody(std::ostream& strm);
    virtual void generateFoot(std::ostream& strm);

    virtual void generateClass(ValueId const&, DmoContainer::TreeType const&, std::ostream&, int const)
    {
    }

    virtual void generateClassMembers(ValueId const&, DmoContainer::TreeType const&, std::ostream&, int const)
    {
    }

    virtual void generateClassInstance(ValueId const&, DmoContainer::TreeType const&, std::ostream&, int const)
    {
    }

private:
    std::string ifndefStr_;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_DMO_META_GENERATOR_CLASS_DEF_H__
