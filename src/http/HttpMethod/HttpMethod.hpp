#pragma once

#ifndef HTTPMETHOD_HPP
# define HTTPMETHOD_HPP

# include <string>
# include <stdexcept>
# include <unordered_map>
# include <vector>

enum class HttpMethod
{
    NONE,
    GET,
    POST,
    DELETE
};

#endif
