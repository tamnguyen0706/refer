/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_meta_generator_class_impl.cpp
 @brief -*-*-*-*-*-
 */
#include "Compan_dmo_meta_generator_class_impl.h"

#include <company_ref_protocol_utils/company_ref_pb_enum.h>
#include <company_ref_protocol_utils/company_ref_stream.h>

#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger MetaGeneratorClassImplLog("generator.classdef", LogLevel::Information);

MetaGeneratorClassImpl::MetaGeneratorClassImpl(DmoContainer& dmo)
    : ValueIdGenerator(dmo, "MetaData")
    , delimCounter_(0)
{
    FunctionLog(MetaGeneratorClassImplLog);
}

bool MetaGeneratorClassImpl::doPreGeneration()
{
    FunctionLog(MetaGeneratorClassImplLog);

    if (!ValueIdGenerator::doPreGeneration()) return false;

    loggerName_ = className_ + std::string("Log");

    return true;
}

void MetaGeneratorClassImpl::generateHead(std::ostream& strm)
{
    FunctionLog(MetaGeneratorClassImplLog);

    strm << "#include \"" << fileNameBase_ << ".h\"" << std::endl << std::endl;

    includePaths_.push_back("company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h");
    includePaths_.push_back("company_ref_protocol_utils/company_ref_pb_init.h");
    includePaths_.push_back("company_ref_protocol_utils/company_ref_pb_enum.h");
    includePaths_.push_back("company_ref_protocol_utils/company_ref_pb_accesors.h");
    includePaths_.push_back("Compan_logger/Compan_logger.h");

    includePaths_.push_back("string");
    includePaths_.push_back("set");

    generateIncludes(strm);

    strm << "using namespace Compan::Edge;" << std::endl << std::endl;

    strm << "CompanLogger " << loggerName_ << "(\"metadata." << fileClass_ << "\");" << std::endl << std::endl;
}

void MetaGeneratorClassImpl::generateFoot(std::ostream&)
{
    FunctionLog(MetaGeneratorClassImplLog);
}

void MetaGeneratorClassImpl::generateBody(std::ostream& strm)
{
    FunctionLog(MetaGeneratorClassImplLog);

    std::stringstream dmoStrm;
    std::stringstream wsStrm;

    strm << "namespace {" << std::endl << std::endl;

    generateNamespaceValueIds(strm);

    //    strm << std::endl;
    //
    strm << "CompanEdgeProtocol::Value createValueWrapper( ValueId const & id, CompanEdgeProtocol::Value_Type const type ) "
            "{"
         << std::endl
         << Indent << "CompanEdgeProtocol::Value value;" << std::endl
         << Indent << "valueInit(value, type);" << std::endl
         << Indent << "value.set_id(id.name());" << std::endl
         << Indent << "return value;" << std::endl
         << "}" << std::endl
         << std::endl;

    createMetaData(strm);

    strm << "} // namespace" << std::endl << std::endl;

    strm << "void " << className() << "::create(VariantValueStore& ws, DmoContainer& dmo) {" << std::endl
         << Indent << "create(ws);" << std::endl
         << Indent << "createMetaData(dmo);" << std::endl
         << "}" << std::endl
         << std::endl;

    createWs(strm);
}

void MetaGeneratorClassImpl::makeDeclValueId(ValueId const& valueId, std::ostream& strm)
{
    if (valueId.empty()) return;

    strm << "// " << valueId << std::endl;

    strm << "ValueId const " << makeValueIdName(valueId) << "(";

    // single entry
    if (!valueId.parent().empty()) {
        if (valueIdVariable_.find(valueId.parent()) != valueIdVariable_.end())
            strm << makeValueIdName(valueId.parent());
        else {
            if (valueId.parent().leaf() == DmoContainer::MetaContainerDelimId) {
                ValueId parent(valueId.parent());
                strm << createValueIdDecl(makeValueIdName(parent.parent()), quoteString(parent.leaf()));
            }
        }

        strm << ", ";
    }

    strm << quoteString(valueId.leaf());

    strm << ");" << std::endl;

    strm << std::endl;
}

void MetaGeneratorClassImpl::generateNamespaceValueIds(std::ostream& strm)
{
    dmo_.visitMetaData([this](CompanEdgeProtocol::Value const& value) {
        valueIdVariable_[value.id()] = makeValueIdName(value.id());
    });

    dmo_.visitValues([this](CompanEdgeProtocol::Value const& value) {
        if (valueIdVariable_.find(value.id()) != valueIdVariable_.end()) return;
        valueIdVariable_[value.id()] = makeValueIdName(value.id());
    });

    for (auto& element : valueIdVariable_) makeDeclValueId(element.first, strm);
}

void MetaGeneratorClassImpl::makeCreateValueWrapper(
        ValueId const& id,
        CompanEdgeProtocol::Value_Type const& type,
        std::ostream& strm)
{
    if (valueIdVariable_.find(id) == valueIdVariable_.end()) {
        ErrorLog(MetaGeneratorClassImplLog) << "makeCreateValueWrapper: " << id << " not declared" << std::endl;
        return;
    }

    strm << "createValueWrapper(" << valueIdVariable_[id]
         << ", CompanEdgeProtocol::" << CompanEdgeProtocol::Value_Type_Name(type) << ")";
}

void MetaGeneratorClassImpl::createWs(std::ostream& strm)
{
    strm << "void " << className() << "::create(VariantValueStore& ws) {" << std::endl;

    dmo_.visitValues([this, &strm](CompanEdgeProtocol::Value const& value) { createWsElement(value, strm); });

    strm << "}" << std::endl << std::endl;
}

