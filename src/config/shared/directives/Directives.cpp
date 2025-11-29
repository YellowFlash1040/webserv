#include "Directives.hpp"

namespace Directives
{

bool isKnownDirective(const std::string& name)
{
    return directives.count(name) > 0;
}

Type getDirectiveType(const std::string& name)
{
    if (isDirective(name))
        return Type::SIMPLE;
    if (isBlockDirective(name))
        return Type::BLOCK;
    return Type::UNKNOWN;
}

bool isBlockDirective(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        return false;

    DirectiveSpec specs = it->second;
    if (specs.type != Type::BLOCK)
        return false;

    return true;
}

bool isDirective(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        return false;

    DirectiveSpec specs = it->second;
    if (specs.type != Type::SIMPLE)
        return false;

    return true;
}

bool isAllowedInContext(const std::string& name, const std::string& context)
{
    auto it = directives.find(name);
    if (it == directives.end())
        return false;

    DirectiveSpec specs = it->second;
    if (specs.allowedIn.count(context) == 0)
        return false;

    return true;
}

const std::set<std::string>& getAllowedContextsFor(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        throw std::logic_error("Unknown directive '" + name + "'");

    const DirectiveSpec& specs = it->second;
    return specs.allowedIn;
}

const std::vector<ArgumentSpec>& getArgSpecs(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        throw std::logic_error("Unknown directive '" + name + "'");

    const DirectiveSpec& directiveSpecs = it->second;
    return directiveSpecs.argSpecs;
}

bool allowsDuplicates(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        throw std::logic_error("Unknown directive '" + name + "'");

    const DirectiveSpec& specs = it->second;
    return specs.allowsDuplicates;
}

const std::vector<std::string>& getConflictingDirectives(
    const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        throw std::logic_error("Unknown directive '" + name + "'");

    const DirectiveSpec& specs = it->second;
    return specs.conflictingDirectives;
}

} // namespace Directives
