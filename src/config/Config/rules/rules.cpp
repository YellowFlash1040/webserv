#include "rules.hpp"
#include "HttpStatusCode.hpp"
#include "ErrorPage.hpp"

void applyErrorPages(const std::vector<ErrorPage>& errorPages,
                     std::map<HttpStatusCode, std::string>& target)
{
    for (const auto& errorPage : errorPages)
    {
        for (const auto& statusCode : errorPage.statusCodes)
            target[statusCode] = errorPage.filePath;
    }
}
