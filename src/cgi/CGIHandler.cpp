#include "CGIHandler.hpp"

void CGIHandler::processCGI(const RequestData& req,
                                const NetworkEndpoint& endpoint,
                                const std::string& interpreter,
                                const std::string& scriptPath,
                                RawResponse& rawResp)
{
    try {
        std::string cgiResult = handleCGI(req, endpoint, interpreter, scriptPath);
        ParsedCGI parsed = CGIParser::parse(cgiResult);

        if (parsed.is_redirect)
		{
            rawResp.setStatus(static_cast<HttpStatusCode>(parsed.status));
            rawResp.addHeader("Location", parsed.headers.at("Location"));
            rawResp.setBody("");
            rawResp.setMimeType("");
        }
		else
		{
            rawResp.setBody(parsed.body);

            int status_code = parsed.status > 0 ? parsed.status : static_cast<int>(HttpStatusCode::OK);
            rawResp.setStatus(static_cast<HttpStatusCode>(status_code));

            std::string ct = parsed.headers.count("Content-Type") ? parsed.headers.at("Content-Type") : "text/plain";
            rawResp.setMimeType(ct);
        }
    }
    catch (const std::exception &e)
	{
        rawResp.setStatus(HttpStatusCode::InternalServerError);
        rawResp.setMimeType("text/plain");
        rawResp.setBody(std::string("CGI parse error: ") + e.what());
    }
}

std::string CGIHandler::handleCGI(const RequestData& req, const NetworkEndpoint& endpoint, const std::string& interpreter, const std::string& scriptPath)
{
	std::cout << "[handleCGI] scriptPath = " << scriptPath
          << ", interpreter = " << interpreter << std::endl;

    std::vector<std::string> args;
    //args.push_back(interpreter);
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

	std::cout << "[handleCGI] Host = " << host << std::endl;

    if (!host.empty())
        env.push_back("SERVER_NAME=" + host);
    else
        env.push_back("SERVER_NAME=" + static_cast<std::string>(localIp));

    const std::string& input = req.body;

    try
    {
        return CGI::execute(interpreter, args, env, input);
    }
    catch (const std::exception& e)
    {
        return std::string("Content-Type: text/plain\r\n\r\nCGI error: ") + e.what();
    }
}