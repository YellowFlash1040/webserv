#pragma once

#ifndef HTTPREDIRECTION_HPP
# define HTTPREDIRECTION_HPP

# include <string>

# include "HttpStatusCode.hpp"

struct HttpRedirection
{
    bool isSet = false;
    HttpStatusCode statusCode = HttpStatusCode::None;
    std::string url;
};

#endif
