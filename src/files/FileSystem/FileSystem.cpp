#include "FileSystem.hpp"

// ---------------------------METHODS-----------------------------

SourceType FileSystem::getSourceType(const std::string& sourcePath)
{
    if (isFile(sourcePath))
        return SourceType::File;
    else if (isDirectory(sourcePath))
        return SourceType::Directory;
    else
        return SourceType::Unknown;
}

bool FileSystem::isDirectory(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false; // cannot access
    return S_ISDIR(info.st_mode);
}

bool FileSystem::isFile(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false; // cannot access
    return S_ISREG(info.st_mode);
}

bool FileSystem::exists(const std::string& path)
{
    return access(path.c_str(), F_OK);
}

bool FileSystem::canRead(const std::string& path)
{
    return access(path.c_str(), R_OK);
}

std::vector<std::string> FileSystem::listFilesIn(
    const std::string& directoryPath)
{
    std::vector<std::string> entries;

    DIR* dir = opendir(directoryPath.c_str());
    if (!dir)
        return entries; // cannot open directory

    struct dirent* entry;
    entry = readdir(dir);
    while (entry)
    {
        entries.push_back(entry->d_name);
        entry = readdir(dir);
    }

    closedir(dir);
    return entries;
}
