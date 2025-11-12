#include "FileHandler.hpp"

FileHandler::FileHandler(bool autoindex, const std::vector<std::string> &indexFiles)
	: _autoindex(autoindex), _indexFiles(indexFiles) {}

bool FileHandler::fileExists(const std::string &path)
{
    struct stat s;
    if (stat(path.c_str(), &s) != 0)
    {
        std::cout << "[fileExists] stat failed for path: \"" << path 
                  << "\" errno: " << errno << " (" << strerror(errno) << ")\n";
        return false;
    }
    std::cout << "[fileExists] stat succeeded: \"" << path << "\", mode=" << s.st_mode
              << ", S_ISREG=" << S_ISREG(s.st_mode) << "\n";
    return S_ISREG(s.st_mode);
}

bool FileHandler::isDirectory(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
		return S_ISDIR(s.st_mode);
	return false;
}

std::string FileHandler::handleDirectory(const std::string &dirPath) const
{
	if (_autoindex)
		return generateAutoindex(dirPath);

	// Try each index file (index.html, etc.)
	for (std::vector<std::string>::const_iterator it = _indexFiles.begin(); it != _indexFiles.end(); ++it)
	{
		std::string indexPath = dirPath + "/" + *it;
		if (fileExists(indexPath))
			return serveFile(indexPath);
	}

	return handleNotFound();
}

std::string FileHandler::generateAutoindex(const std::string &dirPath)
{
	DIR *dir = opendir(dirPath.c_str());
	if (!dir)
		return handleNotFound();

	std::ostringstream html;
	html << "<html><head><title>Index of " << dirPath << "</title></head><body>\n";
	html << "<h1>Index of " << dirPath << "</h1><ul>\n";

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == ".") continue;
		html << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
	}

	closedir(dir);
	html << "</ul></body></html>";
	return html.str();
}

std::string FileHandler::serveFile(const std::string &path)
{
	if (!fileExists(path))
	{
		std::cout << "[serveFile]: not found " << path << "\n";
		return handleNotFound(); // 404
	}

	// Check read permissions
	if (access(path.c_str(), R_OK) != 0)
	{
		std::cout << "[serveFile]: forbidden " << path << "\n";
		return "__FORBIDDEN__"; // special signal
	}

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "[serveFile]: cannot open " << path << "\n";
		return handleNotFound(); // fallback
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();
	std::cout << "[serveFile]: read " << buffer.str().size() << " bytes\n";
	return buffer.str();
}

std::string FileHandler::handleNotFound()
{
	return "<html><body><h1>404 Not Found</h1></body></html>";
}

std::string FileHandler::detectMimeType(const std::string &path)
{
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dot + 1);
	static const std::unordered_map<std::string, std::string> mimeTypes =
	{
		{"html", "text/html"},
		{"htm", "text/html"},
		{"css", "text/css"},
		{"js", "application/javascript"},
		{"json", "application/json"},
		{"jpg", "image/jpeg"},
		{"jpeg", "image/jpeg"},
		{"png", "image/png"},
		{"gif", "image/gif"},
		{"txt", "text/plain"},
		{"pdf", "application/pdf"},
		{"ico", "image/x-icon"}
	};

	std::unordered_map<std::string, std::string>::const_iterator it = mimeTypes.find(ext);
	if (it != mimeTypes.end())
		return it->second;

	return "application/octet-stream";
}

bool FileHandler::isPathInsideRoot(const std::string& root, const std::string& resolved) const
{
	try
	{
		std::filesystem::path rootPath = std::filesystem::canonical(root);
		std::filesystem::path filePath = std::filesystem::weakly_canonical(std::filesystem::path(root) / resolved);

		return std::mismatch(rootPath.begin(), rootPath.end(), filePath.begin()).first == rootPath.end();
	}
	catch (const std::filesystem::filesystem_error&)
	{
		return false;
	}
}

// Returns the first existing index file path in the directory, or empty string if none exist
std::string FileHandler::getIndexFilePath(const std::string &dirPath) const
{
	std::cout << "[getIndexFilePath] dirPath = \"" << dirPath << "\"\n";

	if (!isDirectory(dirPath))
	{
		std::cout << "[getIndexFilePath] Not a directory\n";
		return "";
	}

	for (const auto &idx : _indexFiles)
	{
		std::string indexPath = dirPath;
		if (!indexPath.empty() && indexPath.back() != '/')
			indexPath += '/';
		indexPath += idx;

		bool exists = fileExists(indexPath);
		std::cout << "[getIndexFilePath] Checking index file: " << indexPath
				  << " -> " << (exists ? "exists" : "not found") << "\n";

		if (exists)
			return indexPath;
	}

	std::cout << "[getIndexFilePath] No index file found\n";
	return "";
}
