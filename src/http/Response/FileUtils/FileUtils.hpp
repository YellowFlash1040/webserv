#ifndef FILEUTILS_HPP
#define FILEUTILS_HPP

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
# include<vector>
#include <filesystem>

#include "../utils/debug.hpp"

struct FileInfo
{
    bool exists;
    bool isFile;
    bool isDir;
    bool readable;
    bool writable;
    bool executable;
};

namespace FileUtils
{   
    FileInfo getFileInfo(const std::string& path);

	std::string getFirstValidIndexFile(const std::string& dirPath, const std::vector<std::string>& indexFiles);
	std::string generateAutoindex(const std::string& dirPath);
	std::string detectMimeType(const std::string& path);
	void deleteFile(const std::string& path);
}

#endif