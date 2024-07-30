/**
  Copyright © 2023 COMPAN REF
  @file test_company_ref_app_options.cpp
  @brief Test AppOptions parser
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <company_ref_main_apps/company_ref_app_options.h>
#include <Compan_logger/Compan_logger_sink_buffered.h>

#include <sstream>

using namespace Compan::Edge;

class AppOptionsParserTest : public testing::Test {
public:
    AppOptionsParserTest()
        : options_({
                {'a', "a-arg", false, false},
                {'b', "b_arg", true, false},
                {'c', "carg", true, false},
                {'h', "help", false, false},
        })
        , optionsParser_(options_)
        , singleArg_({{"word"}})
        , multiArg_({{"word1"}, {"word2"}})
    {
    }

    virtual ~AppOptionsParserTest()
    {
        EXPECT_TRUE(coutWrapper_.empty());
    }

    AppOptionsParser::Options options_;
    AppOptionsParser optionsParser_;

    std::vector<std::string> singleArg_;
    std::vector<std::string> multiArg_;

    CompanLoggerSinkBuffered coutWrapper_;
};

TEST_F(AppOptionsParserTest, Configuration)
{
    // AppOptionsParser();
    AppOptionsParser ctor1;
    EXPECT_TRUE(ctor1.get('a').argNameLong.empty());

    // AppOptionsParser();
    AppOptionsParser ctor2(options_);
    EXPECT_FALSE(ctor2.get('a').argNameLong.empty());
    EXPECT_FALSE(ctor2.get('b').argNameLong.empty());
    EXPECT_FALSE(ctor2.get('c').argNameLong.empty());
    EXPECT_FALSE(ctor2.get('h').argNameLong.empty());

    // void add(Options const& options);
    AppOptionsParser ctor3;
    ctor3.add(options_);
    EXPECT_FALSE(ctor3.get('a').argNameLong.empty());
    EXPECT_FALSE(ctor3.get('b').argNameLong.empty());
    EXPECT_FALSE(ctor3.get('c').argNameLong.empty());
    EXPECT_FALSE(ctor3.get('h').argNameLong.empty());

    // void add(Option const& option);
    AppOptionsParser ctor4;
    ctor4.add(options_[0]);
    EXPECT_FALSE(ctor4.get('a').argNameLong.empty());
    EXPECT_TRUE(ctor4.get('b').argNameLong.empty());
    EXPECT_TRUE(ctor4.get('c').argNameLong.empty());
    EXPECT_TRUE(ctor4.get('h').argNameLong.empty());

    // Option get(char const& argName);
    // Tested above

    // Option get(std::string const& argName);
    EXPECT_FALSE(optionsParser_.get("a-arg").argNameLong.empty());
    EXPECT_FALSE(optionsParser_.get("b_arg").argNameLong.empty());
    EXPECT_FALSE(optionsParser_.get("carg").argNameLong.empty());
    EXPECT_FALSE(optionsParser_.get("help").argNameLong.empty());
}

TEST_F(AppOptionsParserTest, NoParams)
{
    int argc = 1;
    char const* argv[] = {"app"};

    EXPECT_TRUE(optionsParser_.parse(argc, argv));
}

TEST_F(AppOptionsParserTest, RequiredParam)
{
    int argc = 1;
    char const* argv[] = {"app"};

    AppOptionsParser missingRequired({
            {'a', "a-arg", false, true},
    });

    EXPECT_FALSE(missingRequired.parse(argc, argv));
}

TEST_F(AppOptionsParserTest, RequiredArguments)
{
    int argc = 2;
    char const* argv[] = {"app", "-b"};

    AppOptionsParser missingRequired({
            {'b', "b_arg", true, false},
    });

    EXPECT_FALSE(missingRequired.parse(argc, argv));
}

TEST_F(AppOptionsParserTest, NoParamsShort)
{
    int argc = 2;
    char const* argv[] = {"app", "-a"};

    EXPECT_TRUE(optionsParser_.parse(argc, argv));
    EXPECT_TRUE(optionsParser_.has('a'));
    EXPECT_TRUE(optionsParser_.has("a-arg"));
}

TEST_F(AppOptionsParserTest, NoParamsLong)
{
    int argc = 2;
    char const* argv[] = {"app", "--a-arg"};

    EXPECT_TRUE(optionsParser_.parse(argc, argv));
    EXPECT_TRUE(optionsParser_.has('a'));
    EXPECT_TRUE(optionsParser_.has("a-arg"));
}

TEST_F(AppOptionsParserTest, SingleParamShort)
{
    int argc = 3;
    char const* argv[] = {"app", "-b", "word"};
    EXPECT_TRUE(optionsParser_.parse(argc, argv));
    EXPECT_TRUE(optionsParser_.has('b'));
    EXPECT_TRUE(optionsParser_.has("b_arg"));
    EXPECT_EQ(optionsParser_.single('b'), "word");
    EXPECT_EQ(optionsParser_.single("b_arg"), "word");
    EXPECT_EQ(optionsParser_.multi('b'), std::vector<std::string>({{"word"}}));
    EXPECT_EQ(optionsParser_.multi("b_arg"), std::vector<std::string>({{"word"}}));
}

TEST_F(AppOptionsParserTest, SingleParamLong)
{
    int argc = 2;
    char const* argv[] = {"app", "--b_arg=word"};
    EXPECT_TRUE(optionsParser_.parse(argc, argv));
    EXPECT_TRUE(optionsParser_.has('b'));
    EXPECT_TRUE(optionsParser_.has("b_arg"));
    EXPECT_EQ(optionsParser_.single('b'), "word");
    EXPECT_EQ(optionsParser_.single("b_arg"), "word");
    EXPECT_EQ(optionsParser_.multi('b'), std::vector<std::string>({{"word"}}));
    EXPECT_EQ(optionsParser_.multi("b_arg"), std::vector<std::string>({{"word"}}));
}

TEST_F(AppOptionsParserTest, MultiParamShort)
{
    int argc = 4;
    char const* argv[] = {"app", "-b", "word1", "word2"};
    EXPECT_TRUE(optionsParser_.parse(argc, argv));
    EXPECT_TRUE(optionsParser_.has('b'));
    EXPECT_TRUE(optionsParser_.has("b_arg"));
    EXPECT_EQ(optionsParser_.multi('b'), multiArg_);
    EXPECT_EQ(optionsParser_.multi("b_arg"), multiArg_);
    EXPECT_EQ(optionsParser_.single('b'), "word1");
    EXPECT_EQ(optionsParser_.single("b_arg"), "word1");
}

TEST_F(AppOptionsParserTest, MultiParamLong)
{
    int argc = 4;
    char const* argv[] = {"app", "-b_arg", "word1", "word2"};
    EXPECT_TRUE(optionsParser_.parse(argc, argv));
    EXPECT_TRUE(optionsParser_.has('b'));
    EXPECT_TRUE(optionsParser_.has("b_arg"));
    EXPECT_EQ(optionsParser_.multi('b'), multiArg_);
    EXPECT_EQ(optionsParser_.multi("b_arg"), multiArg_);
    EXPECT_EQ(optionsParser_.single('b'), "word1");
    EXPECT_EQ(optionsParser_.single("b_arg"), "word1");
}

TEST_F(AppOptionsParserTest, MissingParamsShort)
{
    int argc = 2;
    char const* argv[] = {
            "app",
            "-b",
    };

    EXPECT_FALSE(optionsParser_.parse(argc, argv));
    EXPECT_FALSE(optionsParser_.has('b'));
    EXPECT_FALSE(optionsParser_.has("b_arg"));
}

TEST_F(AppOptionsParserTest, MissingParamsLong)
{
    int argc = 2;
    char const* argv[] = {
            "app",
            "--b_arg",
    };

    EXPECT_FALSE(optionsParser_.parse(argc, argv));
    EXPECT_FALSE(optionsParser_.has('b'));
    EXPECT_FALSE(optionsParser_.has("b_arg"));
}

TEST_F(AppOptionsParserTest, UnknownArg)
{
    int argc = 2;
    char const* argv[] = {
            "app",
            "-x",
    };

    EXPECT_FALSE(optionsParser_.parse(argc, argv));
    EXPECT_FALSE(optionsParser_.has('x'));
}

TEST_F(AppOptionsParserTest, MixedParams)
{
    int argc = 6;
    char const* argv[] = {"app", "--a-arg", "-b", "word1", "word2", "-h"};
    EXPECT_TRUE(optionsParser_.parse(argc, argv));
    EXPECT_TRUE(optionsParser_.has('a'));
    EXPECT_TRUE(optionsParser_.has('b'));
    EXPECT_TRUE(optionsParser_.has('h'));

    EXPECT_EQ(optionsParser_.multi('b'), multiArg_);
    EXPECT_EQ(optionsParser_.multi("b_arg"), multiArg_);
    EXPECT_EQ(optionsParser_.single('b'), "word1");
    EXPECT_EQ(optionsParser_.single("b_arg"), "word1");
}

TEST_F(AppOptionsParserTest, MixedParam)
{
    int argc = 5;
    char const* argv[] = {"./dmo_from_ini", "--ini=/tmp/orchestrator.cfg", "-d", "/tmp/app.dmo", "-s"};

    AppOptionsParser appOptionsParser({
            {'h', "help", false, false},
            {'s', "show", false, false},
            {'i', "ini", true, true},
            {'d', "dmo", true, true},
    });

    EXPECT_TRUE(appOptionsParser.parse(argc, argv));
    EXPECT_TRUE(appOptionsParser.has('i'));
    EXPECT_TRUE(appOptionsParser.has('d'));
    EXPECT_TRUE(appOptionsParser.has('s'));

    EXPECT_EQ(appOptionsParser.multi('i'), std::vector<std::string>({{"/tmp/orchestrator.cfg"}}));
    EXPECT_EQ(appOptionsParser.single('d'), "/tmp/app.dmo");
}
