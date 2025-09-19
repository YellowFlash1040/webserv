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
    { "", {
        "http",
    }},
    { "http", {
        "server",
        "client_max_body_size",
        "error_page"
    }},
    { "server", {
        "listen",
        "server_name",
        "client_max_body_size",
        "error_page",
        "root",
        "location"
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
    {"listen", "server"},
    {"server_name", "server"},
    {"limit_except", "location"},
    {"deny", "limit_except"}
};

enum ArgCountType {
    FixedCount,    // exactly N args
    Between,       // between min and max
    AtLeast,       // at least N args
    Any,           // 0 or more
};

struct DirectiveSpec {
    ArgCountType type;
    size_t min_args;
    size_t max_args;
};

const std::map<std::string, DirectiveSpec> directiveArgSpec = {
    {"http", {FixedCount, 0, 0}},
    {"server", {FixedCount, 0, 0}},
    {"location", {FixedCount, 1, 1}},
    {"limit_except", {Between, 1, 4}},
	{"listen", {FixedCount, 1, 0}},
	{"server_name", {FixedCount, 1, 1}},
	{"error_page", {FixedCount, 2, 2}},
	{"client_max_body_size", {FixedCount, 1, 1}},
	{"return", {FixedCount, 2, 2}},
	{"root", {FixedCount, 1, 1}},
	{"alias", {FixedCount, 1, 1}},
	{"autoindex", {FixedCount, 1, 1}},
	{"index", {AtLeast, 1, static_cast<size_t>(-1)}},
	{"upload_store", {FixedCount, 1, 1}},
	{"cgi_pass", {FixedCount, 1, 1}},
	{"deny", {FixedCount, 1, 1}},
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
        const std::set<std::string>& allowedDirectives = it->second;
        if (allowedDirectives.find(name) != allowedDirectives.end())
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
