#include <engine/elog.h>
#include <gtest/gtest.h>
#include <string>

using namespace std::string_literals;

using namespace elog;

namespace elog {
    // Must be in the same namespace as the enum
    void PrintTo(const MessageType& type, std::ostream* os) {
        switch (type) {

            // default: *os << "Invalid State (" << static_cast<int>(type) << ")"; break;
            case DebugMsg:
                *os << "DebugMsg";
                break;
            case WarningMsg:
                *os << "WarningMsg";
                break;
            case CriticalMsg:
                *os << "CriticalMsg";
                break;
            case FatalMsg:
                *os << "FatalMsg";
                break;
            case InfoMsg:
                *os << "InfoMsg";
                break;
        }
    }

    // Must be in the same namespace as the enum
    void PrintTo(const LogRule::CheckStatus& status, std::ostream* os) {
        switch (status) {
            case LogRule::CheckStatus::NoPass:
            *os << "NoPass";
            break;
            case LogRule::CheckStatus::Ignore:
            *os << "Ignore";
            break;
            case LogRule::CheckStatus::Pass:
            *os << "Pass";
            break;
        }
    }
}

// Must be in the same namespace as the enum
std::ostream& operator<<(std::ostream& os, const MessageType& type) {
    elog::PrintTo(type, &os);
    return os;
}

// Must be in the same namespace as the enum
std::ostream& operator<<(std::ostream& os, const LogRule::CheckStatus& status) {
    elog::PrintTo(status, &os);
    return os;
}

TEST(LogRuleTest, ExactRuleMatchesExactCategory)
{
    LogRule rule("network.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, ExactRuleDoesNotMatchDifferentCategory)
{
    LogRule rule("network.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.ftp"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, ExactRuleDoesNotMatchParentCategory)
{
    LogRule rule("network.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, ExactRuleDoesNotMatchLongerCategory)
{
    LogRule rule("network.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http.client"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, WildcardMatchesSingleSubcategory)
{
    LogRule rule("network.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(rule.pass("network.ftp"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(rule.pass("network.smtp"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, MessageTypeDoesNotMatch)
{
    LogRule rule("network.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::WarningMsg), LogRule::CheckStatus::Ignore);
    EXPECT_EQ(rule.pass("network.ftp"s,MessageType::InfoMsg), LogRule::CheckStatus::Ignore);
    EXPECT_EQ(rule.pass("network.smtp"s,MessageType::FatalMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, WildcardDoesNotMatchDifferentMainCategory)
{
    LogRule rule("network.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("graphics.opengl"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, WildcardDoesNotMatchParentCategory)
{
    LogRule rule("network.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, WildcardDoesNotMatchGrandchildCategory)
{
    LogRule rule("network.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http.client"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, DisabledExactRule)
{
    LogRule rule("network.http", MessageType::DebugMsg, false);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::NoPass);
}

TEST(LogRuleTest, DisabledWildcardRule)
{
    LogRule rule("network.*", MessageType::DebugMsg, false);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::NoPass);
}

TEST(LogRuleTest, RootCategoryMatches)
{
    LogRule rule("network", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, RootCategoryDoesNotMatchSubcategory)
{
    LogRule rule("network", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, SimilarNamesDoNotMatch)
{
    LogRule rule("net.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, CaseSensitiveMatching)
{
    LogRule rule("Network.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
    EXPECT_EQ(rule.pass("Network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, WildcardInMiddleMatches)
{
    LogRule rule("network.*.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.client.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(rule.pass("network.server.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, WildcardInMiddleDoesNotMatchDifferentLastComponent)
{
    LogRule rule("network.*.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.client.https"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, WildcardInMiddleDoesNotMatchDifferentFirstComponent)
{
    LogRule rule("network.*.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("graphics.client.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Ignore);
}

TEST(LogRuleTest, WildcardInMiddleDoesNotMatchDifferentLength)
{
    LogRule rule("network.*.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(rule.pass("network.client.api.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, LeadingWildcardMatches)
{
    LogRule rule("*.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(rule.pass("graphics.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, MultipleWildcardsMatch)
{
    LogRule rule("*.http", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("network.client.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(rule.pass("graphics.opengl.http"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, AllWildcardsMatchAnyCategoryOfSameDepth)
{
    LogRule rule("*.*.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("a.b.c"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(rule.pass("network.http.client"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}

TEST(LogRuleTest, AllWildcardsRequireSameDepth)
{
    LogRule rule("*.*.*", MessageType::DebugMsg, true);

    EXPECT_EQ(rule.pass("a.b"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
    EXPECT_EQ(rule.pass("a.b.c.d"s,MessageType::DebugMsg), LogRule::CheckStatus::Pass);
}