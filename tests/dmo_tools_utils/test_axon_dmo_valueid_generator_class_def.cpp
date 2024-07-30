/**
  Copyright © 2024 COMPAN REF
  @file test_Compan_dmo_valueid_generator_class_def.cpp
  @brief Test ValueId code generator - class definitions
*/

#include "test_dmo_tools_utils_mock.h"

#include <dmo_tools_utils/Compan_dmo_ini_converter.h>
#include <dmo_tools_utils/Compan_dmo_valueid_generator_class_def.h>

TEST_F(DmoToolsUtilsMock, ClassDefGenerateIncludes)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateIncludes(oStrm);

    EXPECT_EQ(
            oStrm.str(),
            "#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>\n"
            "#include <memory>\n"
            "\n");
}

TEST_F(DmoToolsUtilsMock, ClassDefGenerateHeaderFoot)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateHead(oStrm);
    generator.generateFoot(oStrm);

    std::string header = "#ifndef __A_VALUE_IDS_H__\n"
                         "#define __A_VALUE_IDS_H__\n"
                         "\n"
                         "#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>\n"
                         "#include <memory>\n"
                         "\n"
                         "namespace Compan{\n"
                         "namespace Edge {\n"
                         "\n"
                         "class VariantBoolValue;\n"
                         "using VariantBoolValuePtr = std::shared_ptr<VariantBoolValue>;\n"
                         "\n"
                         "class VariantTextValue;\n"
                         "using VariantTextValuePtr = std::shared_ptr<VariantTextValue>;\n"
                         "\n"
                         "class VariantUIntervalValue;\n"
                         "using VariantUIntervalValuePtr = std::shared_ptr<VariantUIntervalValue>;\n"
                         "\n"
                         "class ValueId;\n"
                         "\n"
                         "} // namespace edge\n"
                         "} // namespace Compan\n"
                         "\n"
                         "#endif // __A_VALUE_IDS_H__\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefGenerateBody)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBody(oStrm);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefSingleDepth)
{
    std::stringstream strm(SingleDepth);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header = "// a\n"
                         "class A : public ValueId {\n"
                         "public:\n"
                         "    using Ptr = std::shared_ptr<A>;\n"
                         "    A(VariantValueStore&, ValueId const&);\n"
                         "    ~A() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    VariantBoolValuePtr bool_;\n"
                         "    VariantUIntervalValuePtr interval;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // class A\n\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefMultiDepth)
{
    std::stringstream strm(MultiDepth);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header = "// a\n"
                         "class A : public ValueId {\n"
                         "public:\n"
                         "    using Ptr = std::shared_ptr<A>;\n"
                         "    A(VariantValueStore&, ValueId const&);\n"
                         "    ~A() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    // a.b1\n"
                         "    class B1 : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<B1>;\n"
                         "        B1(VariantValueStore&, ValueId const&);\n"
                         "        ~B1() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        VariantBoolValuePtr bool_;\n"
                         "        VariantUIntervalValuePtr interval;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class B1\n"
                         "\n"
                         "    // a.b2\n"
                         "    class B2 : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<B2>;\n"
                         "        B2(VariantValueStore&, ValueId const&);\n"
                         "        ~B2() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        VariantTextValuePtr text;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class B2\n"
                         "\n"
                         "    B1::Ptr b1;\n"
                         "    B2::Ptr b2;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // class A\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefEnum)
{
    std::stringstream strm(Enums);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header = "// a\n"
                         "class A : public ValueId {\n"
                         "public:\n"
                         "    using Ptr = std::shared_ptr<A>;\n"
                         "    A(VariantValueStore&, ValueId const&);\n"
                         "    ~A() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    enum EnumEnum {\n"
                         "        No = 0,\n"
                         "        Yes = 1,\n"
                         "        Maybe = 2,\n"
                         "    };\n"
                         "\n"
                         "    VariantEnumValuePtr enum_;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // class A\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefEnum2)
{
    std::stringstream strm(Enums);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);

    generator.generateEnumTypes(false);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header = "// a\n"
                         "class A : public ValueId {\n"
                         "public:\n"
                         "    using Ptr = std::shared_ptr<A>;\n"
                         "    A(VariantValueStore&, ValueId const&);\n"
                         "    ~A() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    VariantEnumValuePtr enum_;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // class A\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}
TEST_F(DmoToolsUtilsMock, ClassDefDataDefs)
{
    std::stringstream strm(DataDefs);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header = "// a\n"
                         "class A : public ValueId {\n"
                         "public:\n"
                         "    using Ptr = std::shared_ptr<A>;\n"
                         "    A(VariantValueStore&, ValueId const&);\n"
                         "    ~A() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    // a.b\n"
                         "    class B : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<B>;\n"
                         "        B(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "        ~B() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        VariantBoolValuePtr bool_;\n"
                         "        VariantTextValuePtr text;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class B\n"
                         "\n"
                         "    VariantMapValuePtr b;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // class A\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefDuplicateDataDefs)
{
    std::stringstream strm(DuplicateDataDefs);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header = "// a\n"
                         "class A : public ValueId {\n"
                         "public:\n"
                         "    using Ptr = std::shared_ptr<A>;\n"
                         "    A(VariantValueStore&, ValueId const&);\n"
                         "    ~A() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    // a.l1\n"
                         "    class L1 : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<L1>;\n"
                         "        L1(VariantValueStore&, ValueId const&);\n"
                         "        ~L1() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        // a.l1.c\n"
                         "        class C : public ValueId {\n"
                         "        public:\n"
                         "            using Ptr = std::shared_ptr<C>;\n"
                         "            C(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "            ~C() = default;\n"
                         "\n"
                         "            bool isValid() const;\n"
                         "\n"
                         "            VariantBoolValuePtr bool_;\n"
                         "            VariantTextValuePtr text;\n"
                         "\n"
                         "        private:\n"
                         "            bool isValid_;\n"
                         "        }; // class C\n"
                         "\n"
                         "        VariantMapValuePtr c;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class L1\n"
                         "\n"
                         "    // a.l2\n"
                         "    class L2 : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<L2>;\n"
                         "        L2(VariantValueStore&, ValueId const&);\n"
                         "        ~L2() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        // a.l2.c\n"
                         "        class C : public ValueId {\n"
                         "        public:\n"
                         "            using Ptr = std::shared_ptr<C>;\n"
                         "            C(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "            ~C() = default;\n"
                         "\n"
                         "            bool isValid() const;\n"
                         "\n"
                         "            VariantBoolValuePtr bool_;\n"
                         "            VariantTextValuePtr text;\n"
                         "\n"
                         "        private:\n"
                         "            bool isValid_;\n"
                         "        }; // class C\n"
                         "\n"
                         "        VariantMapValuePtr c;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class L2\n"
                         "\n"
                         "    L1::Ptr l1;\n"
                         "    L2::Ptr l2;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // class A\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefComplexDataDef)
{
    std::stringstream strm(ComplexDataDef);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());

    std::stringstream oStrm;
    generator.generateBody(oStrm);

    std::string header = "// abstract\n"
                         "class AbstractValueIds {\n"
                         "public:\n"
                         "    AbstractValueIds(VariantValueStore&);\n"
                         "    ~AbstractValueIds() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    // abstract.me-you\n"
                         "    class Me_You : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<Me_You>;\n"
                         "        Me_You(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "        ~Me_You() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        // abstract.me-you.nothing\n"
                         "        class Nothing : public ValueId {\n"
                         "        public:\n"
                         "            using Ptr = std::shared_ptr<Nothing>;\n"
                         "            Nothing(VariantValueStore&, ValueId const&);\n"
                         "            ~Nothing() = default;\n"
                         "\n"
                         "            bool isValid() const;\n"
                         "\n"
                         "            enum NothingEnum {\n"
                         "                No = 0,\n"
                         "                Yes = 1,\n"
                         "                Maybe = 2,\n"
                         "            };\n"
                         "\n"
                         "            VariantEnumValuePtr nothing;\n"
                         "\n"
                         "        private:\n"
                         "            bool isValid_;\n"
                         "        }; // class Nothing\n"
                         "\n"
                         "        // abstract.me-you.somthing\n"
                         "        class Somthing : public ValueId {\n"
                         "        public:\n"
                         "            using Ptr = std::shared_ptr<Somthing>;\n"
                         "            Somthing(VariantValueStore&, ValueId const&);\n"
                         "            ~Somthing() = default;\n"
                         "\n"
                         "            bool isValid() const;\n"
                         "\n"
                         "            VariantTextValuePtr message;\n"
                         "\n"
                         "        private:\n"
                         "            bool isValid_;\n"
                         "        }; // class Somthing\n"
                         "\n"
                         "        Nothing::Ptr nothing;\n"
                         "        Somthing::Ptr somthing;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class Me_You\n"
                         "\n"
                         "    VariantIntervalValuePtr count;\n"
                         "    VariantMapValuePtr me_you;\n"
                         "\n"
                         "public:\n"
                         "    static ValueId const BaseValueId;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // AbstractValueIds\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefDataDefNonStruct)
{
    std::stringstream strm(DataDefNonStruct);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateBody(oStrm);

    std::string header = "// a\n"
                         "class AValueIds {\n"
                         "public:\n"
                         "    AValueIds(VariantValueStore&);\n"
                         "    ~AValueIds() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    // a.params\n"
                         "    class Params : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<Params>;\n"
                         "        Params(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "        ~Params() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        VariantTextValuePtr keyValue;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class Params\n"
                         "\n"
                         "    VariantMapValuePtr params;\n"
                         "\n"
                         "public:\n"
                         "    static ValueId const BaseValueId;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // AValueIds\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefDoubleContainerDef)
{
    std::stringstream strm(DoubleContainerDef);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateBody(oStrm);

    std::string header = "// system.log\n"
                         "class SystemLogValueIds {\n"
                         "public:\n"
                         "    SystemLogValueIds(VariantValueStore&);\n"
                         "    ~SystemLogValueIds() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    // system.log.status\n"
                         "    class Status : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<Status>;\n"
                         "        Status(VariantValueStore&, ValueId const&);\n"
                         "        ~Status() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        // system.log.status.logFiles\n"
                         "        class LogFiles : public ValueId {\n"
                         "        public:\n"
                         "            using Ptr = std::shared_ptr<LogFiles>;\n"
                         "            LogFiles(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "            ~LogFiles() = default;\n"
                         "\n"
                         "            bool isValid() const;\n"
                         "\n"
                         "            enum TypeEnum {\n"
                         "                DebugLog = 0,\n"
                         "                Messages = 1,\n"
                         "                Kernel = 2,\n"
                         "                Extra = 3,\n"
                         "            };\n"
                         "\n"
                         "            // system.log.status.logFiles.files\n"
                         "            class Files : public ValueId {\n"
                         "            public:\n"
                         "                using Ptr = std::shared_ptr<Files>;\n"
                         "                Files(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "                ~Files() = default;\n"
                         "\n"
                         "                bool isValid() const;\n"
                         "\n"
                         "                VariantTextValuePtr file;\n"
                         "                VariantUIntervalValuePtr size;\n"
                         "\n"
                         "            private:\n"
                         "                bool isValid_;\n"
                         "            }; // class Files\n"
                         "\n"
                         "            VariantMapValuePtr files;\n"
                         "            VariantTextValuePtr folder;\n"
                         "            VariantTextValuePtr prefix;\n"
                         "            VariantEnumValuePtr type;\n"
                         "\n"
                         "        private:\n"
                         "            bool isValid_;\n"
                         "        }; // class LogFiles\n"
                         "\n"
                         "        VariantMapValuePtr logFiles;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class Status\n"
                         "\n"
                         "    Status::Ptr status;\n"
                         "\n"
                         "public:\n"
                         "    static ValueId const BaseValueId;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // SystemLogValueIds\n"
                         "\n";

    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefConfigDmocDef)
{
    std::stringstream strm(ConfigDmocDef + DataDefNonStruct);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);

    generator.microserviceName(converter.microserviceName());
    generator.className(converter.className());

    EXPECT_TRUE(generator.generate(oStrm));

    std::string header = "#ifndef __SOMETHING_ELSE_ENTIRELY_VALUE_IDS_H__\n"
                         "#define __SOMETHING_ELSE_ENTIRELY_VALUE_IDS_H__\n"
                         "\n"
                         "#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>\n"
                         "#include <memory>\n"
                         "\n"
                         "namespace Compan{\n"
                         "namespace Edge {\n"
                         "\n"
                         "class VariantTextValue;\n"
                         "using VariantTextValuePtr = std::shared_ptr<VariantTextValue>;\n"
                         "\n"
                         "class VariantMapValue;\n"
                         "using VariantMapValuePtr = std::shared_ptr<VariantMapValue>;\n"
                         "\n"
                         "class ValueId;\n"
                         "\n"
                         "// a\n"
                         "class SomethingElseEntirelyValueIds {\n"
                         "public:\n"
                         "    SomethingElseEntirelyValueIds(VariantValueStore&);\n"
                         "    ~SomethingElseEntirelyValueIds() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    // a.params\n"
                         "    class Params : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<Params>;\n"
                         "        Params(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "        ~Params() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        VariantTextValuePtr keyValue;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class Params\n"
                         "\n"
                         "    VariantMapValuePtr params;\n"
                         "\n"
                         "public:\n"
                         "    static ValueId const BaseValueId;\n"
                         "    static std::string const MicroserviceName;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // SomethingElseEntirelyValueIds\n"
                         "\n"
                         "} // namespace edge\n"
                         "} // namespace Compan\n"
                         "\n"
                         "#endif // __SOMETHING_ELSE_ENTIRELY_VALUE_IDS_H__\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefMixedConfigDef)
{
    std::stringstream strm(MixedConfigDef);

    EXPECT_TRUE(cfg_.read(strm));

    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);

    generator.microserviceName(converter.microserviceName());
    generator.className(converter.className());

    EXPECT_TRUE(generator.generate(oStrm));

    std::string const header = "#ifndef __DYNAMIC_DMO_VALUE_IDS_H__\n"
                               "#define __DYNAMIC_DMO_VALUE_IDS_H__\n"
                               "\n"
                               "#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>\n"
                               "#include <memory>\n"
                               "\n"
                               "namespace Compan{\n"
                               "namespace Edge {\n"
                               "\n"
                               "class VariantTextValue;\n"
                               "using VariantTextValuePtr = std::shared_ptr<VariantTextValue>;\n"
                               "\n"
                               "class VariantEnumValue;\n"
                               "using VariantEnumValuePtr = std::shared_ptr<VariantEnumValue>;\n"
                               "\n"
                               "class VariantMapValue;\n"
                               "using VariantMapValuePtr = std::shared_ptr<VariantMapValue>;\n"
                               "\n"
                               "class ValueId;\n"
                               "\n"
                               "// system.ws\n"
                               "class DynamicDmoValueIds {\n"
                               "public:\n"
                               "    DynamicDmoValueIds(VariantValueStore&);\n"
                               "    ~DynamicDmoValueIds() = default;\n"
                               "\n"
                               "    bool isValid() const;\n"
                               "\n"
                               "    // system.ws.dmo\n"
                               "    class Dmo : public ValueId {\n"
                               "    public:\n"
                               "        using Ptr = std::shared_ptr<Dmo>;\n"
                               "        Dmo(VariantValueStore&, ValueId const&,ValueId const&);\n"
                               "        ~Dmo() = default;\n"
                               "\n"
                               "        bool isValid() const;\n"
                               "\n"
                               "        // system.ws.dmo.config\n"
                               "        class Config : public ValueId {\n"
                               "        public:\n"
                               "            using Ptr = std::shared_ptr<Config>;\n"
                               "            Config(VariantValueStore&, ValueId const&);\n"
                               "            ~Config() = default;\n"
                               "\n"
                               "            bool isValid() const;\n"
                               "\n"
                               "            enum ActionEnum {\n"
                               "                None = 0,\n"
                               "                Load = 1,\n"
                               "                Unload = 2,\n"
                               "                OverWrite = 3,\n"
                               "            };\n"
                               "\n"
                               "            VariantEnumValuePtr action;\n"
                               "            VariantTextValuePtr path;\n"
                               "\n"
                               "        private:\n"
                               "            bool isValid_;\n"
                               "        }; // class Config\n"
                               "\n"
                               "        // system.ws.dmo.persisted\n"
                               "        class Persisted : public ValueId {\n"
                               "        public:\n"
                               "            using Ptr = std::shared_ptr<Persisted>;\n"
                               "            Persisted(VariantValueStore&, ValueId const&,ValueId const&);\n"
                               "            ~Persisted() = default;\n"
                               "\n"
                               "            bool isValid() const;\n"
                               "\n"
                               "            // system.ws.dmo.persisted.config\n"
                               "            class Config : public ValueId {\n"
                               "            public:\n"
                               "                using Ptr = std::shared_ptr<Config>;\n"
                               "                Config(VariantValueStore&, ValueId const&);\n"
                               "                ~Config() = default;\n"
                               "\n"
                               "                bool isValid() const;\n"
                               "\n"
                               "                enum ActionEnum {\n"
                               "                    None = 0,\n"
                               "                    Load = 1,\n"
                               "                    Unload = 2,\n"
                               "                    OverWrite = 3,\n"
                               "                };\n"
                               "\n"
                               "                VariantEnumValuePtr action;\n"
                               "                VariantTextValuePtr path;\n"
                               "\n"
                               "            private:\n"
                               "                bool isValid_;\n"
                               "            }; // class Config\n"
                               "\n"
                               "            // system.ws.dmo.persisted.status\n"
                               "            class Status : public ValueId {\n"
                               "            public:\n"
                               "                using Ptr = std::shared_ptr<Status>;\n"
                               "                Status(VariantValueStore&, ValueId const&);\n"
                               "                ~Status() = default;\n"
                               "\n"
                               "                bool isValid() const;\n"
                               "\n"
                               "                enum ActionEnum {\n"
                               "                    None = 0,\n"
                               "                    Complete = 1,\n"
                               "                    Failed = 2,\n"
                               "                    DmoExists = 3,\n"
                               "                    FileNotFound = 4,\n"
                               "                };\n"
                               "\n"
                               "                VariantEnumValuePtr action;\n"
                               "\n"
                               "            private:\n"
                               "                bool isValid_;\n"
                               "            }; // class Status\n"
                               "\n"
                               "            Config::Ptr config;\n"
                               "            Status::Ptr status;\n"
                               "\n"
                               "        private:\n"
                               "            bool isValid_;\n"
                               "        }; // class Persisted\n"
                               "\n"
                               "        // system.ws.dmo.status\n"
                               "        class Status : public ValueId {\n"
                               "        public:\n"
                               "            using Ptr = std::shared_ptr<Status>;\n"
                               "            Status(VariantValueStore&, ValueId const&);\n"
                               "            ~Status() = default;\n"
                               "\n"
                               "            bool isValid() const;\n"
                               "\n"
                               "            enum ActionEnum {\n"
                               "                None = 0,\n"
                               "                Complete = 1,\n"
                               "                Failed = 2,\n"
                               "                DmoExists = 3,\n"
                               "                FileNotFound = 4,\n"
                               "            };\n"
                               "\n"
                               "            VariantEnumValuePtr action;\n"
                               "\n"
                               "        private:\n"
                               "            bool isValid_;\n"
                               "        }; // class Status\n"
                               "\n"
                               "        Config::Ptr config;\n"
                               "        VariantMapValuePtr persisted;\n"
                               "        Status::Ptr status;\n"
                               "\n"
                               "    private:\n"
                               "        bool isValid_;\n"
                               "    }; // class Dmo\n"
                               "\n"
                               "    VariantMapValuePtr dmo;\n"
                               "    VariantTextValuePtr dmoPath;\n"
                               "    VariantTextValuePtr persistPath;\n"
                               "\n"
                               "public:\n"
                               "    static ValueId const BaseValueId;\n"
                               "\n"
                               "private:\n"
                               "    bool isValid_;\n"
                               "}; // DynamicDmoValueIds\n"
                               "\n"
                               "} // namespace edge\n"
                               "} // namespace Compan\n"
                               "\n"
                               "#endif // __DYNAMIC_DMO_VALUE_IDS_H__\n"
                               "\n";

    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefEnumsNonNormal)
{
    std::stringstream strm(EnumsNonNormal);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header = "// a\n"
                         "class A : public ValueId {\n"
                         "public:\n"
                         "    using Ptr = std::shared_ptr<A>;\n"
                         "    A(VariantValueStore&, ValueId const&);\n"
                         "    ~A() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    enum EnumEnum {\n"
                         "        _200Mhz = 0,\n"
                         "        With_Space = 1,\n"
                         "        _1_with_space = 2,\n"
                         "    };\n"
                         "\n"
                         "    VariantEnumValuePtr enum_;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // class A\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}

TEST_F(DmoToolsUtilsMock, ClassDefInstanceDataDef)
{
    std::stringstream strm(InstanceDataDef);

    EXPECT_TRUE(cfg_.read(strm));
    DmoIniConverter converter(dmo_, cfg_);
    converter.doImport();

    ValueId parentId;

    std::ostringstream oStrm;
    ValueIdGeneratorClassDef generator(dmo_);
    EXPECT_TRUE(generator.doPreGeneration());
    generator.generateClass(parentId, dmo_.root().begin()->second, oStrm, 0);

    std::string header = "// system\n"
                         "class System : public ValueId {\n"
                         "public:\n"
                         "    using Ptr = std::shared_ptr<System>;\n"
                         "    System(VariantValueStore&, ValueId const&);\n"
                         "    ~System() = default;\n"
                         "\n"
                         "    bool isValid() const;\n"
                         "\n"
                         "    // system.services\n"
                         "    class Services : public ValueId {\n"
                         "    public:\n"
                         "        using Ptr = std::shared_ptr<Services>;\n"
                         "        Services(VariantValueStore&, ValueId const&);\n"
                         "        ~Services() = default;\n"
                         "\n"
                         "        bool isValid() const;\n"
                         "\n"
                         "        // system.services.tcpServer\n"
                         "        class TcpServer : public ValueId {\n"
                         "        public:\n"
                         "            using Ptr = std::shared_ptr<TcpServer>;\n"
                         "            TcpServer(VariantValueStore&, ValueId const&);\n"
                         "            ~TcpServer() = default;\n"
                         "\n"
                         "            bool isValid() const;\n"
                         "\n"
                         "            // system.services.tcpServer.instances\n"
                         "            class Instances : public ValueId {\n"
                         "            public:\n"
                         "                using Ptr = std::shared_ptr<Instances>;\n"
                         "                Instances(VariantValueStore&, ValueId const&,ValueId const&);\n"
                         "                ~Instances() = default;\n"
                         "\n"
                         "                bool isValid() const;\n"
                         "\n"
                         "                // system.services.tcpServer.instances.config\n"
                         "                class Config : public ValueId {\n"
                         "                public:\n"
                         "                    using Ptr = std::shared_ptr<Config>;\n"
                         "                    Config(VariantValueStore&, ValueId const&);\n"
                         "                    ~Config() = default;\n"
                         "\n"
                         "                    bool isValid() const;\n"
                         "\n"
                         "                    VariantBoolValuePtr enable;\n"
                         "                    VariantUSIntervalValuePtr port;\n"
                         "\n"
                         "                private:\n"
                         "                    bool isValid_;\n"
                         "                }; // class Config\n"
                         "\n"
                         "                // system.services.tcpServer.instances.status\n"
                         "                class Status : public ValueId {\n"
                         "                public:\n"
                         "                    using Ptr = std::shared_ptr<Status>;\n"
                         "                    Status(VariantValueStore&, ValueId const&);\n"
                         "                    ~Status() = default;\n"
                         "\n"
                         "                    bool isValid() const;\n"
                         "\n"
                         "                    VariantBoolValuePtr hasError;\n"
                         "\n"
                         "                private:\n"
                         "                    bool isValid_;\n"
                         "                }; // class Status\n"
                         "\n"
                         "                Config::Ptr config;\n"
                         "                Status::Ptr status;\n"
                         "\n"
                         "            private:\n"
                         "                bool isValid_;\n"
                         "            }; // class Instances\n"
                         "\n"
                         "            VariantMapValuePtr instances;\n"
                         "\n"
                         "        private:\n"
                         "            bool isValid_;\n"
                         "        }; // class TcpServer\n"
                         "\n"
                         "        TcpServer::Ptr tcpServer;\n"
                         "\n"
                         "    private:\n"
                         "        bool isValid_;\n"
                         "    }; // class Services\n"
                         "\n"
                         "    Services::Ptr services;\n"
                         "\n"
                         "private:\n"
                         "    bool isValid_;\n"
                         "}; // class System\n"
                         "\n";
    EXPECT_EQ(oStrm.str(), header);
    //    std::cout << oStrm.str() << std::endl;
}
