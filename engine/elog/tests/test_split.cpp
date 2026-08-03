#include <gtest/gtest.h>
#include <engine/string_utils.h>
#include <ranges>
#include <algorithm>

GTEST_TEST(SplitString, SimpleSplit) {
    std::string s("1,2,3");
    std::vector<std::string> v;
    split(s, ',', v);
    EXPECT_EQ(v.size(), 3);
    std::vector<std::string> expected = {"1", "2", "3"};
    int i = 0;
    for (std::string const& expect : expected) {
        EXPECT_EQ(v[i], expect);
        i++;
    }
}

GTEST_TEST(SplitString, StringToSplitHasNoDelimiter) {
    std::string s("1");
    std::vector<std::string> v;
    split(s, ',', v);
    EXPECT_EQ(v.size(), 1);
    std::vector<std::string> expected = {"1"};
    int i = 0;
    for (std::string const& expect : expected) {
        EXPECT_EQ(v[i], expect);
        i++;
    }
}

GTEST_TEST(SplitString, SimpleSplitSV) {
    std::string s("1,2,3");
    std::vector<std::string_view> v;
    split(s, ',', v);
    EXPECT_EQ(v.size(), 3);
    std::vector<std::string> expected = {"1", "2", "3"};
    int i = 0;
    for (std::string const& expect : expected) {
        EXPECT_EQ(v[i], expect);
        i++;
    }
}

GTEST_TEST(SplitString, StringToSplitHasNoDelimiterSV) {
    std::string s("1");
    std::vector<std::string_view> v;
    split(s, ',', v);
    EXPECT_EQ(v.size(), 1);
    std::vector<std::string> expected = {"1"};
    int i = 0;
    for (std::string const& expect : expected) {
        EXPECT_EQ(v[i], expect);
        i++;
    }
}
