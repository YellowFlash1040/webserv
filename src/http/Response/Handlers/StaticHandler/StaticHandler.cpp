#include "StaticHandler.hpp"

namespace StaticHandler
{
   std::string serve(FileHandler& fileHandler, const RequestContext& ctx, const RequestData& req)
{
	try
	{
		std::cout << "[StaticHandler::serve] req.uri = " << req.uri << std::endl;
		std::string resolvedPath = fileHandler.resolveFilePath(req.uri);

		if (fileHandler.isDirectory(resolvedPath))
			return handleStaticDirectory(fileHandler, ctx, resolvedPath);

		if (!fileHandler.fileExists(resolvedPath))
			return handleNotFound(ctx);

		return serveStaticFile(fileHandler, ctx, resolvedPath);
	}
	catch (const std::exception& e)
	{
		// All path errors, including escaping root, end up here
		return handleStaticError(fileHandler, ctx, e);
	}
}


	std::string handleStaticDirectory(FileHandler& fileHandler, const RequestContext& ctx, const std::string& path)
	{
		if (ctx.autoindex_enabled)
			return fileHandler.handleDirectory(path);

		return handleNotFound(ctx);
	}

std::string serveStaticFile(FileHandler& fileHandler, const RequestContext& /*ctx*/, const std::string& path)
{
    std::string body = fileHandler.serveFile(path);
    std::string mime = fileHandler.detectMimeType(path);

    std::cout << "[serveStaticFile] path = " << path << "\n";
    std::cout << "[serveStaticFile] body.size() = " << body.size() << "\n";
    std::cout << "[serveStaticFile] mime = " << mime << "\n";

    std::ostringstream resp;
    resp << "HTTP/1.1 200 OK\r\n";
    resp << "Content-Length: " << body.size() << "\r\n";
    resp << "Content-Type: " << mime << "\r\n\r\n";
    resp << body;

    std::string respStr = resp.str();
    std::cout << "[serveStaticFile] total response length = " << respStr.size() << "\n";

    // Optional: dump first 100 bytes to see content
    std::cout << "[serveStaticFile] response preview:\n";
    std::cout << respStr.substr(0, std::min<size_t>(100, respStr.size())) << "\n";

    return respStr;
}


	std::string handleStaticError(FileHandler& /*fileHandler*/, const RequestContext& ctx, const std::exception& e)
	{
		std::cerr << "[StaticHandler] File error: " << e.what() << "\n";

		// Example: catch path escape explicitly
		if (std::string(e.what()).find("Access outside root") != std::string::npos)
			return handleNotFound(ctx); // could return 403

		return handleNotFound(ctx); // generic 404
	}

	std::string handleNotFound(const RequestContext& /*ctx*/)
	{
		return "<html><body><h1>404 Not Found</h1></body></html>";
	}
}