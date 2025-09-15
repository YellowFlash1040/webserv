#pragma once

#ifndef DIRECTIVES_HPP
# define DIRECTIVES_HPP

# include <unordered_set>
# include <string>

enum class DirectiveType
{
    UNKNOWN,
    SIMPLE,
    BLOCK
};

namespace Directives
{

extern const std::unordered_set<std::string> blockDirectives;
extern const std::unordered_set<std::string> simpleDirectives;

DirectiveType getDirectiveType(const std::string& name);
bool isBlockDirective(const std::string& name);
bool isSimpleDirective(const std::string& name);

} // namespace Directives

#endif
