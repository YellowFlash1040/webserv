#pragma once

#ifndef DIRECTIVES_HPP
# define DIRECTIVES_HPP

# include <set>
# include <map>
# include <string>
# include <stdexcept>
# include <limits>

# include "Directive.hpp"
# include "BlockDirective.hpp"

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
constexpr const char* LIMIT_EXCEPT = "limit_except";
constexpr const char* RETURN = "return";
constexpr const char* ROOT = "root";
constexpr const char* ALIAS = "alias";
constexpr const char* AUTOINDEX = "autoindex";
constexpr const char* INDEX = "index";
constexpr const char* UPLOAD_STORE = "upload_store";
constexpr const char* CGI_PASS = "cgi_pass";

constexpr size_t UNLIMITED = std::numeric_limits<size_t>::max();

enum class Type
{
    UNKNOWN,
    SIMPLE,
    BLOCK
};

struct ArgumentSpec
{
    std::vector<ArgumentType> possibleTypes;
    size_t minCount;
    size_t maxCount;
};

struct DirectiveSpec
{
    Type type;
    std::set<std::string> allowedIn;
    std::vector<ArgumentSpec> argSpecs;
    std::vector<std::string> conflictingDirectives;
    bool allowsDuplicates;
};

bool isKnownDirective(const std::string& name);
Type getDirectiveType(const std::string& name);
bool isBlockDirective(const std::string& name);
bool isDirective(const std::string& name);
bool isAllowedInContext(const std::string& name, const std::string& context);
const std::set<std::string>& getAllowedContextsFor(const std::string& name);
// bool hasRightAmountOfArguments(const std::string& name, size_t amount);
const std::vector<ArgumentSpec>& getArgSpecs(const std::string& name);
bool allowsDuplicates(const std::string& name);
const std::vector<std::string>& getConflictingDirectives(
    const std::string& name);

} // namespace Directives

#endif
