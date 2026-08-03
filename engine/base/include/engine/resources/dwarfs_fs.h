
#pragma once

#include <vfspp/IFileSystem.h>

#include <mutex>
#include <unordered_map>

struct dwarfs_fs_p;

class dwarfs_fs final : public vfspp::IFileSystem {
    using inode_type = std::uint32_t;
    public:
        // TODO: Make private and take a dwarfs:: filesystem and use static create functions for
        //       "from memory", from "static storage", from "os filesystem"
        explicit dwarfs_fs(std::string virtual_path, std::string dwarfs_path);
        ~dwarfs_fs() override;
        [[nodiscard]] bool Initialize() override;
        void Shutdown() override;
        [[nodiscard]] bool IsInitialized() const override;
        [[nodiscard]] std::string const& BasePath() const override;
        [[nodiscard]] std::string const& VirtualPath() const override;
        [[nodiscard]] FilesList GetFilesList() const override;
        [[nodiscard]] bool IsReadOnly() const override;
        vfspp::IFilePtr OpenFile(std::string const& virtualPath, vfspp::IFile::FileMode mode) override;
        void CloseFile(vfspp::IFilePtr file) override;
        vfspp::IFilePtr CreateFile(std::string const& virtualPath) override;
        bool RemoveFile(std::string const& virtualPath) override;
        bool CopyFile(std::string const& srcVirtualPath, std::string const& dstVirtualPath, bool overwrite) override;
        bool RenameFile(std::string const& srcVirtualPath, std::string const& dstVirtualPath) override;
        [[nodiscard]] bool IsFileExists(std::string const& virtualPath) const override;
    private:
        mutable std::mutex m_Mutex;
        std::string m_dwarfs_path;
        std::string m_VirtualPath;
        std::unique_ptr<dwarfs_fs_p> m_p;
        std::unordered_map<std::string, std::pair<inode_type,vfspp::FileInfo>> m_filelist;
        bool m_initialised = false;
        std::string m_base_path = "";
};
