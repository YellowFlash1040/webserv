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

#include "../../config/shared/RequestContext/RequestContext.hpp"


class FileHandler
{
	public:

		FileHandler() = delete;
		FileHandler(bool autoindex, const std::vector<std::string> &indexFiles);
		FileHandler(const FileHandler &) = default;
		FileHandler &operator=(const FileHandler &) = default;
		~FileHandler() = default;
		
		// Resolve a URI to an absolute filesystem path under a given root.
		std::string resolveFilePath(const std::string &uri, const RequestContext &ctx) const;

		// Filesystem checks
		static bool isDirectory(const std::string &path);
		static bool fileExists(const std::string &path);

		// Directory handling
		std::string handleDirectory(const std::string &dirPath) const;
		static std::string generateAutoindex(const std::string &dirPath);

		// File serving and errors
		static std::string serveFile(const std::string &path);
		static std::string handleNotFound();

		// MIME type detection
		static std::string detectMimeType(const std::string &path);
		
		private:
		std::string _root;
		bool _autoindex;
		std::vector<std::string> _indexFiles;
		bool isPathInsideRoot(const std::string& root, const std::string& resolved) const;
};

#endif