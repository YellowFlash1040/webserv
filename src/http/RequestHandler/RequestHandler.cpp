#include "RequestHandler.hpp"

void RequestHandler::processRequest(RawRequest& rawReq,
									const NetworkEndpoint& endpoint,
									RequestContext& ctx)
{
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
		std::cout << "[processRequest] Method not allowed: "
				  << static_cast<int>(req.method) << std::endl;

		enqueueErrorResponse(ctx, HttpStatusCode::MethodNotAllowed);
		std::cout << "[processRequest] Enqueue \"Method Not Allowed\" response"
				  << std::endl;

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
									 RequestContext& ctx)
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
	for (std::vector<HttpMethod>::const_iterator it = allowed_methods.begin();
		 it != allowed_methods.end(); ++it)
	{
		if (*it == method)
			return true;
	}
	return false;
}

void RequestHandler::processGet(RequestData& req,
								const NetworkEndpoint& endpoint,
								RequestContext& ctx, RawResponse& rawResp)
{
	(void)endpoint;
	(void)req;

	FileHandler fileHandler(ctx.autoindex_enabled, ctx.index_files);

	std::cout << "[processGet] Processing request for path: "
			  << ctx.resolved_path << std::endl;

	// 1. Check CGI first
	if (!ctx.cgi_pass.empty())
	{
		std::cout << "[processGet] Detected CGI configuration, checking file "
					 "validity..."
				  << std::endl;

		if (!fileHandler.existsAndIsFile(ctx.resolved_path))
		{
			std::cout << "[processGet] CGI target not found: "
					  << ctx.resolved_path << std::endl;
			addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::NotFound);
			return;
		}
		if (access(ctx.resolved_path.c_str(), X_OK) != 0)
		{
			std::cout
				<< "[processGet] CGI target exists but is not executable: "
				<< ctx.resolved_path << std::endl;
			addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::Forbidden);
			return;
		}

		std::cout << "[processGet] Running CGI handler for: "
				  << ctx.resolved_path << std::endl;
		// handleCGI(req, endpoint);
		return;
	}

	// 2. Static file handling
	if (fileHandler.existsAndIsFile(ctx.resolved_path))
	{
		std::cout << "[processGet] Detected: file: " << ctx.resolved_path
				  << std::endl;

		if (access(ctx.resolved_path.c_str(), R_OK) != 0)
		{
			std::cout << "[processGet] File exists but not readable"
					  << std::endl;
			addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::Forbidden);
			return;
		}

		std::cout << "[processGet] setting status to OK" << std::endl;
		rawResp.setStatus(HttpStatusCode::OK);

		rawResp.setMimeType(fileHandler.detectMimeType(ctx.resolved_path));
		setFileDelivery(rawResp, ctx.resolved_path, fileHandler);

		std::cout << "[processGet] Served static file with status: "
				  << static_cast<int>(rawResp.getStatusCode()) << std::endl;
		return;
	}

	// 3. Directory handling
	if (fileHandler.existsAndIsDirectory(ctx.resolved_path))
	{
		std::cout << "[processGet] Detected: directory: " << ctx.resolved_path
				  << std::endl;

		// Try to find index file
		std::string indexPath = fileHandler.getIndexFilePath(ctx.resolved_path);
		if (!indexPath.empty())
		{
			// Serve index file as static file
			std::cout << "[processGet] Found index file: " << indexPath
					  << std::endl;
			rawResp.setStatus(HttpStatusCode::OK);
			rawResp.setMimeType(fileHandler.detectMimeType(indexPath));
			setFileDelivery(rawResp, indexPath, fileHandler);

			return;
		}

		// generate autoindex
		if (ctx.autoindex_enabled)
		{
			std::cout
				<< "[processGet] No index file found, generating autoindex..."
				<< std::endl;
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
		std::cout
			<< "[processGet] No index file, autoindex disabled -> Forbidden"
			<< std::endl;
		addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::Forbidden);
		return;
	}
	// 4. Path does not exist at all
	std::cout << "[processGet] Path not found: " << ctx.resolved_path
			  << std::endl;
	addGeneralErrorDetails(rawResp, ctx, HttpStatusCode::NotFound);
	return;
}

//implement
std::string RequestHandler::handleCGI(RequestData& req, const NetworkEndpoint& endpoint)
{
	(void)req;
	(void)endpoint;
	return "";
}

void RequestHandler::setFileDelivery(RawResponse& resp, const std::string& path,
									 FileHandler& fileHandler)
{
	DeliveryInfo info = fileHandler.getDeliveryInfo(path, 1024 * 1024);

	if (info.mode == FileDeliveryMode::InMemory)
	{
		resp.setFileMode(FileDeliveryMode::InMemory);
		resp.setBody(readFileToString(path));
		resp.addHeader("Content-Length", std::to_string(info.size));
	}
	else
	{
		resp.setFileMode(FileDeliveryMode::Streamed);
		resp.setFilePath(path);
	}
}

