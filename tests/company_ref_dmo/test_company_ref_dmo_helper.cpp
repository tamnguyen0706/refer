/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_dmo_helper.cpp
  @brief Test the DMO Helper class
*/

#include "test_dmo_container_mock.h"

#include "company_ref_dmo_helper.h"

#include "company_ref_dmo/company_ref_dmo_container.h"
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <Compan_logger/Compan_logger_sink_buffered.h>

using namespace Compan::Edge;
using namespace CompanEdgeProtocol;

class DmoHelperTest : public DmoContainerTest {
public:
    DmoHelperTest()
        : DmoContainerTest()
        , ws_(ctx_)
        , helper_(dmo_, ws_)
    {
        addMetaDefs();

        dmo_.visitValues([this](CompanEdgeProtocol::Value const& value) { EXPECT_TRUE(ws_.set(value)); });
    }

    virtual ~DmoHelperTest()
    {
    }

    void testOutput()
    {
        dmo_.print(std::cout);
        std::cout << "VariantValueStore" << std::endl;
        ws_.visitValues([](VariantValue::Ptr const& valuePtr) {
            std::cout << valuePtr->id() << " = " << valuePtr->str() << std::endl;
        });

        std::cout << std::endl;
    }

    boost::asio::io_context ctx_;
    VariantValueStore ws_;
    DmoValueStoreHelper helper_;
};

TEST_F(DmoHelperTest, InsertSingleKey)
{
    helper_.insertChild("a.b", "xyz", "");

    std::vector<ValueId> hasList(
            {"a",
             "a.b",
             "a.b.xyz",
             "a.b.xyz.c",
             "a.b.xyz.c.bool",
             "a.b.xyz.c.container1",
             "a.b.xyz.c.container2",
             "a.b.xyz.c.interval",
             "a.b.xyz.c.text",
             "a.b.xyz.s",
             "a.b.xyz.s.bool",
             "a.b.xyz.s.interval"});

    for (auto& valueId : hasList) EXPECT_TRUE(ws_.has(valueId));

    // Inserting into a Container of Text values
    EXPECT_TRUE(helper_.insertChild("a.b.xyz.c.container2", "key1"));

    ASSERT_TRUE(ws_.has("a.b.xyz.c.container2.key1"));
    ASSERT_EQ(ws_.get("a.b.xyz.c.container2.key1")->type(), CompanEdgeProtocol::Text);
}

TEST_F(DmoHelperTest, InsertEmbeddedKey)
{
    const ValueId v1_id("a.b.xyz.c.text");
    const std::string v1_data("Compan");
    const ValueId v2_id("a.b.xyz.c.container2.key1");
    const std::string v2_data("network");

    CompanEdgeProtocol::Value v1;
    v1.set_id(v1_id);
    v1.set_type(CompanEdgeProtocol::Text);
    v1.mutable_textvalue()->set_value(v1_data);

    CompanEdgeProtocol::Value v2;
    v2.set_id(v2_id);
    v2.set_type(CompanEdgeProtocol::Text);
    v2.mutable_textvalue()->set_value(v2_data);

    // Insert a value embedded in a container context
    EXPECT_TRUE(helper_.insertValue(v2));
    ASSERT_TRUE(ws_.has(v2_id));
    ASSERT_EQ(ws_.get(v2_id)->type(), CompanEdgeProtocol::Text);
    ASSERT_EQ(ws_.get(v2_id)->str(), v2_data);

    EXPECT_TRUE(helper_.insertValue(v1));
    ASSERT_TRUE(ws_.has(v1_id));
    ASSERT_EQ(ws_.get(v1_id)->type(), CompanEdgeProtocol::Text);
    ASSERT_EQ(ws_.get(v1_id)->str(), v1_data);
}

TEST_F(DmoHelperTest, InsertWithValueInfo)
{
    helper_.insertChild("a.b", "xyz", "{c.text=pass}");
    ASSERT_EQ(ws_.get("a.b.xyz.c.text")->str(), "pass");

    helper_.insertChild("a.b", "abc", "{c.test=fail}");
    EXPECT_EQ(coutWrapper_.pop(), "dmo.metacreator: Error: Value not found: a.b.abc.c.test\n");

    ASSERT_TRUE(ws_.get("a.b.abc.c.text")->str().empty());
}

#include <company_ref_variant_valuestore/company_ref_variant_valuestore_visitor.h>

TEST_F(DmoHelperTest, LoggerTest)
{
    dmo_.insertMetaContainer("logger", CompanEdgeProtocol::Container);

    CompanEdgeProtocol::Value enumValue;

    for (auto& iter : std::set<std::pair<uint32_t, std::string>>({{0, "No"}, {1, "Yes"}, {2, "Maybe"}})) {

        CompanValueTypes::EnumValue::Enumerator enumerator;
        enumerator.set_value(static_cast<int32_t>(iter.first));
        enumerator.set_text(iter.second);
        *enumValue.mutable_enumvalue()->add_enumerators() = std::move(enumerator);
    }

    enumValue.set_type(CompanEdgeProtocol::Enum);
    dmo_.insertMetaContainer("logger", CompanEdgeProtocol::Container, CompanEdgeProtocol::Container);
    dmo_.insertMetaData("logger.+.+", enumValue);

    {
        CompanEdgeProtocol::Value v;

        v.set_id("logger");
        v.set_type(CompanEdgeProtocol::Container);

        ws_.set(v);
    }

    ASSERT_FALSE(ws_.has("logger.applet"));
    EXPECT_TRUE(helper_.insertChild("logger", "applet"));

    ASSERT_TRUE(ws_.has("logger.applet"));
    ASSERT_EQ(ws_.get("logger.applet")->type(), CompanEdgeProtocol::Container);

    EXPECT_TRUE(helper_.insertChild("logger.applet", "me"));
    ASSERT_TRUE(ws_.has("logger.applet.me"));
    ASSERT_EQ(ws_.get("logger.applet.me")->type(), CompanEdgeProtocol::Enum);

    EXPECT_FALSE(helper_.insertChild("logger.applet.me", "you"));

    EXPECT_EQ(coutWrapper_.pop(), "dmo.metacreator: Error: Not a container: logger.applet.me\n");
}
