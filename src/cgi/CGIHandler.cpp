// #include "CGIHandler.hpp"

// void CGIHandler::processCGI(const RequestData& req,
//                             Client& client,
//                             const std::string& interpreter,
//                             const std::string& scriptPath,
//                             RawResponse& rawResp)
// {
//     try {
//         std::string cgiResult = handleCGI(req, client, interpreter, scriptPath);
//         ParsedCGI parsed = CGIParser::parse(cgiResult);

//         if (parsed.is_redirect)
//         {
//             rawResp.setStatus(static_cast<HttpStatusCode>(parsed.status));
//             rawResp.addHeader("Location", parsed.headers.at("Location"));
//             rawResp.setBody("");
//             rawResp.setMimeType("");
//         }
//         else
//         {
//             rawResp.setBody(parsed.body);

//             int status_code = parsed.status > 0 ? parsed.status : static_cast<int>(HttpStatusCode::OK);
//             rawResp.setStatus(static_cast<HttpStatusCode>(status_code));

//             std::string ct = parsed.headers.count("Content-Type") ? parsed.headers.at("Content-Type") : "text/plain";
//             rawResp.setMimeType(ct);
//         }
//     }
//     catch (const std::exception &e)
//     {
//         rawResp.setStatus(HttpStatusCode::InternalServerError);
//         rawResp.setMimeType("text/plain");
//         rawResp.setBody(std::string("CGI parse error: ") + e.what());
//     }
// }

// std::string CGIHandler::handleCGI(const RequestData& req,
//                                   Client& client,
//                                   const std::string& interpreter,
//                                   const std::string& scriptPath)
// {
//     std::cout << "[handleCGI] scriptPath = " << scriptPath
//               << ", interpreter = " << interpreter << std::endl;

//     std::vector<std::string> args;
//     args.push_back(scriptPath);

//     std::vector<std::string> env = CGI::buildEnvFromRequest(req);
//     env.push_back("SCRIPT_FILENAME=" + scriptPath);

//     const NetworkEndpoint& endpoint = client.getListeningEndpoint();
//     NetworkInterface localIp = endpoint.ip();
//     int localPort = endpoint.port();

//     env.push_back("SERVER_ADDR=" + static_cast<std::string>(localIp));
//     env.push_back("SERVER_PORT=" + std::to_string(localPort));

//     const sockaddr_in& clientAddr = client.getAddress();
//     uint32_t ip = ntohl(clientAddr.sin_addr.s_addr);
//     int remotePort = ntohs(clientAddr.sin_port);

//     std::string remoteIp = std::to_string((ip >> 24) & 0xFF) + "." +
//                            std::to_string((ip >> 16) & 0xFF) + "." +
//                            std::to_string((ip >> 8) & 0xFF) + "." +
//                            std::to_string(ip & 0xFF);

//     env.push_back("REMOTE_ADDR=" + remoteIp);
//     env.push_back("REMOTE_PORT=" + std::to_string(remotePort));

//     std::string host = req.getHeader("Host");

//     std::cout << "[handleCGI] Host = " << host << std::endl;

//     if (!host.empty())
//         env.push_back("SERVER_NAME=" + host);
//     else
//         env.push_back("SERVER_NAME=" + static_cast<std::string>(localIp));

//     const std::string& input = req.body;

//     try
//     {
//         return CGI::execute(interpreter, args, env, input);
//     }
//     catch (const std::exception& e)
//     {
//         return std::string("Content-Type: text/plain\r\n\r\nCGI error: ") + e.what();
//     }
// }
