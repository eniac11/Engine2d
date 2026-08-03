#include <gtest/gtest.h>
#include <engine/elog.h>
#include <print>
#include <string>

using namespace elog;
using namespace std::string_literals;


namespace elog {
    // Must be in the same namespace as the enum
    extern void PrintTo(const MessageType& type, std::ostream* os);

    void PrintTo(LogRule const& rule, std::ostream* os) {
        *os << "LogRule('" <<  rule.rule() << "', ";
        PrintTo(rule.type(), os);
        *os << ", " << std::boolalpha << rule.enabled() << ")";;
    }
}

template<>
struct std::formatter<MessageType> {
    template <typename ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        auto it = ctx.begin();
        if (it == ctx.end())
            return it;
    }

    template<class FmtContext>
    FmtContext::iterator format(MessageType s, FmtContext& ctx) const
    {
        std::ostringstream out;
        elog::PrintTo(s, &out);

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

template<>
struct std::formatter<LogRule> {
    template <typename ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        auto it = ctx.begin();
        if (it == ctx.end())
            return it;
    }

    template<class FmtContext>
    FmtContext::iterator format(LogRule s, FmtContext& ctx) const
    {
        std::ostringstream out;
        elog::PrintTo(s, &out);

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

GTEST_TEST(LogRegistryTest, SimpleFilter) {
    LogRuleRegistry registry;
    registry.installFilter("main{debug}=true");
    LogRule const& rule = registry.rules()[0];
    ASSERT_TRUE(rule.enabled());
    EXPECT_EQ(rule.rule(), std::string("main"));
    EXPECT_EQ(rule.type(), MessageType::DebugMsg);
}

GTEST_TEST(LogRegistryTest, SimpleFilterWildcard) {
    LogRuleRegistry registry;
    registry.installFilter("main.*{debug}=true");
    LogRule const& rule = registry.rules()[0];
    ASSERT_TRUE(rule.enabled());
    EXPECT_EQ(rule.rule(), std::string("main.*"));
    EXPECT_EQ(rule.type(), MessageType::DebugMsg);
}

GTEST_TEST(LogRegistryTest, MulitpleFiltersSimpleFilter) {
    LogRuleRegistry registry;
    registry.installFilter("main{debug}=true:default{debug}=true");
    std::vector<LogRule> const& rules = registry.rules();
    ASSERT_EQ(rules.size(), 2);
    LogRule const& main = rules[0];
    EXPECT_TRUE(main.enabled());
    EXPECT_EQ(main.rule(), std::string("main"));
    EXPECT_EQ(main.type(), MessageType::DebugMsg);
    LogRule const& default1 = rules[1];
    ASSERT_TRUE(default1.enabled());
    EXPECT_EQ(default1.rule(), std::string("default"));
    EXPECT_EQ(default1.type(), MessageType::DebugMsg);
}

GTEST_TEST(LogRegistryTest, FilterMultipleLogLevels) {
    LogRuleRegistry registry;
    registry.installFilter("main{debug,info}=true");
    std::vector<LogRule> const& rules = registry.rules();
    EXPECT_EQ(rules.size(), 2);
    LogRule const& rule = rules[0];
    ASSERT_TRUE(rule.enabled());
    EXPECT_EQ(rule.rule(), std::string("main"));
    EXPECT_EQ(rule.type(), MessageType::DebugMsg);
    LogRule const& rule2 = rules[1];
    ASSERT_TRUE(rule2.enabled());
    EXPECT_EQ(rule2.rule(), std::string("main"));
    EXPECT_EQ(rule2.type(), MessageType::InfoMsg);
}

GTEST_TEST(LogRegistryTest, MultipleFiltersMultipleLogLevels) {
    LogRuleRegistry registry;
    registry.installFilter("main{debug,info}=true:default{debug,info}=true");
    std::vector<LogRule> const& rules = registry.rules();
    EXPECT_EQ(rules.size(), 4);
    {
        LogRule const& rule = rules[0];
        ASSERT_TRUE(rule.enabled());
        EXPECT_EQ(rule.rule(), std::string("main"));
        EXPECT_EQ(rule.type(), MessageType::DebugMsg);

        LogRule const& rule2 = rules[1];
        ASSERT_TRUE(rule2.enabled());
        EXPECT_EQ(rule2.rule(), std::string("main"));
        EXPECT_EQ(rule2.type(), MessageType::InfoMsg);
    }
    {
        LogRule const& rule = rules[2];
        ASSERT_TRUE(rule.enabled());
        EXPECT_EQ(rule.rule(), std::string("default"));
        EXPECT_EQ(rule.type(), MessageType::DebugMsg);

        LogRule const& rule2 = rules[3];
        ASSERT_TRUE(rule2.enabled());
        EXPECT_EQ(rule2.rule(), std::string("default"));
        EXPECT_EQ(rule2.type(), MessageType::InfoMsg);
    }
}

GTEST_TEST(LogRegistryTest, PassSimpleFilter) {
    LogRuleRegistry registry;
    registry.installFilter("main{debug}=true");

    EXPECT_EQ(registry.pass("main"s, DebugMsg), LogRule::CheckStatus::Pass);
}

GTEST_TEST(LogRegistryTest, NoPassSimpleFilter) {
    LogRuleRegistry registry;
    registry.installFilter("main{debug}=false");

    EXPECT_EQ(registry.pass("main"s, DebugMsg), LogRule::CheckStatus::NoPass);
}

GTEST_TEST(LogRegistryTest, PassSimpleFilterWildcard) {
    LogRuleRegistry registry;
    registry.installFilter("main.*{debug}=true");

    EXPECT_EQ(registry.pass("main.network"s, DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(registry.pass("default.network"s, DebugMsg), LogRule::CheckStatus::Ignore);
}

GTEST_TEST(LogRegistryTest, PassMultipleFiltersWildcard) {
    LogRuleRegistry registry;
    registry.installFilter("main.*{debug}=true:default.*{debug}=true");

    EXPECT_EQ(registry.pass("main.network"s, DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(registry.pass("default.network"s, DebugMsg), LogRule::CheckStatus::Pass);
}

GTEST_TEST(LogRegistryTest, PassAndNoPassMultipleFiltersWildcard) {
    LogRuleRegistry registry;
    registry.installFilter("main.*{debug}=true:default.*{debug}=false");

    EXPECT_EQ(registry.pass("main.network"s, DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(registry.pass("default.network"s, DebugMsg), LogRule::CheckStatus::NoPass);
}

GTEST_TEST(GlobalRegistryTest, PassAndNoPassMultipleFiltersWildcard) {
    Elogger::global_logger().install_filter_rules("main.*{debug}=true:default.*{debug}=false");

    EXPECT_EQ(Elogger::global_logger().registry().pass("main.network"s, DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(Elogger::global_logger().registry().pass("default.network"s, DebugMsg), LogRule::CheckStatus::NoPass);
    Elogger::global_logger().reset();
}
