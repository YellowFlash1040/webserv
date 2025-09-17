#include "Directives.hpp"

namespace Directives
{

// clang-format off

const std::unordered_set<std::string> blockDirectives = {
	"http",
    "server",
    "location",
    "limit_except"
};

const std::unordered_set<std::string> simpleDirectives = {
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

} // namespace Directives