void MetaGeneratorClassImpl::createWsElement(CompanEdgeProtocol::Value const& value, std::ostream& strm)
{
    if (value.type() == CompanEdgeProtocol::Struct) return;

    ValueId valueId(value.id());

    if (valueIdVariable_.find(valueId) == valueIdVariable_.end()) {
        ErrorLog(MetaGeneratorClassImplLog) << "makeCreateValueWrapper: " << valueId << " not declared" << std::endl;
        return;
    }

    if (value.type() == CompanEdgeProtocol::Enum) {

        std::string memberVariableName(makeVariableName(valueId) + "Value");

        std::string enumVector = createEnumeratorVector(value, strm);

        strm << Indent << "valueInit(" << memberVariableName << ", " << enumVector << ");" << std::endl;

        strm << Indent << "valueSet<std::string>(" << memberVariableName << ", "
             << quoteString(ValueEnumerators::toString(value)) << ");" << std::endl;

        strm << Indent << "dmo.insertMetaData(" << valueIdVariable_[valueId] << ", " << memberVariableName << ");"
             << std::endl;
        return;
    }

    strm << Indent << "ws.set(";
    makeCreateValueWrapper(value.id(), value.type(), strm);
    strm << ", VariantValue::Remote);" << std::endl;
}

void MetaGeneratorClassImpl::createMetaData(std::ostream& strm)
{
    strm << "void createMetaData(DmoContainer& dmo) {" << std::endl;

    dmo_.visitMetaData([this, &strm](CompanEdgeProtocol::Value const& value) { createMetaDataElement(value, strm); });

    strm << "}" << std::endl << std::endl;
}

void MetaGeneratorClassImpl::createMetaDataElement(CompanEdgeProtocol::Value const& value, std::ostream& strm)
{
    ValueId valueId(value.id());
    ValueId parent(valueId.parent());
    ValueId leaf(valueId.leaf());

    if (value.type() == CompanEdgeProtocol::Struct) return;

    strm << std::endl;
    strm << Indent << "// " << valueId << std::endl;

    if (value.type() == CompanEdgeProtocol::Container) {
        ValueId containerId(valueId);
        if (valueId.leaf() == DmoContainer::MetaContainerDelimId) containerId = valueId.parent();

        if (valueIdVariable_.find(containerId) == valueIdVariable_.end()) {
            ErrorLog(MetaGeneratorClassImplLog) << containerId << " not declared" << std::endl;
            return;
        }

        strm << Indent << "dmo.insertMetaData(" << valueIdVariable_[containerId] << ", ";

        makeCreateValueWrapper(containerId, value.type(), strm);
        strm << ");" << std::endl;

        return;
    }

    if (valueIdVariable_.find(valueId) == valueIdVariable_.end()) {
        ErrorLog(MetaGeneratorClassImplLog) << valueId << " not declared" << std::endl;
        return;
    }

    std::string valueIdName(valueIdVariable_[valueId]);

    std::string memberVariableName(makeVariableName(valueId) + "Value");
    strm << Indent << "CompanEdgeProtocol::Value " << memberVariableName << ";" << std::endl;

    if (parent.leaf() == DmoContainer::MetaContainerDelimId) {
        if (valueIdVariable_.find(parent) == valueIdVariable_.end()) {

            ErrorLog(MetaGeneratorClassImplLog) << parent << " not declared" << std::endl;
            return;
        }

        if (dmo_.getMetaDataType(parent) == CompanEdgeProtocol::Container) {
            strm << Indent << "// double container type" << std::endl;
            strm << Indent << "dmo.setMetaDataType(" << valueIdVariable_[parent] << ", CompanEdgeProtocol::Container);"
                 << std::endl
                 << std::endl;
        }
    }

    strm << Indent << memberVariableName << ".set_id(" << valueIdName << ");" << std::endl;

    if (value.type() == CompanEdgeProtocol::Enum) {
        std::string enumVector = createEnumeratorVector(value, strm);

        strm << Indent << "valueInit(" << memberVariableName << ", " << enumVector << ");" << std::endl;

        strm << Indent << "valueSet<std::string>(" << memberVariableName << ", "
             << quoteString(ValueEnumerators::toString(value)) << ");" << std::endl;

        strm << Indent << "dmo.insertMetaData(" << valueIdName << ", " << memberVariableName << ");" << std::endl;
        return;
    }

    strm << Indent << "dmo.insertMetaData(" << valueIdName << ", ";
    makeCreateValueWrapper(valueId, value.type(), strm);
    strm << ");" << std::endl;

    //    CompanEdgeProtocol::Value_Type_Name(value.type()) << std::endl;
}

std::string MetaGeneratorClassImpl::createEnumeratorVector(CompanEdgeProtocol::Value const& value, std::ostream& strm)
{
    std::string enumSetName(makeNormalizedName(makeVariableName(value.id())) + "Enums");

    strm << Indent << "std::set<std::pair<uint32_t, std::string>> " << enumSetName << "({" << std::endl;

    for (auto& enumerator : value.enumvalue().enumerators()) {
        strm << Indent << Indent << "{" << enumerator.value() << ", " << quoteString(enumerator.text()) << "},"
             << std::endl;
    }
    strm << Indent << "});" << std::endl;

    return enumSetName;
}

std::string MetaGeneratorClassImpl::makeVariableName(ValueId const& parentId)
{
    std::ostringstream classStr;

    bool first(true);
    for (auto& valueId : parentId) {
        if (first) {
            classStr << valueId;
            first = false;
            continue;
        }

        if (valueId == DmoContainer::MetaContainerDelimId) {
            classStr << "_";
            continue;
        }

        classStr << camelCase(valueId);
    }

    return classStr.str();
}

std::string MetaGeneratorClassImpl::makeValueIdName(ValueId const& valueId)
{
    return makeVariableName(valueId) + std::string("Id");
}
