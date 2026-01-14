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

		for (const auto& allowedMethod : allowed_methods)
		{
			DBG("[isMethodAllowed] Comparing with allowed method: " << httpMethodToString(allowedMethod));
			if (method == allowedMethod)
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
			if (status == HttpStatusCode::OK)
			{
				std::string requestedScript = ctx.resolved_path;
				
				DBG("[processGet] interpreter detected, checking file " << requestedScript << " for validity...");

				if (!checkScriptValidity(requestedScript, rawResp, ctx))
					return;
					
				(void)client;
				//CGIHandler::processCGI(req, client, interpreter, requestedScript, rawResp);

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
		
		// Default: no CGI decision has been made yet
		outStatus = HttpStatusCode::None;
		
		// Extract the file extension from the requested URI
		std::filesystem::path requestPath(uri);
		std::string ext = requestPath.extension().string();
		DBG("[getCgiPathFromUri] Extracted extension: \"" << ext << "\"");

		for (const auto &entry : cgi_pass)
		{
			DBG("[getCgiPathFromUri] Checking pair: key=\"" << entry.first
				<< "\", interpreter=\"" << entry.second << "\"");
			
			// Only act when the request extension matches a CGI configuration
			if (entry.first != ext)
			{
				DBG("[getCgiPathFromUri] Extension does not match -> continue");
				continue;
			}
			
			DBG("[getCgiPathFromUri] Extension matches");
			
			const std::string& interpreterPath = entry.second;

			 // 1. Check if the configured interpreter exists
			if (!std::filesystem::exists(interpreterPath))
			{
				DBG("[getCgiPathFromUri] Interpreter path does not exist: \"" 
					<< entry.second << "\" -> BadGateway");
				outStatus = HttpStatusCode::BadGateway;
				return "";
			}
			
			std::filesystem::path canonicalPath;

			// 2. Canonicalize the path to resolve symlinks and normalize it
			try
			{
				canonicalPath = std::filesystem::canonical(interpreterPath);
			}
			catch (const std::exception& e)
			{
				DBG("[getCgiPathFromUri] Canonicalization failed: "
					<< e.what() << " -> BadGateway");
				outStatus = HttpStatusCode::BadGateway;
				return "";
			}
			
			DBG("[getCgiPathFromUri] Canonicalized interpreter path: \""
				<< canonicalPath.string() << "\"");
			
			// 3. Verify that it is a regular file and executable
			if (!FileUtils::existsAndIsFile(canonicalPath) ||
				access(canonicalPath.c_str(), X_OK) != 0)
			{
				DBG("[getCgiPathFromUri] Interpreter is not a regular executable -> BadGateway");
				outStatus = HttpStatusCode::BadGateway;
				return "";
			}
			
			// All checks passed: valid CGI interpreter
			DBG("[getCgiPathFromUri] Interpreter path valid -> OK");
			outStatus = HttpStatusCode::OK;
			return canonicalPath.string();
		}
		
		// No matching CGI extension found
		DBG("[getCgiPathFromUri] No matching CGI extension found");
		return "";
	}
	
	// Helper function to check if a script file exists and is executable
	bool checkScriptValidity(const std::string& scriptPath, 
							RawResponse& rawResp, 
							const RequestContext& ctx)
	{
		// Check that the file exists and is a regular file
		if (!FileUtils::existsAndIsFile(scriptPath))
		{
			DBG("[checkScriptValidity] Script not found: " << scriptPath);
			rawResp.addErrorDetails(ctx, HttpStatusCode::NotFound);
			return false;
		}

		// Check that the file is executable
		if (access(scriptPath.c_str(), X_OK) != 0)
		{
			DBG("[checkScriptValidity] Script exists but is not executable: " << scriptPath);
			rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);
			return false;
		}

		// Passed checks
		DBG("[checkScriptValidity] Script is valid: " << scriptPath);
		return true;
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
		if (ctx.client_max_body_size != 0 && req.body.size() > ctx.client_max_body_size)
		{
			DBG("[processPost] Body too large: " << req.body.size()
					<< " > " << ctx.client_max_body_size);
			
			rawResp.addErrorDetails(ctx, HttpStatusCode::PayloadTooLarge);

			return;
		}
		
		if (!ctx.cgi_pass.empty())
		{
		
			HttpStatusCode status;
			std::string interpreter = getCgiPathFromUri(req.uri, ctx.cgi_pass, status);
			DBG("[processPost] CGI interpreter path: \"" << interpreter
				<< "\", status: " << static_cast<int>(status));

			if (status == HttpStatusCode::BadGateway)
			{
				rawResp.addErrorDetails(ctx, HttpStatusCode::BadGateway);
				return;
			}
			if (status == HttpStatusCode::OK)
			{
				std::string requestedScript = ctx.resolved_path;
				
				DBG("[processPost] interpreter detected, checking file " << requestedScript << " for validity...");

				if (!checkScriptValidity(requestedScript, rawResp, ctx))
					return;

				DBG("Executing CGI interpreter: " << interpreter
					<< " for script: " << requestedScript);

				(void)client;
				// CGIHandler::processCGI(req, client, interpreter, requestedScript, rawResp);
				return;
			}
		}

		if (ctx.upload_store.empty())
		{
			DBG("[processPost] Upload store not configured, forbidden");
			
			rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);
			return;
		}
		
		if (!FileUtils::existsAndIsDirectory(ctx.upload_store))
		{
			DBG("[processPost] Upload store file does not exist in the file system");
			rawResp.addErrorDetails(ctx, HttpStatusCode::InternalServerError);
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
