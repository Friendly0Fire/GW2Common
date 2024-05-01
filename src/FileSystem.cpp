#include "FileSystem.h"

#include <fstream>

#include "Utility.h"

namespace fs = std::filesystem;

ZipArchive* FileSystem::FindOrCache(const fs::path& p) {
    if(auto arc = archiveCache_.find(p); arc != archiveCache_.end())
        return arc->second.get();

    auto zip = std::make_unique<ZipArchive>(p.string());
    if(!zip->open(ZipArchive::ReadOnly))
        return nullptr;

    return (archiveCache_[p] = std::move(zip)).get();
}

std::pair<fs::path, fs::path> FileSystem::SplitZipPath(const fs::path& p, ZipArchive** zip) {
    auto p2 = p;
    do {
        p2 = p2.parent_path();
    }
    while(p2.has_relative_path() && !fs::exists(p2));

    LogDebug(L"Looking for path '{}'; path '{}' is the closest existing parent", p.wstring(), p2.wstring());

    if(p2.has_extension() && p2.extension() == L".zip") {
        auto& fs = i();

        auto z = fs.FindOrCache(p2.string());
        if(z) {
            LogDebug(L"Found and loaded zip file '{}'", p2.wstring());
            if(zip)
                *zip = z;
            return { p2, fs::relative(p, p2) };
        }
    }

    if(zip)
        *zip = nullptr;
    return { p, fs::path() };
}

bool FileSystem::Exists(const fs::path& p) {
    if(fs::exists(p))
        return true;
    if(!p.has_relative_path())
        return false;
    if(p.wstring().find(L".zip") == std::wstring::npos)
        return false;

    ZipArchive* archive;
    const auto& [base, sub] = SplitZipPath(p, &archive);
    if(!archive)
        return false;

    return !archive->getEntry(sub.lexically_normal().generic_string()).isNull();
}

std::vector<std::filesystem::path> FileSystem::IterateZipFolders(const std::filesystem::path& zipPath) {
    LogDebug(L"Iterating files in archive '{}'", zipPath.wstring());

    auto& fs = i();
    ZipArchive* archive = fs.FindOrCache(zipPath);
    if(!archive)
        return {};

    std::vector<std::filesystem::path> paths;
    for(u32 i = 0; i < archive->getEntriesCount(); i++) {
        auto p = archive->getEntry(i);
        if(!p.isDirectory())
            continue;

        auto filepath = (zipPath / p.getName()).lexically_normal();

        LogDebug(L"Found file '{}', mapping to '{}'", utf8_decode(p.getName()), filepath.wstring());

        paths.push_back(filepath);
    }

    return paths;
}

std::vector<byte> FileSystem::ReadFile(std::istream& is) {
    std::vector<byte> output;
    output.reserve(1024);

    byte buffer[1024];
    size_t n = 0;
    do {
        is.read(reinterpret_cast<char*>(buffer), 1024);
        n = is.gcount();
        output.insert(output.end(), std::begin(buffer), std::begin(buffer) + n);
    }
    while(n == 1024);

    return output;
}

std::vector<byte> FileSystem::ReadFile(const libzippp::ZipEntry& ze)
{
    if (ze.isNull())
        return {};

    auto* data = static_cast<u8*>(ze.readAsBinary());
    std::vector<byte> vec(data, data + ze.getSize());
    delete[] data;
    return vec;
}

std::string FileSystem::ReadFileAsText(const libzippp::ZipEntry& ze)
{
    if (ze.isNull())
        return {};

    return ze.readAsText();
}

std::filesystem::path FileSystem::GetSystemPath(const KNOWNFOLDERID& id, DWORD flags) {
    wchar_t* path = nullptr;
    SHGetKnownFolderPath(id, flags, nullptr, &path);

    std::filesystem::path p(path);
    LogDebug(L"Mapped system path {} to '{}'", LogGUID<wchar_t>(id), p.wstring());
    CoTaskMemFree(path);
    return p;
}

std::vector<byte> FileSystem::ReadFile(const fs::path& p) {
    if(fs::exists(p)) {
        std::ifstream is(p.wstring().c_str(), std::ifstream::binary);
        if(is.bad())
            return {};

        return ReadFile(is);
    }

    ZipArchive* archive;
    const auto& [base, sub] = SplitZipPath(p, &archive);

    auto entry = archive->getEntry(sub.generic_string());
    return ReadFile(entry);
}
