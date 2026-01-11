#include "ResponseGenerator.hpp"


namespace ResponseGenerator
{
	void genResponse(const RawRequest& rawReq, const Client& client,
		const RequestContext& ctx, RawResponse& rawResp)
	{
		DBG("[processRequest]:");
		
		bool shouldClose = rawReq.shouldClose();
			if (shouldClose)
				rawResp.addHeader("Connection", "close");
		
		// 0. Bad request
		if (rawReq.isBadRequest())
		{
			rawResp.addErrorDetails(ctx, HttpStatusCode::BadRequest);
			return;
		}
		
		RequestData req = rawReq.buildRequestData();
		
		// 1. External redirection
		if (ctx.redirection.isSet)
		{
			handleExternalRedirect(ctx, req.uri, rawResp);
			return;
		}

		// 2. Method not allowed
		if (!isMethodAllowed(req.method, ctx.allowed_methods))
		{
			DBG("[processRequest] Method not allowed "
				<< static_cast<int>(req.method));

			rawResp.addErrorDetails(ctx, HttpStatusCode::MethodNotAllowed);
			addAllowHeader(rawResp, ctx.allowed_methods);

			return;
		}

		// 3. Dispatch by method
		switch (req.method)
		{
		case HttpMethod::GET:
			processGet(req, client, ctx, rawResp);
			break;
		case HttpMethod::POST:
			processPost(req, client, ctx, rawResp);
			break;
		case HttpMethod::DELETE:
			processDelete(req, client, ctx, rawResp);
			break;
		default:
			rawResp.addErrorDetails(ctx, HttpStatusCode::MethodNotAllowed);
			break;
		}
	}

	bool isMethodAllowed(
		HttpMethod method, const std::vector<HttpMethod>& allowed_methods)
	{
		DBG("[isMethodAllowed] Checking if method " << httpMethodToString(method)
			<< " is allowed");

		for (std::vector<HttpMethod>::const_iterator it = allowed_methods.begin();
			it != allowed_methods.end(); ++it)
		{
			DBG("[isMethodAllowed] Comparing with allowed method: " << httpMethodToString(*it));
			if (*it == method)
			{
				DBG("[isMethodAllowed] Method allowed!");
				return true;
			}
		}

		DBG("[isMethodAllowed] Method NOT allowed");
		return false;
	}

	void addAllowHeader(RawResponse& rawResp,
	                    const std::vector<HttpMethod>& allowed_methods)
	{
	    std::string allowed;

	    allowed += httpMethodToString(allowed_methods[0]);
	    for (size_t i = 1; i < allowed_methods.size(); ++i)
	    {
	        allowed += ", ";
	        allowed += httpMethodToString(allowed_methods[i]);
	    }
	    rawResp.addHeader("Allow", allowed);
	}

	void processGet(RequestData& req, const Client& client, const RequestContext& ctx, RawResponse& rawResp)
	{
		DBG("[processGet] Processing request for resolved path: " << ctx.resolved_path);

		// 1. Check CGI first
		if (!ctx.cgi_pass.empty())
		{
			HttpStatusCode status;
			std::string interpreter = getCgiPathFromUri(req.uri, ctx.cgi_pass, status);
			
			DBG("[processGet] CGI interpreter path: \"" << interpreter
				<< "\", status: " << static_cast<int>(status));


			if (status == HttpStatusCode::BadGateway)
			{
				rawResp.addErrorDetails(ctx, HttpStatusCode::BadGateway);
				return;
			}
			
			DBG("[processGet] interpreter detected, checking file " << ctx.resolved_path << " for validity...");

			if (!FileUtils::existsAndIsFile(ctx.resolved_path))
			{
				DBG("[processGet] CGI Uri not found: " << ctx.resolved_path);
				rawResp.addErrorDetails(ctx, HttpStatusCode::NotFound);
				return;
			}
			if (access(ctx.resolved_path.c_str(), X_OK) != 0)
			{
				DBG("[processGet] CGI Uri exists but is not executable: " << ctx.resolved_path);
				rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);
				return;
			}
			
			if (status == HttpStatusCode::OK)
			{
				DBG("Executing CGI interpreter: " << interpreter << 
					" for script: " << ctx.resolved_path);
					
				(void)client;
				//CGIHandler::processCGI(req, client, interpreter, ctx.resolved_path, rawResp);

				return;
			}
		}

