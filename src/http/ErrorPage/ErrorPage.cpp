#include "ErrorPage.hpp"

ErrorPage::ErrorPage(const std::vector<HttpStatusCode>& codes,
                     const std::string& path)
  : statusCodes(codes)
  , filePath(std::move(path))
{
}
