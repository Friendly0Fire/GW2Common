#pragma once

#include <filesystem>

#include <ShlObj.h>
#include <libzippp/libzippp.h>
using libzippp::ZipArchive;

#include "Common.h"
#include "Singleton.h"

class FileSystem : public Singleton<FileSystem>
{
    std::unordered_map<std::filesystem::path, std::unique_ptr<ZipArchive>> archiveCache_;

    ZipArchive* FindOrCache(const std::filesystem::path& p);

    static std::pair<std::filesystem::path, std::filesystem::path> SplitZipPath(const std::filesystem::path& p,
                                                                                ZipArchive** zip = nullptr);

public:
    static bool Exists(const std::filesystem::path& p);
    static std::vector<byte> ReadFile(const std::filesystem::path& p);
    static std::vector<byte> ReadFile(std::istream& is);
    static std::vector<byte> ReadFile(const libzippp::ZipEntry& ze);
    static std::string ReadFileAsText(const libzippp::ZipEntry& ze);
    static std::filesystem::path GetSystemPath(REFKNOWNFOLDERID id, DWORD flags = KF_FLAG_DEFAULT);
    static std::vector<std::filesystem::path> IterateZipFolders(const std::filesystem::path& zipPath);
};
