#pragma once

#ifndef HTTPREDIRECTION_HPP
# define HTTPREDIRECTION_HPP

# include <string>

# include "HttpStatusCode.hpp"

struct HttpRedirection
{
    HttpStatusCode statusCode = HttpStatusCode::Ok;
    std::string url;
};

#endif
