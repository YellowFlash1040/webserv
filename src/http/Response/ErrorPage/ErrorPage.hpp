#pragma once

#ifndef ERRORPAGE_HPP
# define ERRORPAGE_HPP

# include <string>
# include <vector>

# include "HttpStatusCode.hpp"

struct ErrorPage
{
    std::vector<HttpStatusCode> statusCodes;
    std::string filePath;

    ErrorPage(const std::vector<HttpStatusCode>& codes,
              const std::string& path);
};

#endif
