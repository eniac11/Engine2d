

#include <engine/resources/dwarfs_fs.h>
#include <engine/elog.h>

#include <utility>
#include <vfspp/ThreadingPolicy.hpp>

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#include <dwarfs/reader/filesystem_v2.h>
#include <dwarfs/os_access_generic.h>
// BUG(hadley): workaround for dwarfs::level_type::DEBUG becoming a macro
#ifdef DEBUG
#undef DEBUG
#define FOO_DWFS_DEBUG
#endif
#include <dwarfs/logger.h>
// #define DEBUG


#include <ostream>
#include <print>

ELOG_DECLARE_LOGGING_CATEGORY(lcResources, "engine.resources")

class Dwarfs_elogLogger : public dwarfs::logger {
    public:
        Dwarfs_elogLogger() {
            #ifdef FOO_DWFS_DEBUG
            set_policy<dwarfs::debug_logger_policy>();
            #else
            set_policy<dwarfs::prod_logger_policy>();
            #endif
        };

        void write(level_type level, std::string_view output, dwarfs::source_location loc) override {
            switch (level) {
                case FATAL:
                    elogCCriticalEnabled(lcResources) std::println(elogCCritical(lcResources), "{}", output);
                case ERROR:
                    elogCErrorEnabled(lcResources) std::println(elogCError(lcResources), "{}", output);
                    break;
                case WARN:
                    elogCWarningEnabled(lcResources) std::println(elogCWarning(lcResources), "{}", output);

                    break;
                case INFO:
                    elogCInfoEnabled(lcResources) std::println(elogCInfo(lcResources), "{}", output);

                    break;
                case VERBOSE:
                    elogCDebugEnabled(lcResources) std::println(elogCDebug(lcResources), "VERBOSE: {}", output);

                    break;
                case DEBUG:
                    elogCDebugEnabled(lcResources) std::println(elogCDebug(lcResources), "{}", output);

                    break;
                case TRACE:
                    elogCDebugEnabled(lcResources) std::println(elogCDebug(lcResources), "TRACE: {}", output);

                    break;
            }
        }

        level_type threshold() const override {
            #ifdef FOO_DWFS_DEBUG
            return level_type::TRACE;
            #else
            return level_type::INFO;
            #endif
        }
};

class Dwarfs_File : public vfspp::IFile {
    // dwarfs::reader::inode_view m_view;
    std::uint32_t m_file_inode;
    std::shared_ptr<dwarfs::reader::filesystem_v2> m_fs;
    vfspp::FileInfo m_info;
    bool m_isopen = false;
    std::uint32_t m_open_inode = 0;
    dwarfs::file_off_t m_seek_pos{};

    public:
        explicit Dwarfs_File(const vfspp::FileInfo& info, std::shared_ptr<dwarfs::reader::filesystem_v2> fs,
                             dwarfs::reader::inode_view const& view) :
            m_file_inode(view.inode_num()), m_fs(std::move(fs)), m_info(info) {
        }

        [[nodiscard]] vfspp::FileInfo const& GetFileInfo() const override {
            return m_info;
        }

        [[nodiscard]] uint64_t Size() const override {
            auto view = m_fs->find(m_file_inode);
            if (view) {
                dwarfs::file_stat const& stat = m_fs->getattr(*view);
                return stat.size();
            }

            return 0;
        }

