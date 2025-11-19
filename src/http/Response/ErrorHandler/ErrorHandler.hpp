#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "../ErrorPage/ErrorPage.hpp"
#include "../HttpStatusCode/HttpStatusCode.hpp"

// Forward declaration of Response to avoid circular includes
class Response;

namespace ErrorHandler
{
    // Generate HTML body for given HTTP status code
    std::string generateErrorBody(int code, const std::vector<ErrorPage>& errorPages);

    // Get custom error page file path for given status code
    std::string getErrorPageFilePath(int code, const std::vector<ErrorPage>& errorPages);
}

#endif
