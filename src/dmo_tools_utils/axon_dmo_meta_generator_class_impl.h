/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_meta_generator_class_impl.h
 @brief -*-*-*-*-*-
 */
#ifndef __Compan_DMO_META_GENERATOR_CLASS_IMPL_H__
#define __Compan_DMO_META_GENERATOR_CLASS_IMPL_H__

#include "Compan_dmo_valueid_generator.h"

namespace Compan{
namespace Edge {

class MetaGeneratorClassImpl : public ValueIdGenerator {
public:
    MetaGeneratorClassImpl(DmoContainer& dmo);

    virtual ~MetaGeneratorClassImpl() = default;

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

    // protected:
    void generateNamespaceValueIds(std::ostream&);
    void makeDeclValueId(ValueId const& valueId, std::ostream& strm);

    void createWs(std::ostream&);
    void createWsElement(CompanEdgeProtocol::Value const&, std::ostream&);

    void createMetaData(std::ostream&);
    void createMetaDataElement(CompanEdgeProtocol::Value const&, std::ostream&);

    std::string createEnumeratorVector(CompanEdgeProtocol::Value const&, std::ostream&);

    std::string makeDeclValueIdName(ValueId const& lhs, ValueId const& rhs);
    std::string makeDeclValueIdName(ValueId const& lhs_first, ValueId const& lhs_second, ValueId const& rhs);

    static std::string makeVariableName(ValueId const& parentId);
    static std::string makeValueIdName(ValueId const& valueId);

    void makeCreateValueWrapper(ValueId const&, CompanEdgeProtocol::Value_Type const&, std::ostream&);

private:
    std::string loggerName_;
    int delimCounter_;

    std::map<ValueId, std::string> valueIdVariable_;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_DMO_META_GENERATOR_CLASS_IMPL_H__
