#pragma once

#include <filesystem>

#include <ShlObj.h>
#include <ZipFile.h>
// Fucking lzma
#undef True
#undef False

#include "Common.h"
#include "Singleton.h"

class FileSystem : public Singleton<FileSystem>
{
    std::unordered_map<std::filesystem::path, ZipArchive::Ptr> archiveCache_;

    ZipArchive::Ptr FindOrCache(const std::filesystem::path& p);

    static std::pair<std::filesystem::path, std::filesystem::path> SplitZipPath(const std::filesystem::path& p, ZipArchive::Ptr* zip = nullptr);

public:
    static bool Exists(const std::filesystem::path& p);
    static std::vector<byte> ReadFile(const std::filesystem::path& p);
    static std::vector<byte> ReadFile(std::istream& is);
    static std::filesystem::path GetSystemPath(REFKNOWNFOLDERID id, DWORD flags = KF_FLAG_DEFAULT);
    static std::vector<std::filesystem::path> IterateZipFolders(const std::filesystem::path& zipPath);
};
