/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_valueid_generator_class_impl.cpp
 @brief ValueId code generator - class implementations
 */
#include "Compan_dmo_valueid_generator_class_impl.h"

using namespace Compan::Edge;

std::string const ValueIdGeneratorClassImpl::ValueIdTerminal("valueId");
std::string const ValueIdGeneratorClassImpl::InstanceValueIdTerminal("instanceValueId");

ValueIdGeneratorClassImpl::ValueIdGeneratorClassImpl(DmoContainer& dmo)
    : ValueIdGenerator(dmo, "ValueIds")
{
}

bool ValueIdGeneratorClassImpl::doPreGeneration()
{
    if (!ValueIdGenerator::doPreGeneration()) return false;

    loggerName_ = className_ + std::string("Log");

    boost::filesystem::path const sdkDir = "company_ref_variant_valuestore";

    includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_valuestore_valueid.h"));

    for (auto& types : valueTypes_) {
        switch (types) {

        case CompanEdgeProtocol::Bool:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_bool_value.h"));
            break;

        case CompanEdgeProtocol::SInterval:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_sinterval_value.h"));
            break;
        case CompanEdgeProtocol::USInterval:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_usinterval_value.h"));
            break;
        case CompanEdgeProtocol::Interval:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_interval_value.h"));
            break;
        case CompanEdgeProtocol::UInterval:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_uinterval_value.h"));
            break;
        case CompanEdgeProtocol::LLInterval:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_llinterval_value.h"));
            break;
        case CompanEdgeProtocol::ULLInterval:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_ullinterval_value.h"));
            break;

        case CompanEdgeProtocol::DInterval:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_dinterval_value.h"));
            break;

        case CompanEdgeProtocol::Enum:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_enum_value.h"));
            break;

        case CompanEdgeProtocol::Text:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_text_value.h"));
            break;

        case CompanEdgeProtocol::IPv4:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_ipv4_value.h"));
            break;

        case CompanEdgeProtocol::IPv6:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_ipv6_value.h"));
            break;

        case CompanEdgeProtocol::TimeVal:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_timeval_value.h"));
            break;

        case CompanEdgeProtocol::TimeSpec:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_timespec_value.h"));
            break;

        case CompanEdgeProtocol::EUI48:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_eui48_value.h"));
            break;

            //
            // These need to be filled in when the derived classes are completed
            //
            // case CompanEdgeProtocol::Multi:
            // case CompanEdgeProtocol::Struct:

        case CompanEdgeProtocol::Udid:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_udid_value.h"));
            break;

        case CompanEdgeProtocol::Container:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_map_value.h"));
            break;

        case CompanEdgeProtocol::UnorderedSet:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_unorderedset_value.h"));
            break;

        case CompanEdgeProtocol::Set:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_set_value.h"));
            break;

        case CompanEdgeProtocol::Vector:
            includePaths_.push_back(sdkDir / boost::filesystem::path("company_ref_variant_vector_value.h"));
            break;

        default: break;
        }
    }

    includePaths_.push_back("Compan_logger/Compan_logger.h");

    return true;
}

void ValueIdGeneratorClassImpl::generateHead(std::ostream& strm)
{
    strm << "#include \"" << fileNameBase_ << ".h\"" << std::endl << std::endl;

    generateIncludes(strm);

    strm << "using namespace Compan::Edge;" << std::endl << std::endl;

    strm << "CompanLogger " << loggerName_ << "(\"value_ids." << fileClass_ << "\");" << std::endl << std::endl;

    generateBaseValueIds(strm);
}

