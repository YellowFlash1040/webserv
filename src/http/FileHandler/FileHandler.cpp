#include "FileHandler.hpp"

FileHandler::FileHandler(const std::string &root, bool autoindex, const std::vector<std::string> &indexFiles)
    : _root(root), _autoindex(autoindex), _indexFiles(indexFiles) {}

/**
 * @brief Construct a candidate path for a potential index file in a directory.
 *
 * The / operator for std::filesystem::path performs path concatenation
 * in a platform-aware way (e.g., uses / on Unix-like systems and \ on Windows).
 *
 * @param fullPath The directory path in which to look for an index file.
 * @param index The index filename (e.g., "index.html").
 * @return std::filesystem::path The full candidate path (directory + index file).
 */
std::string FileHandler::resolveFilePath(const std::string &uri, const RequestContext &ctx) const
{
	// std::filesystem::path path = std::filesystem::path(uri).lexically_normal();
    // std::filesystem::path base;

    // if (!ctx.alias.empty())
    //     base = ctx.alias;
    // else if (!ctx.root.empty())
    //     base = ctx.root;
    // else
    //     base = std::filesystem::current_path();

    // std::filesystem::path fullPath;

    // // Check if alias is a file or directory
    // if (!ctx.alias.empty() && std::filesystem::exists(base) && std::filesystem::is_regular_file(base))
    // {
    //     // if alias is a file serve it directly, ignore URI
    //     fullPath = base;
    // }
    // else
    // {
    //     // if alias is a directory (or no alias) append URI normally
    //     fullPath = base / path.relative_path();
    // }

    // std::cout << "[FileHandler::resolveFilePath] Input URI: \"" << uri << "\"\n";
    // std::cout << "[FileHandler::resolveFilePath] Base path: \"" << base.string() << "\"\n";
    // std::cout << "[FileHandler::resolveFilePath] Full path: \"" << fullPath.string() << "\"\n";

    // if (isDirectory(fullPath))
    // {
    //     const std::vector<std::string> &indexFiles = ctx.alias.empty() ? _indexFiles : ctx.index_files;
    //     for (const std::string &index : indexFiles)
    //     {
    //         std::filesystem::path candidate = fullPath / index;
    //         if (fileExists(candidate.string()))
    //             return candidate.string(); // return first found
    //     }
    // }

    // bool inside = isPathInsideRoot(base.string(), fullPath.string());
    // if (!inside)
    //     throw std::runtime_error("Access outside root forbidden");

    // return fullPath.string();
	
    std::filesystem::path path = std::filesystem::path(uri).lexically_normal();
    std::filesystem::path fullPath;

    if (!ctx.alias.empty())
    {
        std::filesystem::path base(ctx.alias);
        if (std::filesystem::is_directory(base))
        {
            std::string rel = path.string();
            if (!ctx.matched_location.empty() && rel.find(ctx.matched_location) == 0)
            {
                rel.erase(0, ctx.matched_location.length());
                if (!rel.empty() && rel.front() == '/')
                    rel.erase(0, 1);
            }
            fullPath = base / rel;
        }
        else
        {
            fullPath = base;
        }
    }
    else if (!ctx.root.empty())
    {
        fullPath = ctx.root / path.relative_path();
    }
    else
    {
        fullPath = std::filesystem::current_path() / path.relative_path();
    }

    if (isDirectory(fullPath))
    {
        for (const std::string &index : ctx.index_files)
        {
            std::filesystem::path candidate = fullPath / index;
            if (fileExists(candidate.string()))
                return candidate.string();
        }
    }

    return fullPath.string();
}


bool FileHandler::fileExists(const std::string &path)
{
    struct stat s;
    return (stat(path.c_str(), &s) == 0);
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
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
	{
		std::cout << "serveFile: cannot open " << path << "\n";
        return handleNotFound();
	}

    std::ostringstream buffer;
    buffer << file.rdbuf();
	std::cout << "serveFile: read " << buffer.str().size() << " bytes\n";
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