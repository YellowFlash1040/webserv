#include "Directives.hpp"

// clang-format off

namespace Directives
{

enum ArgCountType
{
    FixedCount,
    Between,
    AtLeast,
    Any
};

struct DirectiveSpec
{
    DirectiveType type; // "simple" or "block"
    std::string required_parent;
    std::set<std::string> allowed_in;
    ArgCountType arg_type;
    size_t min_args;
    size_t max_args;
};

const std::map<std::string, DirectiveSpec> directives = {
    {"http", {
        DirectiveType::BLOCK,
        "",              // no required parent
        {""},            // allowed in root
        FixedCount, 0, 0,
    }},
    {"server", {
        DirectiveType::BLOCK,
        "http",
        {"http"},
        FixedCount, 0, 0
    }},
    {"server_name", {
        DirectiveType::SIMPLE,
        "server",
        {"server"},
        FixedCount, 1, 1
    }},
    {"listen", {
        DirectiveType::SIMPLE,
        "server",
        {"server"},
        FixedCount, 1, 1
    }},
    {"error_page", {
        DirectiveType::SIMPLE,
        "",
        {"http", "server"},
        FixedCount, 2, 2
    }},
    {"client_max_body_size", {
        DirectiveType::SIMPLE,
        "",
        {"http", "server", "location"},
        FixedCount, 1, 1
    }},
    {"location", {
        DirectiveType::BLOCK,
        "server",
        {"server"},
        AtLeast, 1, static_cast<size_t>(-1)
    }},
    {"limit_except", {
        DirectiveType::BLOCK,
        "location",
        {"location"},
        Between, 1, 4
    }},
    {"deny", {
        DirectiveType::SIMPLE,
        "limit_except",
        {"limit_except"},
        FixedCount, 1, 1
    }},
    {"return", {
        DirectiveType::SIMPLE,
        "location",
        {"location"},
        FixedCount, 2, 2
    }},
    {"root", {
        DirectiveType::SIMPLE,
        "",
        {"server", "location"},
        FixedCount, 1, 1
    }},
    {"alias", {
        DirectiveType::SIMPLE,
        "location",
        {"location"},
        FixedCount, 1, 1
    }},
    {"autoindex", {
        DirectiveType::SIMPLE,
        "location",
        {"location"},
        FixedCount, 1, 1
    }},
    {"index", {
        DirectiveType::SIMPLE,
        "location",
        {"location"},
        AtLeast, 1, static_cast<size_t>(-1)
    }},
    {"upload_store", {
        DirectiveType::SIMPLE,
        "location",
        {"location"},
        FixedCount, 1, 1
    }},
    {"cgi_pass", {
        DirectiveType::SIMPLE,
        "location",
        {"location"},
        FixedCount, 1, 1
    }}
};

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
    auto it = directives.find(name);
    if (it == directives.end())
        return false;

    DirectiveSpec specs = it->second;
    if (specs.type != DirectiveType::BLOCK)
        return false;

    return true; 
}

bool isSimpleDirective(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        return false;

    DirectiveSpec specs = it->second;
    if (specs.type != DirectiveType::SIMPLE)
        return false;

    return true;
}

bool isAllowedInContext(const std::string& name, const std::string& context)
{
    auto it = directives.find(name);
    if (it == directives.end())
        return false;

    DirectiveSpec specs = it->second;
    if (specs.allowed_in.count(context) == 0)
        return false;

    return true;
}

std::pair<bool, std::string> hasRequiredParentContext(
    const std::string& name, const std::string& parentContext)
{
    auto it = directives.find(name);
    if (it == directives.end()) // if we didn't found such a directive, which can not happen
        return {false, ""}; // I return false here, so that it would cause an error

    DirectiveSpec specs = it->second;
    if (specs.required_parent.empty())
        return {true, ""};

    if (specs.required_parent == parentContext)
        return {true, specs.required_parent};

    return {false, specs.required_parent};
}

bool hasRightAmountOfArguments(const std::string& name,
    const std::vector<std::string>& args)
{
    auto it = directives.find(name);
    if (it == directives.end())
    return false;

    DirectiveSpec specs = it->second;

    if (specs.arg_type == FixedCount
        && args.size() == specs.min_args)
        return true;

    if (specs.arg_type == AtLeast
        && args.size() >= specs.min_args)
        return true;

    if (specs.arg_type == Between
        && args.size() >= specs.min_args
        && args.size() <= specs.max_args)
        return true;

    if (specs.arg_type == Any)
        return true;
    
    return false;
}

// clang-format on

} // namespace Directives