        [[nodiscard]] bool IsReadOnly() const override {
            auto view = m_fs->find(m_file_inode);
            if (view) {
                dwarfs::file_stat const& stat = m_fs->getattr(*view);
                auto perms = stat.status().permissions();
                if ((perms & std::filesystem::perms::group_write) == std::filesystem::perms::none or (perms &
                    std::filesystem::perms::owner_write) == std::filesystem::perms::none or (perms &
                    std::filesystem::perms::others_write) == std::filesystem::perms::none) {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] bool Open(FileMode mode) override {
            auto view = m_fs->find(m_file_inode);
            if (view) {
                m_open_inode = m_fs->open(*view);
                m_isopen = true;
            }

            return m_isopen;
        }

        void Close() override {
            // it looks like dwarfs does not track open files
            // m_fs->close(m_open_inode); // does not exist
            m_isopen = false;
            m_open_inode = 0;
        }

        [[nodiscard]] bool IsOpened() const override {
            return m_isopen;
        }

        uint64_t Seek(uint64_t offset, Origin origin) override {
            if (not IsOpened()) {
                return 0;
            }
            dwarfs::file_off_t calculated_offset = m_seek_pos;
            switch (origin) {
                case Origin::Begin:
                    // This is SEEK_SET
                    calculated_offset = static_cast<dwarfs::file_off_t>(offset);
                    break;
                case Origin::End:
                    // BUG: In vfspp offset is uint which cannot have negative numbers
                    elogCWarningEnabled(lcResources)
                        std::println(
                            elogCWarning(lcResources),
                            "Cannot seek to end of file as vfspp cannot accept negative numbers");
                    return 0;
                    break;
                case Origin::Set:
                    // This is SEEK_CUR
                    // FIXME: This may overflow
                    calculated_offset = m_seek_pos + static_cast<dwarfs::file_off_t>(offset);
                    break;
            }
            m_seek_pos = calculated_offset;

            if (auto data = m_fs->seek(m_open_inode, m_seek_pos, dwarfs::reader::seek_whence::data); data >= 0)
                return data;
            // NOTE: 0 in vfspp is the error instead of -1 for unix
            return 0;
        }

        [[nodiscard]] uint64_t Tell() const override {
            return m_seek_pos;
        }

        uint64_t Read(std::span<uint8_t> buffer) override {
            if (not IsOpened()) {
                return 0;
            }
            size_t read = m_fs->read(m_open_inode, reinterpret_cast<char*>(buffer.data()), buffer.size(), m_seek_pos);
            if (read > 0) {
                return read;
            }
            return 0;
        }

        uint64_t Read(std::vector<uint8_t>& buffer, uint64_t size) override {
            buffer.resize(size);
            return Read(std::span(buffer.data(), size));
        }

        uint64_t Write(std::span<const uint8_t> buffer) override {
            return 0;
        }

        uint64_t Write(std::vector<uint8_t> const& buffer) override {
            return 0;
        }
};

struct dwarfs_fs_p {
    ~dwarfs_fs_p() = default;

    dwarfs_fs_p(Dwarfs_elogLogger  lgr)
        : logger(std::move(lgr)) {
    }

    Dwarfs_elogLogger logger;
    dwarfs::os_access_generic access_generic{};
    std::shared_ptr<dwarfs::reader::filesystem_v2> fs;
};

dwarfs_fs::dwarfs_fs(std::string virtual_path, std::string dwarfs_path) : m_dwarfs_path(std::move(dwarfs_path)),
                                                                          m_VirtualPath(std::move(virtual_path)),
                                                                          m_p(std::make_unique<dwarfs_fs_p>(Dwarfs_elogLogger())) {
}

dwarfs_fs::~dwarfs_fs() {

}

bool dwarfs_fs::Initialize() {
    [[maybe_unused]] auto lock = vfspp::ThreadingPolicy::Lock(m_Mutex);


    auto const fs = std::make_shared<dwarfs::reader::filesystem_v2>(m_p->logger, m_p->access_generic, m_dwarfs_path);
    fs->walk_data_order([this](dwarfs::reader::dir_entry_view const& dir_entry) {
        vfspp::FileInfo info(m_VirtualPath, BasePath(), dir_entry.fs_path().string());
        m_filelist.emplace(info.VirtualPath(), std::make_pair(dir_entry.inode().inode_num(), info));
    });
    m_p->fs = fs;
    m_initialised = true;
    return m_initialised;
}

void dwarfs_fs::Shutdown() {
    [[maybe_unused]] auto lock = vfspp::ThreadingPolicy::Lock(m_Mutex);
    m_p->fs = nullptr;
    m_initialised = false;
}

bool dwarfs_fs::IsInitialized() const {
    return m_initialised;
}

std::string const& dwarfs_fs::BasePath() const {
    return m_base_path;
}

std::string const& dwarfs_fs::VirtualPath() const {
    return m_VirtualPath;
}

vfspp::IFileSystem::FilesList dwarfs_fs::GetFilesList() const {
    [[maybe_unused]] auto lock = vfspp::ThreadingPolicy::Lock(m_Mutex);
    FilesList list;
    // std::ranges::transform(m_filelist.begin(), m_filelist.end(), std::back_inserter(list), [](std::pair<std::string, vfspp::FileInfo> const& data) {
    //     return data.second;
    // });
    //
    // for (auto const& [_, info] : m_filelist) {
    //     list.push_back(info);
    // }

    for (auto const& info : m_filelist | std::views::values) {
        list.push_back(info.second);
    }

    return list;
}

bool dwarfs_fs::IsReadOnly() const {
    return true;
}

vfspp::IFilePtr dwarfs_fs::OpenFile(std::string const& virtualPath, vfspp::IFile::FileMode mode) {
    [[maybe_unused]] auto lock = vfspp::ThreadingPolicy::Lock(m_Mutex);
    const bool requestWrite = vfspp::IFile::ModeHasFlag(mode, vfspp::IFile::FileMode::Write);
    // NOTE: Maybe it should be written as (IsReadOnly() && requestWrite) but I have chosen to not,
    //       however if I change IsReadOnly() (unlikely) this will be a silent bug.
    //       Right now it reduces the possible multiple return path checks need for the code to be correct
    //       if IsReadOnly() can change
    if (requestWrite) {
        return nullptr;
    }
    auto entryIt = m_filelist.find(virtualPath);
    if (entryIt == m_filelist.end()) {
        return nullptr;
    }
    auto const& entry = entryIt->second;
    auto const& inode = m_p->fs->find(entry.first);

    if (inode) {
        auto file = std::make_shared<Dwarfs_File>(entry.second, m_p->fs, *inode);
        file->Open(mode);
        return file;
    }

    return nullptr;
}

void dwarfs_fs::CloseFile(vfspp::IFilePtr file) {
    [[maybe_unused]] auto lock = vfspp::ThreadingPolicy::Lock(m_Mutex);
    file->Close();
}

vfspp::IFilePtr dwarfs_fs::CreateFile(std::string const& virtualPath) {
    return nullptr;
}

bool dwarfs_fs::RemoveFile(std::string const& virtualPath) {
    return false;
}

bool dwarfs_fs::CopyFile(std::string const& srcVirtualPath, std::string const& dstVirtualPath, bool overwrite) {
    return false;
}

bool dwarfs_fs::RenameFile(std::string const& srcVirtualPath, std::string const& dstVirtualPath) {
    return false;
}

bool dwarfs_fs::IsFileExists(std::string const& virtualPath) const {
    [[maybe_unused]] auto lock = vfspp::ThreadingPolicy::Lock(m_Mutex);
    for (auto const& [_, info] : m_filelist | std::views::values) {
        if (info.VirtualPath() == virtualPath) {
            return true;
        }
    }
    return false;
}
