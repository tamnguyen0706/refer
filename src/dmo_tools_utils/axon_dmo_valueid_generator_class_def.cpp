/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_valueid_generator_class_def.cpp
 @brief ValueId code generator - class definitions
 */
#include "Compan_dmo_valueid_generator_class_def.h"

#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger ValueIdGeneratorClassDefLog("generator.classdef", LogLevel::Information);

ValueIdGeneratorClassDef::ValueIdGeneratorClassDef(DmoContainer& dmo)
    : ValueIdGenerator(dmo, "ValueIds")
{
}

bool ValueIdGeneratorClassDef::doPreGeneration()
{
    if (!ValueIdGenerator::doPreGeneration()) return false;

    includePaths_.push_back("company_ref_variant_valuestore/company_ref_variant_valuestore.h");
    includePaths_.push_back("memory");

    ifndefStr_ = std::string("__") + fileNameBase_ + std::string("_H__");
    std::transform(
            ifndefStr_.begin(),
            ifndefStr_.end(),
            ifndefStr_.begin(),
            [](unsigned char c) { return std::toupper(c); } // correct
    );

    return true;
}

void ValueIdGeneratorClassDef::generateHead(std::ostream& strm)
{
    strm << "#ifndef " << ifndefStr_ << std::endl;
    strm << "#define " << ifndefStr_ << std::endl << std::endl;

    generateIncludes(strm);

    strm << "namespace Compan{" << std::endl;
    strm << "namespace Edge {" << std::endl << std::endl;

    for (auto& types : valueTypes_) {
        std::string className(makeValueTypeClassName(types));
        strm << "class " << className << ";" << std::endl;
        strm << "using " << className << "Ptr = std::shared_ptr<" << className << ">;" << std::endl;
        strm << std::endl;
    }

    strm << "class ValueId;" << std::endl << std::endl;
}

void ValueIdGeneratorClassDef::generateBody(std::ostream& strm)
{
    strm << "// " << fileClass_ << std::endl;
    strm << "class " << className_ << " {" << std::endl
         << "public:" << std::endl
         << Indent << className_ << "(VariantValueStore&);" << std::endl
         << Indent << "~" << className_ << "() = default;" << std::endl
         << std::endl;

    generateIsValidSignature(strm, 0);

    ValueId parentId;

    parentId += fileClass_;
    DmoContainer::OptionalTreeType parentBranch = dmo_.getMetaData(fileClass_);
    if (parentBranch) { generateChildren(parentId, parentBranch.get(), strm, 0); }

    strm << std::endl;
    strm << "public:" << std::endl << Indent << "static ValueId const BaseValueId;" << std::endl;

    if (!microserviceName_.empty()) strm << Indent << "static std::string const MicroserviceName;" << std::endl;

    generateIsValidDefinition(strm, 0);

    strm << "}; // " << className_ << std::endl << std::endl;
}

void ValueIdGeneratorClassDef::generateFoot(std::ostream& strm)
{
    strm << "} // namespace edge" << std::endl;
    strm << "} // namespace Compan" << std::endl << std::endl;

    strm << "#endif // " << ifndefStr_ << std::endl << std::endl;
}

void ValueIdGeneratorClassDef::generateClass(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const depth)
{
    FunctionArgLog(ValueIdGeneratorClassDefLog) << ": " << parentId << " - " << branch.data().id() << std::endl;

    if (branch.data().id() == DmoContainer::MetaContainerDelim) return;

    std::string const indent(depth * TabSpace, ' ');

    std::string className(getMemberType(branch.data()));

    ValueId classValueId(parentId, branch.data().id());

    strm << indent << "// " << classValueId << std::endl;
    strm << indent << "class " << className << " : public ValueId {" << std::endl
         << indent << "public:" << std::endl
         << indent << Indent << "using Ptr = std::shared_ptr<" << className << ">;" << std::endl
         << indent << Indent << className << "(VariantValueStore&, ValueId const&);" << std::endl
         << indent << Indent << "~" << className << "() = default;" << std::endl
         << std::endl;

    generateIsValidSignature(strm, depth);

    generateChildren(classValueId, branch, strm, depth);

    generateIsValidDefinition(strm, depth);

    strm << indent << "}; // class " << className << std::endl << std::endl;
}

