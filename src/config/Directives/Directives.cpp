#include "Directives.hpp"

namespace Directives
{

// clang-format off

const std::set<std::string> blockDirectives = {
	"http",
    "server",
    "location",
    "limit_except"
};

const std::set<std::string> simpleDirectives = {
	"listen",
	"server_name",
	"error_page",
	"client_max_body_size",
	"return",
	"root",
	"alias",
	"autoindex",
	"index",
	"upload_store",
	"cgi_pass",
	"deny"
};

// maps a set of allowed directives to a context
const std::map<std::string, std::set<std::string>> directivesAllowedByContext = {
    { "http", { //inside 
        "client_max_body_size",
        "error_page",
        "server"
    }},
    { "server", {
        "listen",
        "server_name",
        "root",
        "location",
        "client_max_body_size",
        "error_page"
    }},
    { "location", {
        "root",
        "limit_except",
        "index",
        "autoindex",
        "return",
        "client_max_body_size",
        "upload_store",
        "cgi_pass"
    }},
    { "limit_except", {
        "deny"
    }}
};

// “X must appear inside Y”.
const std::map<std::string, std::string> requiredParentContext = {
    {"server", "http"},
    {"location", "server"},
    {"limit_except", "location"},
    {"deny", "limit_except"}
};

// clang-format on

DirectiveType getDirectiveType(const std::string& name)
{
    if (isSimpleDirective(name))
        return DirectiveType::SIMPLE;
    if (isBlockDirective(name))
        return DirectiveType::BLOCK;
    return DirectiveType::UNKNOWN;
}

bool isBlockDirective(const std::string& name)
{
    return blockDirectives.count(name) > 0;
}

bool isSimpleDirective(const std::string& name)
{
    return simpleDirectives.count(name) > 0;
}

bool isAllowedInContext(const std::string& name, const std::string& context)
{
    auto it = directivesAllowedByContext.find(context);
    if (it != directivesAllowedByContext.end())
    {
        std::set<std::string> allowedDirectives = it->second;
        if (allowedDirectives.find(name) != it->second.end())
            return true;
    }
    return false;
}

std::pair<bool, std::string> hasRequiredParentContext(
    const std::string& name, const std::string& parentContext)
{
    auto it = requiredParentContext.find(name);
    if (it == requiredParentContext.end())
        return {true, ""};

    const std::string& requiredParent = it->second;
    if (requiredParent == parentContext)
        return {true, ""};

    return {false, requiredParent};
}

} // namespace Directives
