/**
 Copyright © 2024 COMPAN REF
 @file Compan_dmo_meta_generator_class_def.cpp
 @brief -*-*-*-*-*-
 */
#include "Compan_dmo_meta_generator_class_def.h"

#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger MetaGeneratorClassDefLog("generator.classdef", LogLevel::Information);

MetaGeneratorClassDef::MetaGeneratorClassDef(DmoContainer& dmo)
    : ValueIdGenerator(dmo, "MetaData")
{
}

bool MetaGeneratorClassDef::doPreGeneration()
{
    if (!ValueIdGenerator::doPreGeneration()) return false;

    includePaths_.push_back("company_ref_variant_valuestore/company_ref_variant_valuestore.h");
    includePaths_.push_back("company_ref_dmo/company_ref_dmo_container.h");
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

void MetaGeneratorClassDef::generateHead(std::ostream& strm)
{
    strm << "#ifndef " << ifndefStr_ << std::endl;
    strm << "#define " << ifndefStr_ << std::endl << std::endl;

    generateIncludes(strm);

    strm << "namespace Compan{" << std::endl;
    strm << "namespace Edge {" << std::endl << std::endl;
}

void MetaGeneratorClassDef::generateFoot(std::ostream& strm)
{
    strm << "} // namespace edge" << std::endl;
    strm << "} // namespace Compan" << std::endl << std::endl;

    strm << "#endif // " << ifndefStr_ << std::endl << std::endl;
}

void MetaGeneratorClassDef::generateBody(std::ostream& strm)
{
    strm << "// " << fileClass_ << std::endl;
    strm << "class " << className() << " {" << std::endl
         << "public:" << std::endl
         << Indent << "~" << className() << "() = default;" << std::endl
         << std::endl
         << Indent << "/// Creation function for VariantValueStore" << std::endl
         << Indent << "static void create(VariantValueStore& ws, DmoContainer& dmoContainer);" << std::endl
         << std::endl
         << Indent << "/// Creation function for MicroServices" << std::endl
         << Indent << "static void create(VariantValueStore& ws);" << std::endl
         << "};" << std::endl
         << std::endl;
}