void ValueIdGeneratorClassDef::generateClassInstance(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const depth)
{
    FunctionArgLog(ValueIdGeneratorClassDefLog) << ": " << parentId << " - " << branch.data().id() << std::endl;

    std::string const indent(depth * TabSpace, ' ');

    std::string memberName(makeNormalizedName(parentId.leaf().name()));
    std::string className = camelCase(memberName);
    ValueId classValueId(parentId);

    strm << indent << "// " << classValueId << std::endl;
    strm << indent << "class " << className << " : public ValueId {" << std::endl
         << indent << "public:" << std::endl
         << indent << Indent << "using Ptr = std::shared_ptr<" << className << ">;" << std::endl
         << indent << Indent << className << "(VariantValueStore&, ValueId const&,ValueId const&);" << std::endl
         << indent << Indent << "~" << className << "() = default;" << std::endl
         << std::endl;

    generateIsValidSignature(strm, depth);

    if (branch.data().type() == CompanEdgeProtocol::Struct)
        generateChildren(classValueId, branch, strm, depth);
    else
        strm << indent << Indent << getClassPtrType(branch.data()) << " keyValue;" << std::endl;

    generateIsValidDefinition(strm, depth);

    strm << indent << "}; // class " << className << std::endl << std::endl;
}

void ValueIdGeneratorClassDef::generateEnums(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const depth)
{
    if (!generateEnumTypes_) return;

    FunctionArgLog(ValueIdGeneratorClassDefLog) << ": " << parentId << " - " << branch.data().id() << std::endl;

    std::string const indent(depth * TabSpace, ' ');

    if (branch.data().type() != CompanEdgeProtocol::Enum) return;

    std::string memberName(branch.data().id());

    std::string const enumeratorIndent((depth + 1) * TabSpace, ' ');

    strm << indent << "enum " << camelCase(memberName) << "Enum {" << std::endl;

    for (auto& enumerator : branch.data().enumvalue().enumerators())
        strm << enumeratorIndent << makeNormalizedName(enumerator.text()) << " = " << enumerator.value() << ","
             << std::endl;

    strm << indent << "};" << std::endl << std::endl;
}

void ValueIdGeneratorClassDef::generateClassMembers(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const depth)
{
    FunctionArgLog(ValueIdGeneratorClassDefLog) << ": " << parentId << " - " << branch.data().id() << std::endl;

    if (branch.data().id() == DmoContainer::MetaContainerDelim) return;

    std::string const indent(depth * TabSpace, ' ');

    for (auto& pos : branch) {

        ValueId parentClassId(parentId, branch.data().id());
        if (dmo_.isInstance(ValueId(parentClassId, pos.second.data().id()))) continue;

        if (pos.first == DmoContainer::MetaContainerDelim) continue;

        strm << indent << Indent << getClassPtrType(pos.second.data()) << " "
             << keywordReplace(getMemberName(pos.second.data())) << ";" << std::endl;
    }

    strm << indent << Indent << getClassPtrType(branch.data()) << " " << keywordReplace(getMemberName(branch.data()))
         << ";" << std::endl;
}

void ValueIdGeneratorClassDef::generateChildren(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const depth)
{
    FunctionArgLog(ValueIdGeneratorClassDefLog) << ": " << parentId << " - " << branch.data().id() << std::endl;

    std::string const indent(depth * TabSpace, ' ');

    std::stringstream enumStrm;
    std::stringstream classStrm;
    std::stringstream memberStrm;

    for (auto& pos : branch) {

        switch (pos.second.data().type()) {
        case CompanEdgeProtocol::Struct: {
            if (pos.second.data().id() == DmoContainer::MetaContainerDelim) continue;

            generateClass(parentId, pos.second, classStrm, depth + 1);

            memberStrm << indent << Indent << getClassPtrType(pos.second.data()) << " "
                       << keywordReplace(getMemberName(pos.second.data())) << ";" << std::endl;
        } break;

        case CompanEdgeProtocol::Enum:
            generateEnums(parentId, pos.second, enumStrm, depth + 1);
            generateClassMembers(parentId, pos.second, memberStrm, depth);
            break;

        default:
            if (pos.second.data().type() == CompanEdgeProtocol::Container) {
                ValueId classInstanceId(parentId, pos.first);

                for (auto& metaPos : pos.second) {
                    // skip creating instance related structs
                    if (metaPos.first != DmoContainer::MetaContainerDelim) continue;
                    generateClassInstance(classInstanceId, metaPos.second, classStrm, depth + 1);
                }
            }

            generateClassMembers(parentId, pos.second, memberStrm, depth);

            break;
        }
    }

    std::string eStr = enumStrm.str();
    std::string cStr = classStrm.str();
    std::string mStr = memberStrm.str();

    if (!eStr.empty()) strm << eStr;
    if (!cStr.empty()) strm << cStr;
    if (!mStr.empty()) strm << mStr;
}

void ValueIdGeneratorClassDef::generateIsValidSignature(std::ostream& strm, int const depth)
{
    std::string const indent(depth * TabSpace, ' ');

    strm << indent << Indent << "bool isValid() const;" << std::endl << std::endl;
}

void ValueIdGeneratorClassDef::generateIsValidDefinition(std::ostream& strm, int const depth)
{
    std::string const indent(depth * TabSpace, ' ');

    strm << std::endl << indent << "private:" << std::endl << indent << Indent << "bool isValid_;" << std::endl;
}