void ValueIdGeneratorClassImpl::generateBody(std::ostream& strm)
{
    strm << className_ << "::" << className_ << "(VariantValueStore& ws)" << std::endl;

    std::stringstream classStrm;
    std::stringstream validationStrm;

    DmoContainer::OptionalTreeType parentBranch = dmo_.getMetaData(fileClass_);
    if (parentBranch) {

        bool isFirst(true);
        for (auto& children : parentBranch.get()) {

            CompanEdgeProtocol::Value branchData = children.second.data();

            std::string memberType(getMemberType(branchData));
            std::string memberName(getMemberName(branchData));

            ValueId parentId(className_, memberType);

            if (branchData.type() == CompanEdgeProtocol::Container) {
                ValueId parentPath(className_, camelCase(makeNormalizedName(memberName)));

                ValueIdGeneratorClassImpl::generateClassInstance(parentPath, children.second, classStrm, 0);
            }

            if (memberName != DmoContainer::MetaContainerDelim) {
                strm << Indent << (isFirst ? ": " : ", ") << keywordReplace(memberName) << "(";

                // opening params
                if (branchData.type() == CompanEdgeProtocol::Struct) {
                    strm << "std::make_shared<" << getMemberType(branchData) << ">(ws, ";
                } else {
                    strm << "ws.get<" << getMemberType(branchData) << ">(";
                }

                if (ValueId(fileClass_, branchData.id()) == baseId_)
                    strm << "BaseValueId";
                else
                    // closing params
                    strm << createValueIdDecl("BaseValueId", quoteString(branchData.id()));

                strm << ")";

                strm << ")" << std::endl;

                generatePointerValidate(parentId, children.second, validationStrm, 0);

                isFirst = false;
            }

            generateClass(parentId, children.second, classStrm, 0);
        }
    }

    strm << Indent << ", isValid_(true)" << std::endl;

    strm << "{" << std::endl << validationStrm.str() << "}" << std::endl << std::endl;

    generateIsValidSignature(
            ValueId(className_),
            dmo_.root(), // this param doesn't matter
            strm,
            0);

    std::string cStr = classStrm.str();
    if (!cStr.empty()) strm << cStr;
}

void ValueIdGeneratorClassImpl::generateFoot(std::ostream&)
{
}

void ValueIdGeneratorClassImpl::generateClass(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const depth)
{
    if (branch.data().type() != CompanEdgeProtocol::Struct) return;

    if (branch.data().id() == DmoContainer::MetaContainerDelim) return;

    std::string memberType(getMemberType(branch.data()));
    std::string memberName(getMemberName(branch.data()));
    std::stringstream validationStrm;

    ValueId parentClass(parentId);
    if (parentClass.empty()) parentClass = memberType;

    strm << convertParent(parentClass) << memberType << "(VariantValueStore& ws, ValueId const& valueId)" << std::endl;

    strm << Indent << ": " << createValueIdDecl(ValueIdTerminal) << std::endl;

    for (auto& pos : branch) {
        generateClassMembers(ValueIdTerminal, pos.second, strm, depth);
        generatePointerValidate(parentId, pos.second, validationStrm, 0);
    }

    strm << Indent << ", isValid_(true)" << std::endl;

    strm << "{" << std::endl << validationStrm.str() << "}" << std::endl << std::endl;

    generateIsValidSignature(parentClass, branch, strm, 0);

    for (auto& pos : branch) {

        if (pos.second.data().type() == CompanEdgeProtocol::Container) {
            ValueId parentPath;

            if (parentId.empty())
                parentPath += parentClass;
            else
                parentPath += parentId;

            parentPath += camelCase(makeNormalizedName(pos.second.data().id()));

            ValueIdGeneratorClassImpl::generateClassInstance(parentPath, pos.second, strm, 0);

            continue;
        }

        ValueId thisClass(parentClass, getMemberType(pos.second.data()));

        generateClass(thisClass, pos.second, strm, depth);
    }
}

void ValueIdGeneratorClassImpl::generateClassInstance(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const)
{
    // The parentId contains the Class type at the end, reuse it
    strm << convertParent(parentId) << parentId.leaf()
         << "(VariantValueStore& ws, ValueId const& valueId, ValueId const& instanceValueId)" << std::endl;

    strm << Indent << ": " << createValueIdDecl(ValueIdTerminal, InstanceValueIdTerminal) << std::endl;

    std::stringstream classStrm;
    std::stringstream validationStrm;

    for (auto& metaPos : branch) {

        // skip creating instance related structs
        if (metaPos.first == DmoContainer::MetaContainerDelim) {
            // unless the value is a POD type
            if (metaPos.second.data().type() != CompanEdgeProtocol::Struct) {

                CompanEdgeProtocol::Value branchData = metaPos.second.data();

                strm << Indent << ", " << ContainerValueTypeName << "(";

                // opening params
                strm << "ws.get<" << getMemberType(branchData) << ">("
                     << createValueIdDecl(ValueIdTerminal, InstanceValueIdTerminal) << ")";

                // closing params
                strm << ")" << std::endl;

                generatePointerValidate(parentId, metaPos.second, validationStrm, 0);

                continue;
            }
        } else
            continue;

        for (auto& pos : metaPos.second) {

            CompanEdgeProtocol::Value branchData = pos.second.data();

            if (branchData.type() == CompanEdgeProtocol::Container) {
                ValueId parentPath(parentId);

                parentPath += camelCase(makeNormalizedName(branchData.id()));

                ValueIdGeneratorClassImpl::generateClassInstance(parentPath, pos.second, classStrm, 0);
            }

            strm << Indent << ", " << keywordReplace(getMemberName(branchData)) << "(";

            // opening params
            if (branchData.type() == CompanEdgeProtocol::Struct) {
                strm << "std::make_shared<" << getMemberType(branchData) << ">(ws, ";
            } else {
                strm << "ws.get<" << getMemberType(branchData) << ">(";
            }

            // closing params
            strm << createValueIdDecl(
                    createValueIdDecl(ValueIdTerminal, InstanceValueIdTerminal), quoteString(branchData.id()));
            strm << "))" << std::endl;

            ValueId thisClass(parentId, getMemberType(pos.second.data()));
            generateClass(thisClass, pos.second, classStrm, 0);

            generatePointerValidate(thisClass, pos.second, validationStrm, 0);
        }
    }

    strm << Indent << ", isValid_(true)" << std::endl;

    strm << "{" << std::endl << validationStrm.str() << "}" << std::endl << std::endl;

    generateIsValidSignature(parentId, branch, strm, 0);

    std::string cStr = classStrm.str();
    if (!cStr.empty()) strm << cStr;
}

