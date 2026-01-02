// #pragma once

// #ifndef CGI_HPP
// # define CGI_HPP

// # include <string>
// # include <vector>
// # include "RequestData.hpp"
// # include "HttpMethod.hpp"

// class CGI
// {
//   public:
//     static std::string execute(const std::string& scriptPath,
//                                const std::vector<std::string>& args,
//                                const std::vector<std::string>& env,
//                                const std::string& input = "");

//     static std::vector<std::string> buildEnvFromRequest(const RequestData& req);
// };

// #endif