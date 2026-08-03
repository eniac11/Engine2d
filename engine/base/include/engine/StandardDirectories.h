#pragma once
#include <filesystem>
#include <vector>

class StandardDirectories final {
    public:
        enum class KnownDirectories { Data, Config, Cache, State, Runtime };

        std::filesystem::path get_user_dir(KnownDirectories kd) const;
        std::vector<std::filesystem::path> get_user_dirs(KnownDirectories kd) const;

        std::filesystem::path get_user_dir(KnownDirectories kd, std::string const& prefix) const;
        std::vector<std::filesystem::path> get_user_dirs(KnownDirectories kd, std::string const& prefix) const;

        std::filesystem::path get_system_dir(KnownDirectories kd) const;
        std::vector<std::filesystem::path> get_system_dirs(KnownDirectories kd) const;

        std::filesystem::path get_system_dir(KnownDirectories kd, std::string const& prefix) const;
        std::vector<std::filesystem::path> get_system_dirs(KnownDirectories kd, std::string const& prefix) const;
};
