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

// Maximum file size to load in memory (1 MB default)
constexpr size_t MAX_IN_MEMORY_FILE_SIZE = 1024 * 1024;

namespace FileUtils
{
	bool pathExists(const std::string& path);
	bool existsAndIsFile(const std::string& path);
	bool existsAndIsDirectory(const std::string& path);
	bool hasReadPermission(const std::string& path);
	bool hasWritePermission(const std::string& path);
	bool deleteFile(const std::string& path);

	std::string readFileToString(const std::string& path);
	std::string getFirstValidIndexFile(const std::string& dirPath, const std::vector<std::string>& indexFiles);
	std::string generateAutoindex(const std::string& dirPath);
	std::string detectMimeType(const std::string& path);


}

#endif