void ValueIdGeneratorClassImpl::generateClassMembers(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const)
{
    if (branch.data().id() == DmoContainer::MetaContainerDelim) return;

    strm << Indent << ", " << keywordReplace(getMemberName(branch.data())) << "(";
    if (branch.data().type() == CompanEdgeProtocol::Struct) {
        strm << "std::make_shared<" << getMemberType(branch.data()) << ">(ws, "
             << createValueIdDecl(parentId, quoteString(branch.data().id())) << ")";
    } else {
        strm << "ws.get<" << getMemberType(branch.data()) << ">("
             << createValueIdDecl(parentId, quoteString(branch.data().id())) << ")";
    }

    strm << ")" << std::endl;
    ;
}

void ValueIdGeneratorClassImpl::generatePointerValidate(
        ValueId const&,
        DmoContainer::TreeType const& branch,
        std::ostream& strm,
        int const)
{
    std::string memberName(getMemberName(branch.data()));

    if (branch.data().id() == DmoContainer::MetaContainerDelim) {
        if (branch.data().type() == CompanEdgeProtocol::Struct) return;

        memberName = ContainerValueTypeName;
    }

    std::string orStatement;
    if (branch.data().type() == CompanEdgeProtocol::Struct) {
        std::stringstream orStrm;

        orStrm << " || !" << keywordReplace(memberName) << "->isValid()";
        orStatement = orStrm.str();
    }

    strm << Indent << "if (!" << keywordReplace(memberName) << orStatement << ") {" << std::endl;
    strm << Indent << Indent << "isValid_=false;" << std::endl;
    strm << Indent << Indent << "ErrorLog(" << loggerName_ << ") << \"Invalid pointer: " << keywordReplace(memberName)
         << "\" << std::endl;" << std::endl;

    strm << Indent << Indent << "return;" << std::endl;

    strm << Indent << "}" << std::endl;
}

void ValueIdGeneratorClassImpl::generateIsValidSignature(
        ValueId const& parentId,
        DmoContainer::TreeType const&,
        std::ostream& strm,
        int const)
{
    strm << "bool " << convertParent(parentId) << "isValid() const {" << std::endl
         << Indent << "return isValid_;" << std::endl
         << "}" << std::endl
         << std::endl;
}

void ValueIdGeneratorClassImpl::generateBaseValueIds(std::ostream& strm)
{
    ValueId lastId(baseId_.leaf());

    std::string prevValueName;

    if (!baseId_.parent().empty()) {

        strm << "namespace {" << std::endl;
        for (auto& valueId : baseId_.parent()) {

            std::string valueName = camelCase(makeNormalizedName(valueId.name())) + "Id";
            strm << Indent << "ValueId const " << valueName << "(";

            if (prevValueName.empty())
                strm << quoteString(valueId);
            else
                strm << createValueIdDecl(prevValueName, quoteString(valueId));

            strm << ");" << std::endl;

            prevValueName = valueName;
        }

        strm << "} // namespace" << std::endl << std::endl;
    }

    strm << "ValueId const " << className_ << "::BaseValueId(";

    if (prevValueName.empty())
        strm << quoteString(lastId);
    else
        strm << createValueIdDecl(prevValueName, quoteString(lastId));

    strm << ");" << std::endl;

    if (!microserviceName_.empty())
        strm << "std::string const " << className_ << "::MicroserviceName(" << quoteString(microserviceName_) << ");"
             << std::endl;

    strm << std::endl;
}
