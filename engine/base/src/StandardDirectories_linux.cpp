#include <algorithm>
#include <cstdlib>
#include <engine/StandardDirectories.h>
#include <engine/string_utils.h>
#include <filesystem>
#include <ranges>
#include <utility>
#include <vector>
std::filesystem::path StandardDirectories::get_user_dir(StandardDirectories::KnownDirectories kd) const {
    namespace fs = std::filesystem;
    std::pair<std::string, fs::path> env_and_fallback{};
    switch (kd) {

        case KnownDirectories::Data:
            env_and_fallback = std::make_pair("XDG_DATA_HOME", fs::path(".local") / "share");
            break;
        case KnownDirectories::Config:
            env_and_fallback = std::make_pair("XDG_CONFIG_HOME", fs::path(".config"));
            break;
        case KnownDirectories::State:
            env_and_fallback = std::make_pair("XDG_STATE_HOME", fs::path(".local") / "state");
            break;
        case KnownDirectories::Runtime:
            env_and_fallback = std::make_pair("XDG_RUNTIME_DIR", fs::path{});
            break;
        case KnownDirectories::Cache:
            env_and_fallback = std::make_pair("XDG_CACHE_HOME", fs::path(".cache"));
            break;
    }
    const char* env = std::getenv(env_and_fallback.first.c_str());
    if (env != nullptr) {
        return fs::path(env);
    }
    if (kd == KnownDirectories::Runtime)
        std::unreachable();
    const char* home_env = std::getenv("HOME");
    if (home_env == nullptr) {
        std::unreachable();
    }

    return fs::path{home_env} / env_and_fallback.second;
}
std::vector<std::filesystem::path> StandardDirectories::get_user_dirs(StandardDirectories::KnownDirectories kd) const {

    namespace fs = std::filesystem;
    using vectype = std::vector<fs::path>;
    fs::path const& user_dir = get_user_dir(kd);
    std::pair<std::string, vectype> env_and_fallback{};
    switch (kd) {

        case KnownDirectories::Data:
            env_and_fallback = std::make_pair(
                "XDG_DATA_DIRS",
                vectype{user_dir, fs::path("/") / "usr" / "local" / "share", fs::path("/") / "usr" / "share"});
            break;
        case KnownDirectories::Config:
            env_and_fallback = std::make_pair("XDG_CONFIG_DIRS", vectype{user_dir, fs::path("/") / "etc" / "xdg"});
            break;
        case KnownDirectories::State:
            env_and_fallback = std::make_pair("XDG_STATE_DIRS", vectype{user_dir});
            break;
        case KnownDirectories::Runtime:
            env_and_fallback = std::make_pair("XDG_RUNTIME_DIR", vectype{});
            break;
        case KnownDirectories::Cache:
            env_and_fallback = std::make_pair("XDG_CAHCE_DIRS", vectype{user_dir});
            break;
    }
    const char* env = std::getenv(env_and_fallback.first.c_str());
    if (env != nullptr) {
        std::vector<std::string> str_paths{};
        split(std::string(env), ':', str_paths);
        vectype paths;
        paths.append_range(str_paths |
                           std::views::transform([](std::string const& str_path) { return fs::path(str_path); }));
        return paths;
    }
    if (kd == KnownDirectories::Runtime)
        std::unreachable();
    return env_and_fallback.second;
}

std::filesystem::path StandardDirectories::get_user_dir(StandardDirectories::KnownDirectories kd,
                                                        std::string const& prefix) const {
    namespace fs = std::filesystem;
    fs::path path = get_user_dir(kd);
    return path / prefix;
}
std::vector<std::filesystem::path> StandardDirectories::get_user_dirs(StandardDirectories::KnownDirectories kd,
                                                                      std::string const& prefix) const {
    namespace fs = std::filesystem;
    using vectype = std::vector<fs::path>;
    auto const& mypaths = get_user_dirs(kd);
    vectype paths;
    paths.append_range(mypaths | std::views::transform([prefix](fs::path const& path) { return path / prefix; }));
    return paths;
}

std::filesystem::path StandardDirectories::get_system_dir(StandardDirectories::KnownDirectories kd) const {}
std::vector<std::filesystem::path>
StandardDirectories::get_system_dirs(StandardDirectories::KnownDirectories kd) const {}

std::filesystem::path StandardDirectories::get_system_dir(StandardDirectories::KnownDirectories kd,
                                                          std::string const& prefix) const {}
std::vector<std::filesystem::path> StandardDirectories::get_system_dirs(StandardDirectories::KnownDirectories kd,
                                                                        std::string const& prefix) const {}
