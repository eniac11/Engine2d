#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <ranges>
#include <utility>
#include <vector>
#include "engine/StandardDirectories.h"

namespace fs = std::filesystem;

TEST(StandardDirectoriesTests, user_dir) {
    StandardDirectories sd;
    auto home = fs::path(getenv("HOME"));
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::State), home / ".local" / "state");
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Config), home / ".config");
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Cache), home / ".cache");
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Data), home / ".local" / "share");
}

TEST(StandardDirectoriesTests, user_dir_prefix) {
    StandardDirectories sd;
    auto home = fs::path(getenv("HOME"));
    std::string prefix = "prefix";
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::State, prefix),
              home / ".local" / "state" / prefix);
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::State, prefix),
              home / ".local" / "state" / prefix);
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Config, prefix), home / ".config" / prefix);
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Cache, prefix), home / ".cache" / prefix);
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Data, prefix), home / ".local" / "share" / prefix);
}

TEST(StandardDirectoriesTests, user_dirs) {
    StandardDirectories sd;
    auto home = fs::path(getenv("HOME"));
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::State), home / ".local" / "state");
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Config), home / ".config");
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Cache), home / ".cache");
    EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Data), home / ".local" / "share");
}

template <typename T>
void expect_vector(std::vector<T> results, std::vector<T> const& expected) {
    EXPECT_FALSE(results.empty());
    EXPECT_GE(results.size(), expected.size());
    for (auto const &pair : std::ranges::zip_transform_view(
             [](T const& result, T const& expect) { return std::make_pair(result, expect); }, results, expected)) {
        EXPECT_EQ(pair.first, pair.second);
    }
}

TEST(StandardDirectoriesTests, user_dirs_prefix) {
    StandardDirectories sd;
    using vectype = std::vector<fs::path>;
    auto home = fs::path(getenv("HOME"));
    std::string prefix = "prefix";
    expect_vector(sd.get_user_dirs(StandardDirectories::KnownDirectories::State, prefix),
                  vectype{home / ".local" / "state" / prefix});
    // EXPECT_EQ(sd.get_user_dirs(StandardDirectories::KnownDirectories::State, prefix),
    //           home / ".local" / "state" / prefix);
    // EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Config, prefix), home / ".config" / prefix);
    // EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Cache, prefix), home / ".cache" / prefix);
    // EXPECT_EQ(sd.get_user_dir(StandardDirectories::KnownDirectories::Data, prefix), home / ".local" / "share" /
    // prefix);
}
