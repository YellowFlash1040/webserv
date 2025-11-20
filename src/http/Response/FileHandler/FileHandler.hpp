#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <cerrno>   // for errno
#include <cstring>  // for strerror

#include "../../../config/Config/request_resolving/RequestContext/RequestContext.hpp"
#include "../../Request/RequestData/RequestData.hpp"
#include "../RawResponse/RawResponse.hpp"

class FileHandler
{
	private:
	std::string _root;
	std::vector<std::string> _indexFiles;
	
	public:
	FileHandler() = delete;
	FileHandler(const std::vector<std::string> &indexFiles);
	FileHandler(const FileHandler &) = default;
	FileHandler &operator=(const FileHandler &) = default;
	~FileHandler() = default;
	
	DeliveryInfo getDeliveryInfo(const std::string &path, size_t maxInMemory = 1024 * 1024);
	static bool pathExists(const std::string& path);
	static bool existsAndIsFile(const std::string& path);
	static bool existsAndIsDirectory(const std::string& path);
	static bool hasReadPermission(const std::string& path);
	static bool hasWritePermission(const std::string& path);
	static bool deleteFile(const std::string& path);

	std::string generateAutoindex(const std::string& dirPath);
	
	// static FileResult internalServerError();
	static std::string detectMimeType(const std::string& path);
	std::string getIndexFilePath(const std::string& dirPath) const;
	
	//std::string readFileToString(const std::string& path);
};

#endif