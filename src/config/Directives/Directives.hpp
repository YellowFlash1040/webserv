#pragma once

#ifndef DIRECTIVES_HPP
# define DIRECTIVES_HPP

# include <set>
# include <map>
# include <string>

# include "ADirective.hpp"
# include "BlockDirective.hpp"
# include "SimpleDirective.hpp"

enum class DirectiveType
{
    UNKNOWN,
    SIMPLE,
    BLOCK
};

namespace Directives
{

constexpr const char* GLOBAL_CONTEXT = "global";
constexpr const char* HTTP = "http";
constexpr const char* SERVER = "server";
constexpr const char* SERVER_NAME = "server_name";
constexpr const char* LISTEN = "listen";
constexpr const char* ERROR_PAGE = "error_page";
constexpr const char* CLIENT_MAX_BODY_SIZE = "client_max_body_size";
constexpr const char* LOCATION = "location";
constexpr const char* ACCEPTED_METHODS = "limit_except";
constexpr const char* RETURN = "return";
constexpr const char* ROOT = "root";
constexpr const char* ALIAS = "alias";
constexpr const char* AUTOINDEX = "autoindex";
constexpr const char* INDEX = "index";
constexpr const char* UPLOAD_STORE = "upload_store";
constexpr const char* CGI_PASS = "cgi_pass";

extern const std::set<std::string> blockDirectives;
extern const std::set<std::string> simpleDirectives;
extern const std::map<std::string, std::set<std::string>>
    directivesAllowedByContext;
extern const std::map<std::string, std::string> requiredParentContext;

bool isKnownDirective(const std::string& name);
DirectiveType getDirectiveType(const std::string& name);
bool isBlockDirective(const std::string& name);
bool isSimpleDirective(const std::string& name);
bool isAllowedInContext(const std::string& name, const std::string& context);
std::set<std::string> getAllowedContextsFor(const std::string& name);
bool hasRightAmountOfArguments(const std::string& name, size_t amount);

} // namespace Directives

#endif
