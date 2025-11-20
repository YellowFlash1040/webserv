#include "RequestHandler.hpp"

void RequestHandler::processRequest(RawRequest& rawReq,
									const NetworkEndpoint& endpoint,
									const RequestContext& ctx)
{
	DBG("[processRequest]:");
	
	// 0. Bad request
	if (rawReq.isBadRequest())
	{
		serveBadRequest(rawReq, endpoint, ctx);
		return;
	}

	RequestData req = rawReq.buildRequestData();

	// 1. External redirection
	if (ctx.redirection.isSet)
	{
		RawResponse resp(req, ctx);
		resp.handleExternalRedirect(req.uri);
		clientState.enqueueRawResponse(resp);
		return;
	}

	// 2. Method not allowed
	if (rawReq.getUri() != "/favicon.ico"
		&& !isMethodAllowed(req.method, ctx.allowed_methods))
	{
		DBG("[processRequest] Method not allowed "
				  << static_cast<int>(req.method));

		enqueueErrorResponse(ctx, HttpStatusCode::MethodNotAllowed);
		DBG("[processRequest] Enqueue \"Method Not Allowed\" response");

		return;
	}

	// 3. Dispatch by method
	RawResponse resp(req, ctx);

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
	clientState.enqueueRawResponse(resp);
}

void RequestHandler::serveBadRequest(RawRequest& rawReq,
									 const NetworkEndpoint& endpoint,
									 const RequestContext& ctx)
{
	(void)endpoint;
	(void)ctx;
	(void)rawReq;
	RawResponse resp; // default constructor, OK for bad requests
	resp.setStatus(HttpStatusCode::BadRequest);
	resp.addDefaultErrorDetails(HttpStatusCode::BadRequest);
	clientState.enqueueRawResponse(resp);
}

bool RequestHandler::isMethodAllowed(
    HttpMethod method, const std::vector<HttpMethod>& allowed_methods)
{
    DBG("[isMethodAllowed] Checking if method " << RawResponse::httpMethodToString(method)
        << " is allowed among " << allowed_methods.size() << " methods");

    for (std::vector<HttpMethod>::const_iterator it = allowed_methods.begin();
         it != allowed_methods.end(); ++it)
    {
        DBG("[isMethodAllowed] Comparing with allowed method: " << RawResponse::httpMethodToString(*it));
        if (*it == method)
        {
            DBG("[isMethodAllowed] Method allowed!");
            return true;
        }
    }

    DBG("[isMethodAllowed] Method NOT allowed");
    return false;
}

