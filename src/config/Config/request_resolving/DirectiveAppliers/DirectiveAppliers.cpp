#include "DirectiveAppliers.hpp"

namespace DirectiveAppliers
{

void applyErrorPages(const std::vector<ErrorPage>& errorPages,
                     std::map<HttpStatusCode, std::string>& target)
{
    for (const auto& errorPage : errorPages)
    {
        for (const auto& statusCode : errorPage.statusCodes)
            target[statusCode] = errorPage.filePath;
    }
}

} // namespace DirectiveAppliers
