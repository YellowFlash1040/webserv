#include "ResponseGenerator.hpp"

namespace ResponseGenerator
{

void genResponse(const RawRequest& rawReq, const RequestContext& ctx,
                 RawResponse& rawResp, CgiRequestResult& cgiResult)
{
    setConnectionHeader(rawReq, rawResp);

    if (rawReq.isBadRequest())
        return handleBadRequest(ctx, rawResp);

    const RequestData req = rawReq.buildRequestData();

    if (ctx.redirection.isSet)
        return handleExternalRedirect(ctx, req.uri, rawResp);

    if (!isMethodAllowed(req.method, ctx.allowed_methods))
        return handleMethodNotAllowed(ctx, rawResp);

    switch (req.method)
    {
    case HttpMethod::GET:
        return processGet(req, ctx, rawResp, cgiResult);
    case HttpMethod::POST:
        return processPost(req, ctx, rawResp, cgiResult);
    case HttpMethod::DELETE:
        return processDelete(ctx, rawResp);
    default:
        return handleServerError(ctx, rawResp);
    }
}

bool isMethodAllowed(HttpMethod method,
                     const std::vector<HttpMethod>& allowed_methods)
{
    for (const auto& allowedMethod : allowed_methods)
        if (method == allowedMethod)
            return true;

    return false;
}

void addAllowHeader(RawResponse& rawResp,
                    const std::vector<HttpMethod>& allowed_methods)
{
    std::string allowed;

    if (allowed_methods.empty())
        return;

    allowed += httpMethodToString(allowed_methods[0]);
    for (size_t i = 1; i < allowed_methods.size(); ++i)
    {
        allowed += ", ";
        allowed += httpMethodToString(allowed_methods[i]);
    }
    rawResp.addHeader("Allow", allowed);
}

void processGet(const RequestData& req, const RequestContext& ctx,
                RawResponse& rawResp, CgiRequestResult& cgiResult)
{
    const std::string ext = getFileExtension(req.uri);
    if (!ctx.cgi_pass.empty() && ctx.cgi_pass.count(ext))
        return handleCGI(req, ctx, rawResp, cgiResult, ext);

    const FileInfo path = FileUtils::getFileInfo(ctx.resolved_path);

    if (path.exists && path.isFile)
        return handleStaticFile(ctx, rawResp, path);

    if (path.exists && path.isDir)
        return handleDirectory(ctx, rawResp);

    handleNotFound(ctx, rawResp);
}

void processDelete(const RequestContext& ctx, RawResponse& rawResp)
{
    const FileInfo path = FileUtils::getFileInfo(ctx.resolved_path);

    if (!path.exists)
        return handleNotFound(ctx, rawResp);

    if (path.isDir || !path.writable)
        return handleNoPermission(ctx, rawResp);

    try
    {
        FileUtils::deleteFile(ctx.resolved_path);
        rawResp.setStatusCode(HttpStatusCode::NoContent);
    }
    catch (const std::exception& e)
    {
        handleServerError(ctx, rawResp);
    }
}

void processPost(const RequestData& req, const RequestContext& ctx,
                 RawResponse& rawResp, CgiRequestResult& cgiResult)
{
    if (ctx.client_max_body_size != 0
        && req.body.size() > ctx.client_max_body_size)
        return handlePayloadTooLarge(ctx, rawResp);

    const std::string ext = getFileExtension(req.uri);
    if (!ctx.cgi_pass.empty() && ctx.cgi_pass.count(ext))
        return handleCGI(req, ctx, rawResp, cgiResult, ext);

    if (ctx.upload_store.empty())
        return handleNoPermission(ctx, rawResp);

    const FileInfo storage = FileUtils::getFileInfo(ctx.upload_store);
    if (!(storage.exists && storage.isDir))
        return handleServerError(ctx, rawResp);

    try
    {
        UploadModule::processUpload(req, ctx, rawResp);
    }
    catch (const std::exception& e)
    {
        handleServerError(ctx, rawResp);
    }
}

void fillSuccessfulResponse(RawResponse& resp, const std::string& filePath)
{
    const std::string body = FileReader::readFile(filePath);
    resp.setBody(body);
    resp.addHeader("Content-Length", std::to_string(body.size()));
    resp.setMimeType(FileUtils::detectMimeType(filePath));
    resp.setStatusCode(HttpStatusCode::OK);
}

void fillAutoindexResponse(RawResponse& resp, const std::string& dirPath)
{
    const std::string body = FileUtils::generateAutoindex(dirPath);
    resp.setBody(body);
    resp.addHeader("Content-Length", std::to_string(body.size()));
    resp.setMimeType("text/html");
    resp.setStatusCode(HttpStatusCode::OK);
}

void handleExternalRedirect(const RequestContext& ctx,
                            const std::string& reqUri, RawResponse& rawResp)
{
    if (ctx.redirection.url == reqUri)
    {
        // Handle self-redirect differently
        rawResp.setStatusCode(HttpStatusCode::LoopDetected);
        rawResp.setBody("<html><head><title>Error</title></head>"
                        "<body>Redirection loop detected for "
                        + ctx.redirection.url + "</body></html>");
    }
    else
    {
        // Standard external redirection
        rawResp.setStatusCode(ctx.redirection.statusCode);
        rawResp.addHeader("Location", ctx.redirection.url);
    }
}

void handleStaticFile(const RequestContext& ctx, RawResponse& rawResp,
                      const FileInfo& file)
{
    if (!file.readable)
        return handleNoPermission(ctx, rawResp);

    serveStaticFile(ctx, rawResp);
}

void serveStaticFile(const RequestContext& ctx, RawResponse& rawResp)
{
    try
    {
        fillSuccessfulResponse(rawResp, ctx.resolved_path);
    }
    catch (const std::exception& e)
    {
        handleServerError(ctx, rawResp);
    }
}

void handleDirectory(const RequestContext& ctx, RawResponse& rawResp)
{
    std::string indexFilePath
        = FileUtils::getFirstValidIndexFile(ctx.resolved_path, ctx.index_files);
    if (!indexFilePath.empty())
        return serveIndexFile(indexFilePath, ctx, rawResp);

    if (ctx.autoindex_enabled)
        return generateAutoIndex(ctx, rawResp);

    handleNoPermission(ctx, rawResp);
}

void handleNotFound(const RequestContext& ctx, RawResponse& rawResp)
{
    rawResp.addErrorDetails(ctx, HttpStatusCode::NotFound);
}

void handleNoPermission(const RequestContext& ctx, RawResponse& rawResp)
{
    rawResp.addErrorDetails(ctx, HttpStatusCode::Forbidden);
}

void handleServerError(const RequestContext& ctx, RawResponse& rawResp)
{
    rawResp.addErrorDetails(ctx, HttpStatusCode::InternalServerError);
}

void handlePayloadTooLarge(const RequestContext& ctx, RawResponse& rawResp)
{
    rawResp.addErrorDetails(ctx, HttpStatusCode::PayloadTooLarge);
}

void handleBadRequest(const RequestContext& ctx, RawResponse& rawResp)
{
    rawResp.addErrorDetails(ctx, HttpStatusCode::BadRequest);
}

void handleMethodNotAllowed(const RequestContext& ctx, RawResponse& rawResp)
{
    rawResp.addErrorDetails(ctx, HttpStatusCode::MethodNotAllowed);
    addAllowHeader(rawResp, ctx.allowed_methods);
}

void handleBadGateway(const RequestContext& ctx, RawResponse& rawResp)
{
    rawResp.addErrorDetails(ctx, HttpStatusCode::BadGateway);
}

void serveIndexFile(const std::string& indexPath, const RequestContext& ctx,
                    RawResponse& rawResp)
{
    try
    {
        fillSuccessfulResponse(rawResp, indexPath);
    }
    catch (const std::exception& e)
    {
        handleServerError(ctx, rawResp);
    }
}

void generateAutoIndex(const RequestContext& ctx, RawResponse& rawResp)
{
    try
    {
        fillAutoindexResponse(rawResp, ctx.resolved_path);
    }
    catch (const std::exception& e)
    {
        handleServerError(ctx, rawResp);
    }
}

void setConnectionHeader(const RawRequest& rawReq, RawResponse& rawResp)
{
    bool shouldClose = rawReq.shouldClose();
    if (shouldClose)
        rawResp.addHeader("Connection", "close");
    else
        rawResp.addHeader("Connection", "keep-alive");
}

std::string getFileExtension(const std::string& uri)
{
    size_t pos = uri.find_last_of('.');
    if (pos == std::string_view::npos)
        return {};
    return uri.substr(pos);
}

void handleCGI(const RequestData& req, const RequestContext& ctx,
               RawResponse& rawResp, CgiRequestResult& cgiResult,
               const std::string& ext)
{
    const std::string& interpreter = ctx.cgi_pass.find(ext)->second;

    const FileInfo info = FileUtils::getFileInfo(interpreter);
    if (!(info.exists && info.isFile && info.executable))
        return handleBadGateway(ctx, rawResp);

    std::string requestedScript = ctx.resolved_path;

    HttpStatusCode result = checkScriptValidity(requestedScript);
    if (result != HttpStatusCode::OK)
        return handleScriptInvalidity(result, ctx, rawResp);

    cgiResult.spawnCgi = true;
    cgiResult.cgiInterpreter = interpreter;
    cgiResult.cgiScriptPath = requestedScript;
    cgiResult.requestData = req;
}

HttpStatusCode checkScriptValidity(const std::string& scriptPath)
{
    const FileInfo path = FileUtils::getFileInfo(scriptPath);

    if (!path.exists)
        return HttpStatusCode::NotFound;

    if (path.isDir)
        return HttpStatusCode::Forbidden;

    return HttpStatusCode::OK;
}

void handleScriptInvalidity(HttpStatusCode status, const RequestContext& ctx,
                            RawResponse& rawResp)
{
    switch (status)
    {
    case HttpStatusCode::NotFound:
        return handleNotFound(ctx, rawResp);
    case HttpStatusCode::Forbidden:
        return handleNoPermission(ctx, rawResp);
    default:
        return handleServerError(ctx, rawResp);
    }
}

} // namespace ResponseGenerator
