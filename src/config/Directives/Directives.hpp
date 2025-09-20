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

extern const std::set<std::string> blockDirectives;
extern const std::set<std::string> simpleDirectives;
extern const std::map<std::string, std::set<std::string>>
    directivesAllowedByContext;
extern const std::map<std::string, std::string> requiredParentContext;

DirectiveType getDirectiveType(const std::string& name);
bool isBlockDirective(const std::string& name);
bool isSimpleDirective(const std::string& name);
std::pair<bool, std::string> hasRequiredParentContext(
    const std::string& name, const std::string& parentContext);
bool isAllowedInContext(const std::string& name, const std::string& context);
bool hasRightAmountOfArguments(const std::string& name, size_t amount);

} // namespace Directives

#endif