		//no CGI mapping -> continue to static file handling

		// 2. Static file handling
		if (FileUtils::existsAndIsFile(ctx.resolved_path))
		{
			DBG("[processGet] Detected: file: " << ctx.resolved_path);

			if (access(ctx.resolved_path.c_str(), R_OK) != 0)
			{
				DBG("[processGet] File exists but not readable");
				rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);
				return;
			}

			fillSuccessfulResponse(rawResp, ctx.resolved_path);

			DBG("[processGet] Served static file with status: " << static_cast<int>(rawResp.getStatusCode())
			<< " (might change later if it is an internal redirection");
			
			return;

		}

		// 3. Directory handling
		if (FileUtils::existsAndIsDirectory(ctx.resolved_path))
		{
			DBG("[processGet] Detected: directory: " << ctx.resolved_path);

			// Try to find index file
			std::string indexPath = FileUtils::getFirstValidIndexFile(ctx.resolved_path, ctx.index_files);
			if (!indexPath.empty())
			{
				// Serve index file as static file
				fillSuccessfulResponse(rawResp, indexPath);
				return;
			}

			// Generate autoindex
			if (ctx.autoindex_enabled)
			{
				DBG("[processGet] No index file found, generating autoindex...");
		
				fillAutoindexResponse(rawResp, ctx.resolved_path);
				return;
			}
			// No index, autoindex disabled: 403 Forbidden
			DBG("[processGet] No index file, autoindex disabled -> Forbidden");
			rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);
			return;
		}

		// 5. Path does not exist at all
		DBG("[processGet] Path not found: " << ctx.resolved_path);
		rawResp.addErrorDetails(ctx, HttpStatusCode::NotFound);
		return;
	}

	std::string getCgiPathFromUri(
		const std::string& uri,
		const std::map<std::string, std::string>& cgi_pass,
		HttpStatusCode& outStatus)
	{
		DBG("[getCgiPathFromUri] Called with URI: \"" << uri << "\"");
		outStatus = HttpStatusCode::None;

		if (cgi_pass.empty())
		{
			DBG("[getCgiPathFromUri] cgi_pass empty → no CGI");
			return "";
		}

		std::filesystem::path path(uri);
		std::string ext = path.extension().string();
		DBG("[getCgiPathFromUri] Extracted extension: \"" << ext << "\"");

		for (const auto &entry : cgi_pass)
		{
			DBG("[getCgiPathFromUri] Checking pair: key=\"" << entry.first
				<< "\", interpreter=\"" << entry.second << "\"");

			if (entry.first == ext)
			{
				DBG("[getCgiPathFromUri] Extension matches");

				// Check if interpreter exists
				if (!std::filesystem::exists(entry.second))
				{
					DBG("[getCgiPathFromUri] Interpreter path does not exist: \"" 
						<< entry.second << "\" → BadGateway");
					outStatus = HttpStatusCode::BadGateway;
					return "";
				}

				// Canonicalize interpreter path
				std::filesystem::path canonicalPath =
					std::filesystem::canonical(entry.second);

				DBG("[getCgiPathFromUri] Canonicalized interpreter path: \"" 
					<< canonicalPath.string() << "\"");

				// Verify it stays inside /usr/bin/
				if (canonicalPath.string().compare(
						0,
						std::string("/usr/bin/").size(),
						"/usr/bin/") != 0)
				{
					DBG("[getCgiPathFromUri] Interpreter path escapes /usr/bin/ → BadGateway");
					outStatus = HttpStatusCode::BadGateway;
					return "";
				}

				DBG("[getCgiPathFromUri] Interpreter path valid -> OK");
				outStatus = HttpStatusCode::OK;
				return entry.second;
			}
			else
			{
				DBG("[getCgiPathFromUri] Extension does not match key: \"" 
					<< entry.first << "\" -> continue");
			}
		}

		DBG("[getCgiPathFromUri] No matching CGI extension found");
		return "";
	}

	void processDelete(RequestData& req, const Client& client, const RequestContext& ctx, RawResponse& rawResp)
	{
		(void)req;
		(void)client;

		DBG("[processDelete] Processing DELETE for path: "
				<< ctx.resolved_path);

		if (!FileUtils::pathExists(ctx.resolved_path))
		{
			DBG("[processDelete] Path does not exist: "
					<< ctx.resolved_path);
					
			rawResp.addErrorDetails(ctx, HttpStatusCode::NotFound);
			return;
		}

		if (FileUtils::existsAndIsDirectory(ctx.resolved_path))
		{
			DBG("[processDelete] Path is a directory, forbidden to delete: "
				<< ctx.resolved_path);
				
			rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);
		
			return;
		}

		if (!FileUtils::hasWritePermission(ctx.resolved_path))
		{
			DBG("[processDelete] No write permission for path: "
					<< ctx.resolved_path);
					
			rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);

			return;
		}

		if (FileUtils::deleteFile(ctx.resolved_path))
		{
			DBG("[processDelete] File deleted successfully: "
					<< ctx.resolved_path);
			rawResp.setStatusCode(HttpStatusCode::NoContent);
			//should not have the Content-Length header!
		}
		else
		{
			DBG("[processDelete] Failed to delete file: "
					<< ctx.resolved_path);
			
			rawResp.addErrorDetails(ctx, HttpStatusCode::InternalServerError);
		}

		return;
	}

	void processPost(RequestData& req,
									const Client& client,
									const RequestContext& ctx, RawResponse& rawResp)
	{
		HttpStatusCode status;
		std::string interpreter = getCgiPathFromUri(req.uri, ctx.cgi_pass, status);

		DBG("[processPost] Processing POST for path: " << ctx.resolved_path);

		if (ctx.client_max_body_size != 0 && req.body.size() > ctx.client_max_body_size)
		{
			DBG("[processPost] Body too large: " << req.body.size()
					<< " > " << ctx.client_max_body_size);
			
			rawResp.addErrorDetails(ctx, HttpStatusCode::PayloadTooLarge);

			return;
		}

		if (!ctx.cgi_pass.empty())
		{
			DBG("[processPost] CGI configured, executing...");
			
			(void)client;
			//CGIHandler::processCGI(req, client, interpreter, ctx.resolved_path, rawResp);

			return;
		}

		if (ctx.upload_store.empty())
		{
			DBG("[processPost] Upload store not configured, forbidden");
			
			rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);
			return;
		}

		UploadModule::processUpload(req, ctx, rawResp);
		DBG("[processPost] Upload processed, body size: "
				<< rawResp.getBody().size());

		return;
	}

	void fillSuccessfulResponse(RawResponse& resp, const std::string& filePath)
	{
		resp.setStatusCode(HttpStatusCode::OK);
		
		// Set MIME type
		resp.setMimeType(FileUtils::detectMimeType(filePath));

		// Decide delivery mode and set body or file path
		struct stat s;
		if (stat(filePath.c_str(), &s) != 0)
		{
			// Fallback if file disappears between checks
			resp.setFileSize(0);
			return;
		}

		size_t fileSize = static_cast<size_t>(s.st_size);
		resp.setFileSize(fileSize);
		resp.setBody(FileUtils::readFileToString(filePath));
		resp.addHeader("Content-Length", std::to_string(fileSize));
	}

	void fillAutoindexResponse(RawResponse& resp, const std::string& dirPath)
	{
		resp.setStatusCode(HttpStatusCode::OK);
		resp.setMimeType("text/html");

		std::string body = FileUtils::generateAutoindex(dirPath);
		//add catch?
		resp.setBody(body);
		resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));
	}

	void handleExternalRedirect(const RequestContext& ctx, std::string& reqUri, RawResponse& rawResp)
	{
		if (ctx.redirection.url == reqUri)
		{
			// Handle self-redirect differently
			rawResp.setStatusCode(HttpStatusCode::LoopDetected);
			rawResp.setBody("<html><head><title>Error</title></head>"
					"<body>Redirection loop detected for " + ctx.redirection.url + "</body></html>");
		}
		else
		{
			// Standard external redirection
			rawResp.setStatusCode(ctx.redirection.statusCode);
			rawResp.addHeader("Location", ctx.redirection.url);
			rawResp.addHeader("Content-Length", std::to_string(rawResp.getBody().size()));
			rawResp.addHeader("Content-Type", "text/html");
			rawResp.setBody("");
		}
	}
}

