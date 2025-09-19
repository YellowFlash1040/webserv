#include "Validator.hpp"

// ---------------------------METHODS-----------------------------

void Validator::validate(const std::unique_ptr<ADirective>& node)
{
    const std::string& name = node->name();

    const BlockDirective* block
        = dynamic_cast<const BlockDirective*>(node.get());
    if (block)
        validateChildren(*block, name);
}

void Validator::validateNode(const std::unique_ptr<ADirective>& node,
                             const std::string& parentContext)
{
    const std::string& name = node->name();

    checkParentConstraint(name, parentContext);
    checkAllowedDirective(name, parentContext);
    checkArguments(name, node->args());

    const BlockDirective* block
        = dynamic_cast<const BlockDirective*>(node.get());
    if (block)
        validateChildren(*block, name);
}

void Validator::validateChildren(const BlockDirective& block,
                                 const std::string& parentContext)
{
    for (const auto& directive : block.directives())
        validateNode(directive, parentContext);
}

void Validator::checkParentConstraint(const std::string& name,
                                      const std::string& parentContext)
{
    std::pair<bool, std::string> result
        = Directives::hasRequiredParentContext(name, parentContext);
    if (result.first)
        return;

    const std::string& requiredParent = result.second;
    throw DirectiveWrongParentException(name, requiredParent);
}

void Validator::checkAllowedDirective(const std::string& name,
                                      const std::string& context)
{
    if (!Directives::isAllowedInContext(name, context))
        throw DirectiveNotAllowedException(name, context);
}

void Validator::checkArguments(const std::string& name,
                               const std::vector<std::string>& args)
{
    if (!Directives::hasRightAmountOfArguments(name, args))
        throw ConfigException(" wrong amount of arguments: " + name);
}