void RequestHandler::processDelete(RequestData& req,
								   const NetworkEndpoint& endpoint,
								   RequestContext& ctx, RawResponse& resp)
{
	(void)req;
	(void)resp;
	(void)endpoint;

	std::cout << "[processDelete] Processing DELETE for path: "
			  << ctx.resolved_path << std::endl;

	if (!FileHandler::pathExists(ctx.resolved_path))
	{
		std::cout << "[processDelete] Path does not exist: "
				  << ctx.resolved_path << std::endl;
		enqueueErrorResponse(ctx, HttpStatusCode::NotFound);
		resp.addHeader("Content-Length", "0");
		return;
	}

	if (FileHandler::existsAndIsDirectory(ctx.resolved_path))
	{
		std::cout
			<< "[processDelete] Path is a directory, forbidden to delete: "
			<< ctx.resolved_path << std::endl;
		enqueueErrorResponse(ctx, HttpStatusCode::Forbidden);
		resp.addHeader("Content-Length", "0");
		return;
	}

	if (!FileHandler::hasWritePermission(ctx.resolved_path))
	{
		std::cout << "[processDelete] No write permission for path: "
				  << ctx.resolved_path << std::endl;
		enqueueErrorResponse(ctx, HttpStatusCode::Forbidden);
		resp.addHeader("Content-Length", "0");
		return;
	}

	if (FileHandler::deleteFile(ctx.resolved_path))
	{
		std::cout << "[processDelete] File deleted successfully: "
				  << ctx.resolved_path << std::endl;
		resp.setStatus(HttpStatusCode::NoContent);
		resp.addHeader("Content-Length", "0");
	}
	else
	{
		std::cout << "[processDelete] Failed to delete file: "
				  << ctx.resolved_path << std::endl;
		enqueueErrorResponse(ctx, HttpStatusCode::InternalServerError);
		resp.addHeader("Content-Length", "0");
	}

	return;
}


// Body size gets filled for the default errors
void RequestHandler::addGeneralErrorDetails(RawResponse& resp,
											RequestContext& ctx,
											HttpStatusCode code)
{
	std::cout << "[addGeneralErrorDetails] Called with code: "
			  << static_cast<int>(code) << "\n";

	resp.setStatusCode(code);
	std::cout << "[addGeneralErrorDetails] Code set to"
			  << static_cast<int>(code) << "\n";

	auto it = ctx.error_pages.find(code);
	if (it != ctx.error_pages.end())
	{
		std::cout
			<< "[addGeneralErrorDetails] Found entry in ctx.error_pages for "
			<< static_cast<int>(code) << ": \"" << it->second << "\"\n";

		if (!it->second.empty())
		{
			std::cout << "[addGeneralErrorDetails] Using CUSTOM error page: "
					  << it->second << "\n";
			resp.setInternalRedirect(true);
			resp.setRedirectTarget(it->second);

			std::cout << "[addGeneralErrorDetails] Marked as internal "
						 "redirect. Target set to: "
					  << resp.getRedirectTarget() << "\n";
			return;
		}
		else
		{
			std::cout << "[addGeneralErrorDetails] Entry found but empty path "
						 "for code "
					  << static_cast<int>(code) << "\n";
		}
	}
	else
	{
		std::cout << "[addGeneralErrorDetails] No custom error page entry "
					 "found for code "
				  << static_cast<int>(code) << "\n";
	}

	std::cout << "[addGeneralErrorDetails] Generating DEFAULT error page for "
			  << static_cast<int>(code) << "\n";

	resp.addDefaultErrorDetails(code);
	resp.addHeader("Content-Type", "text/html");
}

void RequestHandler::enqueueErrorResponse(RequestContext& ctx,
										  HttpStatusCode status)
{
	RawResponse rawResp;
	rawResp.setStatus(status);

	std::cout << "[enqueueErrorResponse] Called with status "
			  << static_cast<int>(status) << std::endl;

	// Check if a custom error page exists
	auto it = ctx.error_pages.find(status);
	if (it != ctx.error_pages.end() && !it->second.empty())
	{
		std::string pageUri = it->second;
		std::cout << "[enqueueErrorResponse] Custom error page found: "
				  << pageUri << "\n";

		// Instead of enqueuing here, mark internal redirect
		rawResp.setInternalRedirect(true);
		rawResp.setFilePath(pageUri); // optional debug

		std::cout << "[enqueueErrorResponse] Internal redirect set, custom "
					 "page will be handled in ConnectionManager.\n";
		clientState.enqueueRawResponse(rawResp); // still enqueue signal
		return;
	}

	// No custom page, generate default
	rawResp.addDefaultErrorDetails(status);
	rawResp.addHeader("Content-Type", "text/html");
	clientState.enqueueRawResponse(rawResp);

	std::cout << "[enqueueErrorResponse] No custom page, default error page "
				 "generated for status "
			  << static_cast<int>(status) << std::endl;
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
								 RequestContext& ctx, RawResponse& resp)
{
	(void)endpoint;
	std::cout << "[processPost] Processing POST for path: " << ctx.resolved_path
			  << std::endl;

	if (ctx.client_max_body_size != 0 && req.body.size() > ctx.client_max_body_size)
	{
		std::cout << "[processPost] Body too large: " << req.body.size()
				  << " > " << ctx.client_max_body_size << std::endl;
		enqueueErrorResponse(ctx, HttpStatusCode::PayloadTooLarge);
		return;
	}

	if (!ctx.cgi_pass.empty())
	{
		std::cout << "[processPost] CGI configured, executing..." << std::endl;
		handleCGI(req, endpoint);
		return;
	}

	if (ctx.upload_store.empty())
	{
		std::cout << "[processPost] Upload store not configured, forbidden"
				  << std::endl;
		enqueueErrorResponse(ctx, HttpStatusCode::Forbidden);
		return;
	}

	UploadModule::processUpload(req, ctx, resp);
	std::cout << "[processPost] Upload processed, body size: "
			  << resp.getBody().size() << std::endl;
	
	clientState.enqueueRawResponse(resp);

	return;
}

std::string RequestHandler::readFileToString(const std::string& path)
{
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file)
	{
		std::cout << "[readFileToString] Failed to open: " << path << std::endl;
		return "";
	}
	std::ostringstream contents;
	contents << file.rdbuf();
	std::cout << "[readFileToString] Read " << contents.str().size()
			  << " bytes from " << path << std::endl;
	return contents.str();
}

