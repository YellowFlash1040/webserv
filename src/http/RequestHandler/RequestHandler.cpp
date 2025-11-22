#include "RequestHandler.hpp"
#include "../utils/utils.hpp"
#include "CGIHandler.hpp"

void RequestHandler::processRequest(RawRequest& rawReq,
									const NetworkEndpoint& endpoint,
									const RequestContext& ctx)
{
	DBG("[processRequest]:");
	
	// 0. Bad request
	if (rawReq.isBadRequest())
	{
		enqueueBadResponse();
		return;
	}
	
	bool shouldClose = rawReq.shouldClose();
	
	RequestData req = rawReq.buildRequestData();
	
	// 1. External redirection
	if (ctx.redirection.isSet)
	{
		RawResponse resp;
		handleExternalRedirect(ctx, req.uri, resp);
		clientState.enqueueRawResponse(resp, shouldClose);
		return;
	}

	// 2. Method not allowed
	if (!isMethodAllowed(req.method, ctx.allowed_methods))
	{
		DBG("[processRequest] Method not allowed "
				  << static_cast<int>(req.method));

		enqueueErrorResponse(ctx, HttpStatusCode::MethodNotAllowed, shouldClose);
		DBG("[processRequest] Enqueue \"Method Not Allowed\" response");

		return;
	}

	// 3. Dispatch by method
	RawResponse resp;

	switch (req.method)
	{
	case HttpMethod::GET:
		processGet(req, endpoint, ctx, resp);
		break;
	case HttpMethod::POST:
		processPost(req, endpoint, ctx, resp);
		break;
	case HttpMethod::DELETE:
		processDelete(req, endpoint, ctx, resp);
		break;
	default:
		addGeneralErrorDetails(resp, ctx, HttpStatusCode::MethodNotAllowed);
		break;
	}

	// Enqueue the response for sending
	clientState.enqueueRawResponse(resp, shouldClose);
}

void RequestHandler::enqueueBadResponse()
{
	RawResponse resp;
	
	resp.setStatus(HttpStatusCode::BadRequest);
	resp.addDefaultError(HttpStatusCode::BadRequest);
	clientState.enqueueRawResponse(resp, false); //keep alive
}

bool RequestHandler::isMethodAllowed(
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

void RequestHandler::processGet(RequestData& req, const NetworkEndpoint& endpoint, const RequestContext& ctx, RawResponse& rawResp)
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
			addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::BadGateway);
			return;
		}
		
		DBG("[processGet] interpreter detected, checking file " << ctx.resolved_path << " for validity...");

		if (!FileUtils::existsAndIsFile(ctx.resolved_path))
		{
			DBG("[processGet] CGI Uri not found: " << ctx.resolved_path);
			addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::NotFound);
			return;
		}
		if (access(ctx.resolved_path.c_str(), X_OK) != 0)
		{
			DBG("[processGet] CGI Uri exists but is not executable: " << ctx.resolved_path);
			addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::Forbidden);
			return;
		}
		
		if (status == HttpStatusCode::OK)
		{
			DBG("Executing CGI interpreter: " << interpreter << 
				" for script: " << ctx.resolved_path);
			CGIHandler::processCGI(req, endpoint, interpreter, ctx.resolved_path, rawResp);

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
			addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::Forbidden);
			return;
		}

		fillSuccessfulResponse(rawResp, ctx.resolved_path);
		
		DBG("[processGet] setting status to OK");
		rawResp.setStatus(HttpStatusCode::OK);

		rawResp.setMimeType(FileUtils::detectMimeType(ctx.resolved_path));

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
		addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::Forbidden);
		return;
	}

	// 5. Path does not exist at all
	DBG("[processGet] Path not found: " << ctx.resolved_path);
	addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::NotFound);
	return;
}

