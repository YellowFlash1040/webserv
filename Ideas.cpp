#include "Directives.hpp"

// clang-format off

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
    {"listen", {
        DirectiveType::SIMPLE,
        "server",
        {"server"},
        FixedCount, 1, 0
    }},
    {"location", {
        DirectiveType::BLOCK,
        "server",
        {"server"},
        FixedCount, 1, 1
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
    }}
    // ... and so on for all directives
};

bool isBlockDirective(const std::string& name) {
    auto it = directives.find(name);
    return it != directives.end() && it->second.type == DirectiveType::BLOCK;
}

bool isSimpleDirective(const std::string& name) {
    auto it = directives.find(name);
    return it != directives.end() && it->second.type == DirectiveType::SIMPLE;
}

bool isAllowedInContext(const std::string& name, const std::string& context) {
    auto it = directives.find(name);
    if (it == directives.end()) return false;
    return it->second.allowed_in.count(context) > 0;
}

std::pair<bool, std::string> hasRequiredParentContext(
    const std::string& name, const std::string& parentContext)
{
    auto it = directives.find(name);
    if (it == directives.end() || it->second.required_parent.empty())
        return {true, ""};
    return {it->second.required_parent == parentContext, it->second.required_parent};
}

// clang-format on
