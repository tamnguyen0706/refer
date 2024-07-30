/**
  Copyright © 2024 COMPAN REF
  @file test_Compan_dmo_valueid_generator_class_impl.cpp
  @brief Test ValueId code generator - class implementations
*/

#include "test_dmo_tools_utils_mock.h"

#include <dmo_tools_utils/Compan_dmo_ini_converter.h>
#include <dmo_tools_utils/Compan_dmo_valueid_generator_class_impl.h>

TEST_F(DmoToolsUtilsMock, ClassImpGenerateIncludes)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateIncludes(oStrm);

    EXPECT_EQ(
            oStrm.str(),
            "#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h>\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_bool_value.h>\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>\n"
            "#include <Compan_logger/Compan_logger.h>\n\n");
}

TEST_F(DmoToolsUtilsMock, ClassImplGenerateBaseValueSingleDepth)
{
    std::stringstream strm(SingleDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBaseValueIds(oStrm);

    std::string output = "ValueId const AValueIds::BaseValueId(\"a\");\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), output);
}

TEST_F(DmoToolsUtilsMock, ClassImplGenerateBaseValueWideDepth)
{
    std::stringstream strm(MultiWideDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBaseValueIds(oStrm);

    std::string output = "namespace {\n"
                         "    ValueId const AId(\"a\");\n"
                         "    ValueId const BId(ValueId(AId, \"b\"));\n"
                         "} // namespace\n"
                         "\n"
                         "ValueId const ABCValueIds::BaseValueId(ValueId(BId, \"c\"));\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), output);
}

TEST_F(DmoToolsUtilsMock, ClassImplGenerateBaseValueOverRide)
{
    std::stringstream strm(DataDefs);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    generator.baseId(ValueId("a.b"));

    std::stringstream oStrm;
    generator.generateBaseValueIds(oStrm);

    std::string output = "namespace {\n"
                         "    ValueId const AId(\"a\");\n"
                         "} // namespace\n"
                         "\n"
                         "ValueId const AValueIds::BaseValueId(ValueId(AId, \"b\"));\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), output);

    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplGenerateHeaderFoot)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateHead(oStrm);
    generator.generateFoot(oStrm);

    std::string header = "#include \"a_value_ids.h\"\n"
                         "\n"
                         "#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h>\n"
                         "#include <company_ref_variant_valuestore/company_ref_variant_bool_value.h>\n"
                         "#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>\n"
                         "#include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>\n"
                         "#include <Compan_logger/Compan_logger.h>\n"
                         "\n"
                         "using namespace Compan::Edge;\n"
                         "\n"
                         "CompanLogger AValueIdsLog(\"value_ids.a\");\n"
                         "\n"
                         "ValueId const AValueIds::BaseValueId(\"a\");\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplGenerateBody)
{
    std::stringstream strm(DuplicateDataDefs);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBody(oStrm);

    std::string source =
            "AValueIds::AValueIds(VariantValueStore& ws)\n"
            "    : l1(std::make_shared<L1>(ws, ValueId(BaseValueId, \"l1\")))\n"
            "    , l2(std::make_shared<L2>(ws, ValueId(BaseValueId, \"l2\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!l1 || !l1->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: l1\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!l2 || !l2->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: l2\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AValueIds::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "AValueIds::L1::L1(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , c(ws.get<VariantMapValue>(ValueId(valueId, \"c\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!c) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: c\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AValueIds::L1::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "AValueIds::L1::C::C(VariantValueStore& ws, ValueId const& valueId, ValueId const& instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , bool_(ws.get<VariantBoolValue>(ValueId(ValueId(valueId, instanceValueId), \"bool\")))\n"
            "    , text(ws.get<VariantTextValue>(ValueId(ValueId(valueId, instanceValueId), \"text\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!bool_) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: bool_\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!text) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: text\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AValueIds::L1::C::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "AValueIds::L2::L2(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , c(ws.get<VariantMapValue>(ValueId(valueId, \"c\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!c) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: c\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AValueIds::L2::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "AValueIds::L2::C::C(VariantValueStore& ws, ValueId const& valueId, ValueId const& instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , bool_(ws.get<VariantBoolValue>(ValueId(ValueId(valueId, instanceValueId), \"bool\")))\n"
            "    , text(ws.get<VariantTextValue>(ValueId(ValueId(valueId, instanceValueId), \"text\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!bool_) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: bool_\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!text) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: text\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AValueIds::L2::C::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n";
    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplSingleDepth)
{
    std::stringstream strm(SingleDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string source = "A::A(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , bool_(ws.get<VariantBoolValue>(ValueId(valueId, \"bool\")))\n"
                         "    , interval(ws.get<VariantUIntervalValue>(ValueId(valueId, \"interval\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!bool_) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: bool_\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "    if (!interval) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: interval\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n";
    ;
    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplMultiDepth)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string source = "A::A(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , b1(std::make_shared<B1>(ws, ValueId(valueId, \"b1\")))\n"
                         "    , b2(std::make_shared<B2>(ws, ValueId(valueId, \"b2\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!b1 || !b1->isValid()) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: b1\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "    if (!b2 || !b2->isValid()) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: b2\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n"
                         "A::B1::B1(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , bool_(ws.get<VariantBoolValue>(ValueId(valueId, \"bool\")))\n"
                         "    , interval(ws.get<VariantUIntervalValue>(ValueId(valueId, \"interval\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!bool_) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: bool_\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "    if (!interval) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: interval\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::B1::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n"
                         "A::B2::B2(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , text(ws.get<VariantTextValue>(ValueId(valueId, \"text\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!text) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: text\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::B2::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplEnums)
{
    std::stringstream strm(Enums);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string source = "A::A(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , enum_(ws.get<VariantEnumValue>(ValueId(valueId, \"enum\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!enum_) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: enum_\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplDataDefs)
{
    std::stringstream strm(DataDefs);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string source = "A::A(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , b(ws.get<VariantMapValue>(ValueId(valueId, \"b\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!b) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: b\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n"
                         "A::B::B(VariantValueStore& ws, ValueId const& valueId, ValueId const& instanceValueId)\n"
                         "    : ValueId(valueId, instanceValueId)\n"
                         "    , bool_(ws.get<VariantBoolValue>(ValueId(ValueId(valueId, instanceValueId), \"bool\")))\n"
                         "    , text(ws.get<VariantTextValue>(ValueId(ValueId(valueId, instanceValueId), \"text\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!bool_) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: bool_\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "    if (!text) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog() << \"Invalid pointer: text\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::B::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplDuplicateDataDefs)
{
    std::stringstream strm(DuplicateDataDefs);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string source = "A::A(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , l1(std::make_shared<L1>(ws, ValueId(valueId, \"l1\")))\n"
                         "    , l2(std::make_shared<L2>(ws, ValueId(valueId, \"l2\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!l1 || !l1->isValid()) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog(AValueIdsLog) << \"Invalid pointer: l1\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "    if (!l2 || !l2->isValid()) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog(AValueIdsLog) << \"Invalid pointer: l2\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n"
                         "A::L1::L1(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , c(ws.get<VariantMapValue>(ValueId(valueId, \"c\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!c) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog(AValueIdsLog) << \"Invalid pointer: c\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::L1::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n"
                         "A::L1::C::C(VariantValueStore& ws, ValueId const& valueId, ValueId const& instanceValueId)\n"
                         "    : ValueId(valueId, instanceValueId)\n"
                         "    , bool_(ws.get<VariantBoolValue>(ValueId(ValueId(valueId, instanceValueId), \"bool\")))\n"
                         "    , text(ws.get<VariantTextValue>(ValueId(ValueId(valueId, instanceValueId), \"text\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!bool_) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog(AValueIdsLog) << \"Invalid pointer: bool_\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "    if (!text) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog(AValueIdsLog) << \"Invalid pointer: text\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::L1::C::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n"
                         "A::L2::L2(VariantValueStore& ws, ValueId const& valueId)\n"
                         "    : ValueId(valueId)\n"
                         "    , c(ws.get<VariantMapValue>(ValueId(valueId, \"c\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!c) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog(AValueIdsLog) << \"Invalid pointer: c\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::L2::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n"
                         "A::L2::C::C(VariantValueStore& ws, ValueId const& valueId, ValueId const& instanceValueId)\n"
                         "    : ValueId(valueId, instanceValueId)\n"
                         "    , bool_(ws.get<VariantBoolValue>(ValueId(ValueId(valueId, instanceValueId), \"bool\")))\n"
                         "    , text(ws.get<VariantTextValue>(ValueId(ValueId(valueId, instanceValueId), \"text\")))\n"
                         "    , isValid_(true)\n"
                         "{\n"
                         "    if (!bool_) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog(AValueIdsLog) << \"Invalid pointer: bool_\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "    if (!text) {\n"
                         "        isValid_=false;\n"
                         "        ErrorLog(AValueIdsLog) << \"Invalid pointer: text\" << std::endl;\n"
                         "        return;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "bool A::L2::C::isValid() const {\n"
                         "    return isValid_;\n"
                         "}\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplComplexDataDef)
{
    std::stringstream strm(ComplexDataDef);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBody(oStrm);

    std::string source =
            "AbstractValueIds::AbstractValueIds(VariantValueStore& ws)\n"
            "    : count(ws.get<VariantIntervalValue>(ValueId(BaseValueId, \"Count\")))\n"
            "    , me_you(ws.get<VariantMapValue>(ValueId(BaseValueId, \"me-you\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!count) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AbstractValueIdsLog) << \"Invalid pointer: count\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!me_you) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AbstractValueIdsLog) << \"Invalid pointer: me_you\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AbstractValueIds::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "AbstractValueIds::Me_You::Me_You(VariantValueStore& ws, ValueId const& valueId, ValueId const& "
            "instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , nothing(std::make_shared<Nothing>(ws, ValueId(ValueId(valueId, instanceValueId), \"nothing\")))\n"
            "    , somthing(std::make_shared<Somthing>(ws, ValueId(ValueId(valueId, instanceValueId), \"somthing\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!nothing || !nothing->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AbstractValueIdsLog) << \"Invalid pointer: nothing\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!somthing || !somthing->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AbstractValueIdsLog) << \"Invalid pointer: somthing\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AbstractValueIds::Me_You::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "AbstractValueIds::Me_You::Nothing::Nothing(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , nothing(ws.get<VariantEnumValue>(ValueId(valueId, \"nothing\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!nothing) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AbstractValueIdsLog) << \"Invalid pointer: nothing\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AbstractValueIds::Me_You::Nothing::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "AbstractValueIds::Me_You::Somthing::Somthing(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , message(ws.get<VariantTextValue>(ValueId(valueId, \"message\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!message) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AbstractValueIdsLog) << \"Invalid pointer: message\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AbstractValueIds::Me_You::Somthing::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n";
    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplDataDefNonStruct)
{
    std::stringstream strm(DataDefNonStruct);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateBody(oStrm);

    std::string source =
            "AValueIds::AValueIds(VariantValueStore& ws)\n"
            "    : params(ws.get<VariantMapValue>(ValueId(BaseValueId, \"params\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!params) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: params\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AValueIds::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "AValueIds::Params::Params(VariantValueStore& ws, ValueId const& valueId, ValueId const& instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , keyValue(ws.get<VariantTextValue>(ValueId(valueId, instanceValueId)))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!keyValue) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(AValueIdsLog) << \"Invalid pointer: keyValue\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool AValueIds::Params::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n";
    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplDoubleContainerDef)
{
    std::stringstream strm(DoubleContainerDef);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateBody(oStrm);

    std::string header =
            "SystemLogValueIds::SystemLogValueIds(VariantValueStore& ws)\n"
            "    : status(std::make_shared<Status>(ws, ValueId(BaseValueId, \"status\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!status || !status->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemLogValueIdsLog) << \"Invalid pointer: status\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool SystemLogValueIds::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "SystemLogValueIds::Status::Status(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , logFiles(ws.get<VariantMapValue>(ValueId(valueId, \"logFiles\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!logFiles) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemLogValueIdsLog) << \"Invalid pointer: logFiles\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool SystemLogValueIds::Status::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "SystemLogValueIds::Status::LogFiles::LogFiles(VariantValueStore& ws, ValueId const& valueId, ValueId "
            "const& instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , files(ws.get<VariantMapValue>(ValueId(ValueId(valueId, instanceValueId), \"files\")))\n"
            "    , folder(ws.get<VariantTextValue>(ValueId(ValueId(valueId, instanceValueId), \"folder\")))\n"
            "    , prefix(ws.get<VariantTextValue>(ValueId(ValueId(valueId, instanceValueId), \"prefix\")))\n"
            "    , type(ws.get<VariantEnumValue>(ValueId(ValueId(valueId, instanceValueId), \"type\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!files) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemLogValueIdsLog) << \"Invalid pointer: files\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!folder) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemLogValueIdsLog) << \"Invalid pointer: folder\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!prefix) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemLogValueIdsLog) << \"Invalid pointer: prefix\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!type) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemLogValueIdsLog) << \"Invalid pointer: type\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool SystemLogValueIds::Status::LogFiles::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "SystemLogValueIds::Status::LogFiles::Files::Files(VariantValueStore& ws, ValueId const& valueId, ValueId "
            "const& instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , file(ws.get<VariantTextValue>(ValueId(ValueId(valueId, instanceValueId), \"file\")))\n"
            "    , size(ws.get<VariantUIntervalValue>(ValueId(ValueId(valueId, instanceValueId), \"size\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!file) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemLogValueIdsLog) << \"Invalid pointer: file\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!size) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemLogValueIdsLog) << \"Invalid pointer: size\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool SystemLogValueIds::Status::LogFiles::Files::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplConfigDmocDef)
{
    std::stringstream strm(ConfigDmocDef + DataDefNonStruct);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);

    generator.microserviceName(converter.microserviceName());
    generator.className(converter.className());

    EXPECT_TRUE(generator.generate(oStrm));

    std::string source =
            "#include \"something_else_entirely_value_ids.h\"\n"
            "\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h>\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>\n"
            "#include <Compan_logger/Compan_logger.h>\n"
            "\n"
            "using namespace Compan::Edge;\n"
            "\n"
            "CompanLogger SomethingElseEntirelyValueIdsLog(\"value_ids.a\");\n"
            "\n"
            "ValueId const SomethingElseEntirelyValueIds::BaseValueId(\"a\");\n"
            "std::string const SomethingElseEntirelyValueIds::MicroserviceName(\"Something\");\n"
            "\n"
            "SomethingElseEntirelyValueIds::SomethingElseEntirelyValueIds(VariantValueStore& ws)\n"
            "    : params(ws.get<VariantMapValue>(ValueId(BaseValueId, \"params\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!params) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SomethingElseEntirelyValueIdsLog) << \"Invalid pointer: params\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool SomethingElseEntirelyValueIds::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "SomethingElseEntirelyValueIds::Params::Params(VariantValueStore& ws, ValueId const& valueId, ValueId "
            "const& instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , keyValue(ws.get<VariantTextValue>(ValueId(valueId, instanceValueId)))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!keyValue) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SomethingElseEntirelyValueIdsLog) << \"Invalid pointer: keyValue\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool SomethingElseEntirelyValueIds::Params::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n";

    EXPECT_EQ(oStrm.str(), source);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplMixedConfigDef)
{
    std::stringstream strm(MixedConfigDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);

    generator.microserviceName(converter.microserviceName());
    generator.className(converter.className());

    EXPECT_TRUE(generator.generate(oStrm));

    std::string const header =
            "#include \"dynamic_dmo_value_ids.h\"\n"
            "\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valueid.h>\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_enum_value.h>\n"
            "#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>\n"
            "#include <Compan_logger/Compan_logger.h>\n"
            "\n"
            "using namespace Compan::Edge;\n"
            "\n"
            "CompanLogger DynamicDmoValueIdsLog(\"value_ids.system.ws\");\n"
            "\n"
            "namespace {\n"
            "    ValueId const SystemId(\"system\");\n"
            "} // namespace\n"
            "\n"
            "ValueId const DynamicDmoValueIds::BaseValueId(ValueId(SystemId, \"ws\"));\n"
            "\n"
            "DynamicDmoValueIds::DynamicDmoValueIds(VariantValueStore& ws)\n"
            "    : dmo(ws.get<VariantMapValue>(ValueId(BaseValueId, \"dmo\")))\n"
            "    , dmoPath(ws.get<VariantTextValue>(ValueId(BaseValueId, \"dmoPath\")))\n"
            "    , persistPath(ws.get<VariantTextValue>(ValueId(BaseValueId, \"persistPath\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!dmo) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: dmo\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!dmoPath) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: dmoPath\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!persistPath) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: persistPath\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool DynamicDmoValueIds::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "DynamicDmoValueIds::Dmo::Dmo(VariantValueStore& ws, ValueId const& valueId, ValueId const& "
            "instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , config(std::make_shared<Config>(ws, ValueId(ValueId(valueId, instanceValueId), \"config\")))\n"
            "    , persisted(ws.get<VariantMapValue>(ValueId(ValueId(valueId, instanceValueId), \"persisted\")))\n"
            "    , status(std::make_shared<Status>(ws, ValueId(ValueId(valueId, instanceValueId), \"status\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!config || !config->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: config\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!persisted) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: persisted\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!status || !status->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: status\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool DynamicDmoValueIds::Dmo::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "DynamicDmoValueIds::Dmo::Config::Config(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , action(ws.get<VariantEnumValue>(ValueId(valueId, \"action\")))\n"
            "    , path(ws.get<VariantTextValue>(ValueId(valueId, \"path\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!action) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: action\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!path) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: path\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool DynamicDmoValueIds::Dmo::Config::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "DynamicDmoValueIds::Dmo::Persisted::Persisted(VariantValueStore& ws, ValueId const& valueId, ValueId "
            "const& instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , config(std::make_shared<Config>(ws, ValueId(ValueId(valueId, instanceValueId), \"config\")))\n"
            "    , status(std::make_shared<Status>(ws, ValueId(ValueId(valueId, instanceValueId), \"status\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!config || !config->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: config\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!status || !status->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: status\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool DynamicDmoValueIds::Dmo::Persisted::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "DynamicDmoValueIds::Dmo::Persisted::Config::Config(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , action(ws.get<VariantEnumValue>(ValueId(valueId, \"action\")))\n"
            "    , path(ws.get<VariantTextValue>(ValueId(valueId, \"path\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!action) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: action\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!path) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: path\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool DynamicDmoValueIds::Dmo::Persisted::Config::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "DynamicDmoValueIds::Dmo::Persisted::Status::Status(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , action(ws.get<VariantEnumValue>(ValueId(valueId, \"action\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!action) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: action\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool DynamicDmoValueIds::Dmo::Persisted::Status::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "DynamicDmoValueIds::Dmo::Status::Status(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , action(ws.get<VariantEnumValue>(ValueId(valueId, \"action\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!action) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(DynamicDmoValueIdsLog) << \"Invalid pointer: action\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool DynamicDmoValueIds::Dmo::Status::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n";

    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassImplInstanceDataDef)
{
    std::stringstream strm(InstanceDataDef);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassImpl generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header =
            "System::System(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , services(std::make_shared<Services>(ws, ValueId(valueId, \"services\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!services || !services->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemServicesValueIdsLog) << \"Invalid pointer: services\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool System::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "System::Services::Services(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , tcpServer(std::make_shared<TcpServer>(ws, ValueId(valueId, \"tcpServer\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!tcpServer || !tcpServer->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemServicesValueIdsLog) << \"Invalid pointer: tcpServer\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool System::Services::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "System::Services::TcpServer::TcpServer(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , instances(ws.get<VariantMapValue>(ValueId(valueId, \"instances\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!instances) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemServicesValueIdsLog) << \"Invalid pointer: instances\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool System::Services::TcpServer::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "System::Services::TcpServer::Instances::Instances(VariantValueStore& ws, ValueId const& valueId, ValueId "
            "const& instanceValueId)\n"
            "    : ValueId(valueId, instanceValueId)\n"
            "    , config(std::make_shared<Config>(ws, ValueId(ValueId(valueId, instanceValueId), \"config\")))\n"
            "    , status(std::make_shared<Status>(ws, ValueId(ValueId(valueId, instanceValueId), \"status\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!config || !config->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemServicesValueIdsLog) << \"Invalid pointer: config\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!status || !status->isValid()) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemServicesValueIdsLog) << \"Invalid pointer: status\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool System::Services::TcpServer::Instances::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "System::Services::TcpServer::Instances::Config::Config(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , enable(ws.get<VariantBoolValue>(ValueId(valueId, \"enable\")))\n"
            "    , port(ws.get<VariantUSIntervalValue>(ValueId(valueId, \"port\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!enable) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemServicesValueIdsLog) << \"Invalid pointer: enable\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "    if (!port) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemServicesValueIdsLog) << \"Invalid pointer: port\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool System::Services::TcpServer::Instances::Config::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n"
            "System::Services::TcpServer::Instances::Status::Status(VariantValueStore& ws, ValueId const& valueId)\n"
            "    : ValueId(valueId)\n"
            "    , hasError(ws.get<VariantBoolValue>(ValueId(valueId, \"hasError\")))\n"
            "    , isValid_(true)\n"
            "{\n"
            "    if (!hasError) {\n"
            "        isValid_=false;\n"
            "        ErrorLog(SystemServicesValueIdsLog) << \"Invalid pointer: hasError\" << std::endl;\n"
            "        return;\n"
            "    }\n"
            "}\n"
            "\n"
            "bool System::Services::TcpServer::Instances::Status::isValid() const {\n"
            "    return isValid_;\n"
            "}\n"
            "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}