void RequestHandler::processGet(RequestData& req,
								const NetworkEndpoint& endpoint,
								const RequestContext& ctx, RawResponse& rawResp)
{
	(void)endpoint;
	(void)req;

	FileHandler fileHandler(ctx.index_files);

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

		switch (status)
		{
			case HttpStatusCode::None: DBG("None"); break;
			case HttpStatusCode::OK: DBG("OK"); break;
			case HttpStatusCode::BadGateway: DBG("BadGateway"); break;
			case HttpStatusCode::Forbidden: DBG("Forbidden"); break;
			default: DBG("Other"); break;
		}	

		DBG("[processGet] interpreter detected, checking file " << ctx.resolved_path << " for validity...");

		if (!fileHandler.existsAndIsFile(ctx.resolved_path))
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
			DBG("Executing CGI interpreter: " << interpreter << " for script: " << ctx.resolved_path);
			std::string cgiResult = handleCGI(req, endpoint, interpreter, ctx.resolved_path);

			rawResp.setBody(cgiResult);
			rawResp.setStatus(HttpStatusCode::OK);
			rawResp.setMimeType("text/plain");
			return;
		}
	}

	// status == None -> no CGI mapping -> continue to static file handling

	// 2. Static file handling
	if (fileHandler.existsAndIsFile(ctx.resolved_path))
	{
		DBG("[processGet] Detected: file: " << ctx.resolved_path);

		if (access(ctx.resolved_path.c_str(), R_OK) != 0)
		{
			DBG("[processGet] File exists but not readable");
			addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::Forbidden);
			return;
		}

		DBG("[processGet] setting status to OK");
		rawResp.setStatus(HttpStatusCode::OK);

		rawResp.setMimeType(fileHandler.detectMimeType(ctx.resolved_path));
		setFileDelivery(rawResp, ctx.resolved_path, fileHandler);

		DBG("[processGet] Served static file with status: " << static_cast<int>(rawResp.getStatusCode()));
		DBG("[processGet] might change later if it is an internal redirection");
		return;
	}

	// 3. Directory handling
	if (fileHandler.existsAndIsDirectory(ctx.resolved_path))
	{
		DBG("[processGet] Detected: directory: " << ctx.resolved_path);

		// Try to find index file
		std::string indexPath = fileHandler.getIndexFilePath(ctx.resolved_path);
		if (!indexPath.empty())
		{
			// Serve index file as static file
			DBG("[processGet] Found index file: " << indexPath);
			rawResp.setStatus(HttpStatusCode::OK);
			rawResp.setMimeType(fileHandler.detectMimeType(indexPath));
			setFileDelivery(rawResp, indexPath, fileHandler);

			return;
		}

		// generate autoindex
		if (ctx.autoindex_enabled)
		{
			DBG("[processGet] No index file found, generating autoindex...");
			rawResp.setStatus(HttpStatusCode::OK);
			rawResp.setMimeType("text/html");
			rawResp.setFileMode(FileDeliveryMode::InMemory);

			// ADD catching
			rawResp.setBody(fileHandler.generateAutoindex(ctx.resolved_path));
			rawResp.addHeader("Content-Length",
							  std::to_string(rawResp.getBody().size()));
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

std::string RequestHandler::handleCGI(RequestData& req,
									  const NetworkEndpoint& endpoint,
									  const std::string& interpreter,
									  const std::string& scriptPath)
{
	DBG("[handleCGI] scriptPath = " << scriptPath
		  << ", interpreter = " << interpreter);

	std::vector<std::string> args;
	args.push_back(scriptPath);

	std::vector<std::string> env = CGI::buildEnvFromRequest(req);
	env.push_back("SCRIPT_FILENAME=" + scriptPath);

	NetworkInterface localIp = endpoint.ip();
	int localPort = endpoint.port();

	env.push_back("SERVER_ADDR=" + static_cast<std::string>(localIp));
	env.push_back("SERVER_PORT=" + std::to_string(localPort));

	env.push_back("REMOTE_ADDR="); // TODO: add client IP
	env.push_back("REMOTE_PORT="); // TODO: addd client port

	std::string host = req.getHeader("Host");
	if (!host.empty())
		env.push_back("SERVER_NAME=" + host);
	else
		env.push_back("SERVER_NAME=" + static_cast<std::string>(localIp));

	const std::string& input = req.body;

	try
	{
		return CGI::execute(interpreter, args, env, input, "./www/site1/py");
	}
	catch (const std::exception& e)
	{
		return std::string("Content-Type: text/plain\r\n\r\nCGI error: ") + e.what();
	}
}

void RequestHandler::setFileDelivery(RawResponse& resp, const std::string& path,
									 FileHandler& fileHandler)
{
	DeliveryInfo info = fileHandler.getDeliveryInfo(path, 1024 * 1024);
	
	resp.setFileMode(info.mode);       // always set mode
    resp.setFileSize(info.size);       // store file size
	
	if (info.mode == FileDeliveryMode::InMemory)
	{
		resp.setBody(readFileToString(path));
		resp.addHeader("Content-Length", std::to_string(info.size));
	}
	else
	{
		resp.setFilePath(path);
		//body is left empty; Content-Length comes from _fileSize
	}
}

void RequestHandler::processDelete(RequestData& req,
								   const NetworkEndpoint& endpoint,
								   const RequestContext& ctx, RawResponse& resp)
{
	(void)req;
	(void)resp;
	(void)endpoint;

	DBG("[processDelete] Processing DELETE for path: "
			  << ctx.resolved_path);

	if (!FileHandler::pathExists(ctx.resolved_path))
	{
		DBG("[processDelete] Path does not exist: "
				  << ctx.resolved_path);
		enqueueErrorResponse(ctx, HttpStatusCode::NotFound);
		resp.addHeader("Content-Length", "0");
		return;
	}

	if (FileHandler::existsAndIsDirectory(ctx.resolved_path))
	{
		DBG("[processDelete] Path is a directory, forbidden to delete: "
			<< ctx.resolved_path);
		enqueueErrorResponse(ctx, HttpStatusCode::Forbidden);
		resp.addHeader("Content-Length", "0");
		return;
	}

	if (!FileHandler::hasWritePermission(ctx.resolved_path))
	{
		DBG("[processDelete] No write permission for path: "
				  << ctx.resolved_path);
		enqueueErrorResponse(ctx, HttpStatusCode::Forbidden);
		resp.addHeader("Content-Length", "0");
		return;
	}

	if (FileHandler::deleteFile(ctx.resolved_path))
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
		enqueueErrorResponse(ctx, HttpStatusCode::InternalServerError);
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

	resp.addDefaultErrorDetails(code);
	resp.addHeader("Content-Type", "text/html");
}

void RequestHandler::enqueueErrorResponse(const RequestContext& ctx,
										  HttpStatusCode status)
{
	RawResponse rawResp;
	rawResp.setStatus(status);

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
		clientState.enqueueRawResponse(rawResp); // still enqueue signal
		return;
	}

	// No custom page, generate default
	rawResp.addDefaultErrorDetails(status);
	rawResp.addHeader("Content-Type", "text/html");
	clientState.enqueueRawResponse(rawResp);

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
	(void)endpoint;
	DBG("[processPost] Processing POST for path: " << ctx.resolved_path);

	if (req.body.size() > ctx.client_max_body_size)
	{
		DBG("[processPost] Body too large: " << req.body.size()
				  << " > " << ctx.client_max_body_size);
		enqueueErrorResponse(ctx, HttpStatusCode::PayloadTooLarge);
		return;
	}

	if (!ctx.cgi_pass.empty())
	{
		DBG("[processPost] CGI configured, executing...");
		//handleCGI(req, endpoint, interpreter, ctx.resolved_path);
		return;
	}

	if (ctx.upload_store.empty())
	{
		DBG("[processPost] Upload store not configured, forbidden");
		enqueueErrorResponse(ctx, HttpStatusCode::Forbidden);
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

std::string RequestHandler::readFileToString(const std::string& path)
{
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file)
	{
		DBG("[readFileToString] Failed to open: " << path);
		return "";
	}
	std::ostringstream contents;
	contents << file.rdbuf();
	DBG("[readFileToString] Read " << contents.str().size()
			  << " bytes from " << path);
	return contents.str();
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
