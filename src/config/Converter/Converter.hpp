#pragma once

#ifndef CONVERTER_HPP
# define CONVERTER_HPP

# include <string>
# include <map>

# include "HttpMethod.hpp"
# include "BodySize.hpp"
# include "HttpStatusCode.hpp"

namespace Converter
{

HttpMethod toHttpMethod(const std::string& value);
bool toBool(const std::string& value);
BodySize toBodySize(const std::string& value);
HttpStatusCode toHttpStatusCode(const std::string& value);

}; // namespace Converter

#endif