std::string RequestHandler::getCgiPathFromUri(
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

void RequestHandler::processDelete(RequestData& req, const NetworkEndpoint& endpoint, const RequestContext& ctx, RawResponse& resp)
{
	(void)req;
	(void)endpoint;

	DBG("[processDelete] Processing DELETE for path: "
			  << ctx.resolved_path);

	if (!FileUtils::pathExists(ctx.resolved_path))
	{
		DBG("[processDelete] Path does not exist: "
				  << ctx.resolved_path);
				  
		addGeneralErrorDetails(resp, ctx, HttpStatusCode::NotFound);

		resp.addHeader("Content-Length", "0");
		return;
	}

	if (FileUtils::existsAndIsDirectory(ctx.resolved_path))
	{
		DBG("[processDelete] Path is a directory, forbidden to delete: "
			<< ctx.resolved_path);
			
		addGeneralErrorDetails(resp, ctx, HttpStatusCode::Forbidden);
		
		resp.addHeader("Content-Length", "0");
		return;
	}

	if (!FileUtils::hasWritePermission(ctx.resolved_path))
	{
		DBG("[processDelete] No write permission for path: "
				  << ctx.resolved_path);
				  
		addGeneralErrorDetails(resp, ctx, HttpStatusCode::Forbidden);
		resp.addHeader("Content-Length", "0");
		return;
	}

	if (FileUtils::deleteFile(ctx.resolved_path))
	{
		DBG("[processDelete] File deleted successfully: "
				  << ctx.resolved_path);
		resp.setStatus(HttpStatusCode::NoContent);
		resp.addHeader("Content-Length", "0");
	}
	else
	{
		DBG("[processDelete] Failed to delete file: "
				  << ctx.resolved_path);
		
		addGeneralErrorDetails(resp, ctx, HttpStatusCode::InternalServerError);
		
		resp.addHeader("Content-Length", "0");
	}

	return;
}

// Body size gets filled for the default errors
void RequestHandler::addGeneralErrorDetails(RawResponse& resp,
											const RequestContext& ctx,
											HttpStatusCode code)
{
	DBG("[addGeneralErrorDetails] Called with code: " << static_cast<int>(code));

	resp.setStatusCode(code);
	DBG("[addGeneralErrorDetails] Code set to " << static_cast<int>(code));

	auto it = ctx.error_pages.find(code);
	if (it != ctx.error_pages.end())
	{
		DBG("[addGeneralErrorDetails] Found entry in ctx.error_pages for "
			<< static_cast<int>(code) << ": \"" << it->second << "\"");

		if (!it->second.empty())
		{
			DBG("[addGeneralErrorDetails] Using CUSTOM error page: " << it->second);
			resp.setInternalRedirect(true);
			resp.setRedirectTarget(it->second);

			DBG("[addGeneralErrorDetails] Marked as internal redirect. Target set to: "
				<< resp.getRedirectTarget());
			return;
		}
		else
		{
			DBG("[addGeneralErrorDetails] Entry found but empty path for code "
				<< static_cast<int>(code));
		}
	}
	else
	{
		DBG("[addGeneralErrorDetails] No custom error page entry found for code "
			<< static_cast<int>(code));
	}

	DBG("[addGeneralErrorDetails] Generating DEFAULT error page for "
		<< static_cast<int>(code));

	resp.addDefaultError(code);
	resp.addHeader("Content-Type", "text/html");
}

void RequestHandler::enqueueErrorResponse(const RequestContext& ctx,
										  HttpStatusCode status,  bool shouldClose)
{
	RawResponse rawResp;
	rawResp.setStatus(status);
	
	if (shouldClose)
        rawResp.addHeader("Connection", "close");

	DBG("[enqueueErrorResponse] Called with status "
			  << static_cast<int>(status));

	// Check if a custom error page exists
	auto it = ctx.error_pages.find(status);
	if (it != ctx.error_pages.end() && !it->second.empty())
	{
		std::string pageUri = it->second;
		DBG("[enqueueErrorResponse] Custom error page found: "
				  << pageUri);

		// Instead of enqueuing here, mark internal redirect
		rawResp.setInternalRedirect(true);
		rawResp.setFilePath(pageUri); // optional debug

		DBG("[enqueueErrorResponse] Internal redirect set, custom page will be handled in ConnectionManager");
		clientState.enqueueRawResponse(rawResp, shouldClose); // still enqueue signal
		return;
	}

	// No custom page, generate default
	rawResp.addDefaultError(status);
	rawResp.addHeader("Content-Type", "text/html");
	clientState.enqueueRawResponse(rawResp, shouldClose);

	DBG("[enqueueErrorResponse] No custom page, default error page "
				 "generated for status "
			  << static_cast<int>(status));
}

std::string RequestHandler::getErrorPageUri(const RequestContext& ctx,
											HttpStatusCode status) const
{
	auto it = ctx.error_pages.find(status);
	if (it != ctx.error_pages.end() && !it->second.empty())
	{
		return it->second; // e.g., "/errors/405.html"
	}
	return "";
}

void RequestHandler::processPost(RequestData& req,
								 const NetworkEndpoint& endpoint,
								 const RequestContext& ctx, RawResponse& resp)
{
	HttpStatusCode status;
	std::string interpreter = getCgiPathFromUri(req.uri, ctx.cgi_pass, status);

	DBG("[processPost] Processing POST for path: " << ctx.resolved_path);

	if (req.body.size() > ctx.client_max_body_size)
	{
		DBG("[processPost] Body too large: " << req.body.size()
				  << " > " << ctx.client_max_body_size);
		
		addGeneralErrorDetails(resp, ctx, HttpStatusCode::PayloadTooLarge);

		return;
	}

	if (!ctx.cgi_pass.empty())
	{
		DBG("[processPost] CGI configured, executing...");
		CGIHandler::processCGI(req, endpoint, interpreter, ctx.resolved_path, resp);

		return;
	}

	if (ctx.upload_store.empty())
	{
		DBG("[processPost] Upload store not configured, forbidden");
		
		addGeneralErrorDetails(resp, ctx, HttpStatusCode::Forbidden);
		return;
	}

	processUpload(req, ctx, resp);
	DBG("[processPost] Upload processed, body size: "
			  << resp.getBody().size());

	return;
}


void RequestHandler::processUpload(RequestData& req, const RequestContext& ctx,
								   RawResponse& resp)
{
	(void)req;
	(void)ctx;
	(void)resp;

}



std::string RequestHandler::httpMethodToString(HttpMethod method)
{
	switch (method)
	{
		case HttpMethod::GET:    return "GET";
		case HttpMethod::POST:   return "POST";
		case HttpMethod::DELETE: return "DELETE";
		default:                 return "NONE";
	}
}

void RequestHandler::fillSuccessfulResponse(RawResponse& resp, const std::string& filePath)
{
	resp.setStatus(HttpStatusCode::OK);
	
    // Set MIME type
    resp.setMimeType(FileUtils::detectMimeType(filePath));

    // Decide delivery mode and set body or file path
    struct stat s;
    if (stat(filePath.c_str(), &s) != 0)
    {
        // Fallback if file disappears between checks
        resp.setFileMode(FileDeliveryMode::Streamed);
        resp.setFileSize(0);
        resp.setFilePath(filePath);
        return;
    }

    size_t fileSize = static_cast<size_t>(s.st_size);
    FileDeliveryMode mode = (fileSize <= MAX_IN_MEMORY_FILE_SIZE)
                                ? FileDeliveryMode::InMemory
                                : FileDeliveryMode::Streamed;
    resp.setFileMode(mode);
    resp.setFileSize(fileSize);

    if (mode == FileDeliveryMode::InMemory)
    {
        resp.setBody(FileUtils::readFileToString(filePath));
        resp.addHeader("Content-Length", std::to_string(fileSize));
    }
    else
    {
        resp.setFilePath(filePath);
    }
}

void RequestHandler::fillAutoindexResponse(RawResponse& resp, const std::string& dirPath)
{
    resp.setStatus(HttpStatusCode::OK);
    resp.setMimeType("text/html");
    resp.setFileMode(FileDeliveryMode::InMemory);

    std::string body = FileUtils::generateAutoindex(dirPath);
	//add catch?
    resp.setBody(body);
    resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));
}

void RequestHandler::handleExternalRedirect(const RequestContext& ctx, std::string& reqUri, RawResponse& rawResp)
{
	if (ctx.redirection.url == reqUri)
	{
		// Handle self-redirect differently
		rawResp.setStatus(HttpStatusCode::LoopDetected);
		rawResp.setBody("<html><head><title>Error</title></head>"
				"<body>Redirection loop detected for " + ctx.redirection.url + "</body></html>");
	}
	else
	{
		// Standard external redirection
		rawResp.setExternalRedirect(true);
		rawResp.setRedirectTarget(ctx.redirection.url);
		rawResp.setStatus(ctx.redirection.statusCode);
		rawResp.setBody("<html><head><title>Moved</title></head>"
				"<body>Redirection in progress. <a href=\"" + ctx.redirection.url + "\">Click here</a></body></html>");

	}
	rawResp.addHeader("Location", ctx.redirection.url);
	rawResp.addHeader("Content-Length", std::to_string(rawResp.getBody().size()));
	rawResp.addHeader("Content-Type", "text/html");
}
