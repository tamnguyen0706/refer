/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_dmo_container.cpp
  @brief Test DMO Container
*/

#include "test_dmo_container_mock.h"

#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_stream.h>

TEST_F(DmoContainerTest, AddValues)
{
    addValueDefs();

    // not allowed to insert map children
    CompanEdgeProtocol::Value badValue;
    badValue.set_id("a.b.xyz,c.bool");
    badValue.set_type(CompanEdgeProtocol::Bool);
    EXPECT_FALSE(dmo_.insertValue(badValue));

    ValueDefType valueVisited;
    dmo_.visitValues([&valueVisited](CompanEdgeProtocol::Value const& value) {
        valueVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(valueVisited, resultsValuesVisited_);

    ValueDefType metaVisited;

    dmo_.visitMetaData([&metaVisited](CompanEdgeProtocol::Value const& value) {
        metaVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    ValueDefType expectedMetaDefs({
            std::make_pair("a.b", CompanEdgeProtocol::Container),
            std::make_pair("a.struct", CompanEdgeProtocol::Struct), // completely legal, but not recommended
            std::make_pair("a.struct.container", CompanEdgeProtocol::Container),
    });

    EXPECT_EQ(metaVisited, expectedMetaDefs);
}

TEST_F(DmoContainerTest, AddMetaData)
{
    addMetaDefs();

    ValueDefType metaVisited;

    dmo_.visitMetaData([&metaVisited](CompanEdgeProtocol::Value const& value) {
        metaVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(metaVisited, resultsMetaVisited_);
}

TEST_F(DmoContainerTest, AddMixed)
{
    addValueDefs();
    addMetaDefs();

    ValueDefType valueVisited;
    dmo_.visitValues([&valueVisited](CompanEdgeProtocol::Value const& value) {
        valueVisited.push_back(std::make_pair(value.id(), value.type()));
    });
    EXPECT_EQ(valueVisited, resultsValuesVisited_);

    ValueDefType metaVisited;
    dmo_.visitMetaData([&metaVisited](CompanEdgeProtocol::Value const& value) {
        metaVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(metaVisited, resultsMetaVisited_);
}

TEST_F(DmoContainerTest, AddInterlaced)
{
    addValueDefs();
    addMetaDefs();
    addInstanceDefs();

    ValueDefType valueVisited;
    dmo_.visitValues([&valueVisited](CompanEdgeProtocol::Value const& value) {
        valueVisited.push_back(std::make_pair(value.id(), value.type()));
    });
    EXPECT_EQ(valueVisited, resultsInstanceVisited_);

    ValueDefType metaVisited;
    dmo_.visitMetaData([&metaVisited](CompanEdgeProtocol::Value const& value) {
        metaVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(metaVisited, resultsMetaVisited_);
}

TEST_F(DmoContainerTest, RemoveValues)
{
    addValueDefs();

    for (auto& child : valueDefs_) {

        ValueId element(child.first);
        EXPECT_TRUE(dmo_.removeValue(element));
    }

    EXPECT_TRUE(dmo_.empty());
}

TEST_F(DmoContainerTest, PruneValues)
{
    addValueDefs();
    addMetaDefs();

    // insert a container key value - which must get prune out
    {
        CompanEdgeProtocol::Value vsValue;

        vsValue.set_id("a.b.container1");
        vsValue.set_type(CompanEdgeProtocol::Struct);
        EXPECT_TRUE(dmo_.insertValue(vsValue, false));

        vsValue.set_id("a.b.container1.c");
        vsValue.set_type(CompanEdgeProtocol::Struct);
        EXPECT_TRUE(dmo_.insertValue(vsValue, false));

        vsValue.set_id("a.b.container1.c.bool");
        vsValue.set_type(CompanEdgeProtocol::Bool);
        EXPECT_TRUE(dmo_.insertValue(vsValue, false));
    }

    dmo_.pruneValues();

    EXPECT_FALSE(dmo_.empty());

    ValueDefType valueVisited;
    ValueDefType prunedVisited({
            std::make_pair("a", CompanEdgeProtocol::Struct),
            std::make_pair("a.b", CompanEdgeProtocol::Container),
            std::make_pair("a.struct", CompanEdgeProtocol::Struct),
            std::make_pair("a.struct.container", CompanEdgeProtocol::Container),

    });

    dmo_.visitValues([&valueVisited](CompanEdgeProtocol::Value const& value) {
        valueVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(valueVisited, prunedVisited);

    ValueDefType metaVisited;
    dmo_.visitMetaData([&metaVisited](CompanEdgeProtocol::Value const& value) {
        metaVisited.push_back(std::make_pair(value.id(), value.type()));
    });

    EXPECT_EQ(metaVisited, resultsMetaVisited_);
}

TEST_F(DmoContainerTest, SearchMetaData)
{
    ValueId const mapName("a.b");
    EXPECT_TRUE(dmo_.insertMetaContainer(mapName, CompanEdgeProtocol::Container));

    addMetaDefs();

    DmoContainer::OptionalTreeType metaTree;

    ValueId search1("a.b.0.c.container1");
    metaTree = dmo_.getMetaData(search1);
    EXPECT_TRUE(metaTree);

    ValueId search2("a.b.0.c.container1.1");
    metaTree = dmo_.getMetaData(search2);
    EXPECT_TRUE(metaTree);

    // legal, but not really meta data
    ValueId search3("a.b");
    metaTree = dmo_.getMetaData(search3);
    EXPECT_TRUE(metaTree);

    ValueId search4("a.b.x");
    metaTree = dmo_.getMetaData(search4);
    EXPECT_TRUE(metaTree);

    ValueId search5("a.b.+.c.container1.+.eui48");
    metaTree = dmo_.getMetaData(search5);
    EXPECT_TRUE(metaTree);
}

TEST_F(DmoContainerTest, ReplaceValues)
{
    addValueDefs();

    CompanEdgeProtocol::Value vsValue;
    vsValue.set_id("a.bool");
    vsValue.set_type(CompanEdgeProtocol::Bool);
    vsValue.mutable_boolvalue()->set_value(true);

    EXPECT_TRUE(dmo_.insertValue(vsValue));

    CompanEdgeProtocol::Value chkValue = dmo_.root().get<CompanEdgeProtocol::Value>("a.bool");

    EXPECT_EQ(vsValue.boolvalue().value(), chkValue.boolvalue().value());
}

TEST_F(DmoContainerTest, ValueTypedMetaData)
{
    ValueId const mapName("a.b");
    EXPECT_TRUE(dmo_.insertMetaContainer(mapName, CompanEdgeProtocol::Container, CompanEdgeProtocol::Text));

    CompanEdgeProtocol::Value chkValue = dmo_.root().get<CompanEdgeProtocol::Value>("a.b.+");

    EXPECT_EQ(CompanEdgeProtocol::Text, chkValue.type());
}

TEST_F(DmoContainerTest, GetValueDefinition)
{
    ValueId const mapName("a.b");
    EXPECT_TRUE(dmo_.insertMetaContainer(mapName, CompanEdgeProtocol::Container));

    addMetaDefs();

    CompanEdgeProtocol::Value interval = dmo_.getValueDefinition("a.b.xyz.c.interval");
    EXPECT_EQ(interval.type(), CompanEdgeProtocol::Interval);

    CompanEdgeProtocol::Value eui = dmo_.getValueDefinition("a.b.abc.c.container1.xyz.eui48");
    EXPECT_EQ(eui.type(), CompanEdgeProtocol::EUI48);
}

TEST_F(DmoContainerTest, ContainerOutOfOrderCreation)
{
    ValueId deepName("a.b.+.c.container2.+");
    ValueId shallowName("a.b");

    EXPECT_TRUE(dmo_.insertMetaContainer(deepName, CompanEdgeProtocol::Container));
    EXPECT_TRUE(dmo_.insertMetaContainer(shallowName, CompanEdgeProtocol::Container));
}

TEST_F(DmoContainerTest, ValueOutOfOrderCreation)
{
    CompanEdgeProtocol::Value deepValue;
    valueInit(deepValue, CompanEdgeProtocol::EUI48);
    deepValue.set_id("a.b.+.c.container1.+.eui48");

    CompanEdgeProtocol::Value shallowName;
    valueInit(shallowName, CompanEdgeProtocol::Container);
    shallowName.set_id("a.b.+.c.container1");

    EXPECT_TRUE(dmo_.insertMetaData(deepValue.id(), deepValue));
    EXPECT_TRUE(dmo_.insertMetaData(shallowName.id(), shallowName));
}

TEST_F(DmoContainerTest, MinMaxValidation)
{
    // reported bug by Froi

    CompanEdgeProtocol::Value setVal;

    setVal.set_id("ranged");
    setVal.set_type(CompanEdgeProtocol::Interval);
    setVal.mutable_intervalvalue()->set_value(42);
    setVal.mutable_intervalvalue()->set_min(20);
    setVal.mutable_intervalvalue()->set_max(50);

    EXPECT_TRUE(dmo_.insertValue(setVal));

    CompanEdgeProtocol::Value getVal = dmo_.getValueDefinition("ranged");

    EXPECT_EQ(setVal.id(), getVal.id());
    EXPECT_EQ(setVal.type(), getVal.type());
    EXPECT_EQ(setVal.intervalvalue().value(), getVal.intervalvalue().value());
    EXPECT_EQ(setVal.intervalvalue().min(), getVal.intervalvalue().min());
    EXPECT_EQ(setVal.intervalvalue().max(), getVal.intervalvalue().max());
}

TEST_F(DmoContainerTest, IsInstance)
{
    addValueDefs();
    addMetaDefs();
    addInstanceDefs();

    EXPECT_FALSE(dmo_.isInstance("a"));
    EXPECT_FALSE(dmo_.isInstance("a.b"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c.bool"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c.container1"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c.container1.+.eui48"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c.container2"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c.container2.+"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c.enum"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c.interval"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.c.text"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.s"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.s.bool"));
    EXPECT_FALSE(dmo_.isInstance("a.b.+.s.interval"));
    EXPECT_FALSE(dmo_.isInstance("a.struct"));
    EXPECT_FALSE(dmo_.isInstance("a.struct.container"));
    EXPECT_FALSE(dmo_.isInstance("a.struct.container.+.c"));
    EXPECT_FALSE(dmo_.isInstance("a.struct.container.+.c.bool"));
    EXPECT_FALSE(dmo_.isInstance("a.bool"));
    EXPECT_FALSE(dmo_.isInstance("a.enum"));
    EXPECT_FALSE(dmo_.isInstance("a.struct"));
    EXPECT_FALSE(dmo_.isInstance("a.struct.container"));
    EXPECT_FALSE(dmo_.isInstance("a.struct.enum"));
    EXPECT_FALSE(dmo_.isInstance("x"));
    EXPECT_FALSE(dmo_.isInstance("x.text"));

    EXPECT_TRUE(dmo_.isInstance("a.struct.container.0"));
    EXPECT_TRUE(dmo_.isInstance("a.struct.container.0.c"));
    EXPECT_TRUE(dmo_.isInstance("a.struct.container.0.c.bool"));
    EXPECT_TRUE(dmo_.isInstance("a.struct.container.1"));
    EXPECT_TRUE(dmo_.isInstance("a.struct.container.1.c"));
    EXPECT_TRUE(dmo_.isInstance("a.struct.container.1.c.bool"));
}

TEST_F(DmoContainerTest, IsMetaData)
{
    addValueDefs();
    addMetaDefs();
    addInstanceDefs();

    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c.bool"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c.container1"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c.container1.+.eui48"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c.container2"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c.container2.+"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c.enum"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c.interval"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.c.text"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.s"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.s.bool"));
    EXPECT_TRUE(dmo_.isMetaData("a.b.+.s.interval"));
    EXPECT_TRUE(dmo_.isMetaData("a.struct.container.+.c"));
    EXPECT_TRUE(dmo_.isMetaData("a.struct.container.+.c.bool"));

    EXPECT_FALSE(dmo_.isMetaData("a"));
    EXPECT_FALSE(dmo_.isMetaData("a.b"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.container"));

    EXPECT_FALSE(dmo_.isMetaData("a.bool"));
    EXPECT_FALSE(dmo_.isMetaData("a.enum"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.container"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.container.0"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.container.0.c"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.container.0.c.bool"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.container.1"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.container.1.c"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.container.1.c.bool"));
    EXPECT_FALSE(dmo_.isMetaData("a.struct.enum"));
    EXPECT_FALSE(dmo_.isMetaData("x"));
    EXPECT_FALSE(dmo_.isMetaData("x.text"));
}

TEST_F(DmoContainerTest, GetMetaDataPath)
{
    addValueDefs();
    addMetaDefs();
    addInstanceDefs();

    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c"), ValueId("a.b.+.c"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c.bool"), ValueId("a.b.+.c.bool"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c.container1"), ValueId("a.b.+.c.container1.+"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c.container1.+.eui48"), ValueId("a.b.+.c.container1.+.eui48"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c.container2"), ValueId("a.b.+.c.container2.+"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c.container2.+"), ValueId("a.b.+.c.container2.+"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c.enum"), ValueId("a.b.+.c.enum"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c.interval"), ValueId("a.b.+.c.interval"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.c.text"), ValueId("a.b.+.c.text"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.s"), ValueId("a.b.+.s"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.s.bool"), ValueId("a.b.+.s.bool"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b.+.s.interval"), ValueId("a.b.+.s.interval"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container.+.c"), ValueId("a.struct.container.+.c"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container.+.c.bool"), ValueId("a.struct.container.+.c.bool"));

    EXPECT_EQ(dmo_.getMetaDataPath("a"), ValueId("a"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.b"), ValueId("a.b.+"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct"), ValueId("a.struct"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container"), ValueId("a.struct.container.+"));

    EXPECT_EQ(dmo_.getMetaDataPath("a.bool"), ValueId("a.bool"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.enum"), ValueId("a.enum"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct"), ValueId("a.struct"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container"), ValueId("a.struct.container.+"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container.0"), ValueId("a.struct.container.+"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container.0.c"), ValueId("a.struct.container.+.c"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container.0.c.bool"), ValueId("a.struct.container.+.c.bool"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container.1"), ValueId("a.struct.container.+"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container.1.c"), ValueId("a.struct.container.+.c"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.container.1.c.bool"), ValueId("a.struct.container.+.c.bool"));
    EXPECT_EQ(dmo_.getMetaDataPath("a.struct.enum"), ValueId("a.struct.enum"));
    EXPECT_EQ(dmo_.getMetaDataPath("x"), ValueId("x"));
    EXPECT_EQ(dmo_.getMetaDataPath("x.text"), ValueId("x.text"));
}

TEST_F(DmoContainerTest, SetMetaDataType)
{
    ValueId baseId("x");

    EXPECT_TRUE(dmo_.insertMetaContainer(baseId, CompanEdgeProtocol::Container));
    EXPECT_TRUE(dmo_.setMetaDataType(ValueId(baseId, DmoContainer::MetaContainerDelimId), CompanEdgeProtocol::Container));

    baseId += ValueId(DmoContainer::MetaContainerDelimId, DmoContainer::MetaContainerDelimId);

    CompanEdgeProtocol::Value boolVal;

    boolVal.set_id(baseId.name());
    valueInit(boolVal, CompanEdgeProtocol::Bool);
    EXPECT_TRUE(dmo_.insertMetaData(baseId, boolVal));

    EXPECT_EQ(dmo_.getMetaDataType("x"), CompanEdgeProtocol::Container);
    EXPECT_EQ(dmo_.getMetaDataType("x.+"), CompanEdgeProtocol::Container);
    EXPECT_EQ(dmo_.getMetaDataType("x.+.+"), CompanEdgeProtocol::Bool);
}
