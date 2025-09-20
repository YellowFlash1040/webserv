#include "Validator.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Validator::Validator(const std::unique_ptr<ADirective>& rootNode)
  : m_rootNode(rootNode)
{
}

// Destructor
Validator::~Validator() {}

// ---------------------------METHODS-----------------------------

void Validator::validate(const std::unique_ptr<ADirective>& rootNode)
{
    Validator(rootNode).validate();
}

void Validator::validate()
{
    const std::string& name = m_rootNode->name();

    const BlockDirective* block
        = dynamic_cast<const BlockDirective*>(m_rootNode.get());
    if (block)
        validateChildren(*block, name);
}

void Validator::validateNode(const std::unique_ptr<ADirective>& node,
                             const std::string& parentContext)
{
    const std::string& name = node->name();
    m_errorLine = node->line();
    m_errorColumn = node->column();

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
    throw DirectiveWrongParentException(m_errorLine, m_errorColumn, name,
                                        requiredParent);
}

void Validator::checkAllowedDirective(const std::string& name,
                                      const std::string& context)
{
    if (!Directives::isAllowedInContext(name, context))
        throw DirectiveNotAllowedException(m_errorLine, m_errorColumn, name,
                                           context);
}

void Validator::checkArguments(const std::string& name,
                               const std::vector<Argument>& args)
{
    if (!Directives::hasRightAmountOfArguments(name, args.size()))
        throw InvalidArgumentCountException(m_errorLine, m_errorColumn, name);
}
