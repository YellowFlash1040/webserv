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
    if (!block)
        throw std::logic_error(
            "The root node has to be a block directive 'global'");

    if (block->name() != "global")
        throw std::logic_error(
            "The root node has to be a block directive 'global'");

    validateChildren(*block, name);

    if (block->directives().size() > 1)
        throw std::logic_error("duplicate 'http' directive");
}

void Validator::validateNode(const std::unique_ptr<ADirective>& node,
                             const std::string& parentContext)
{
    const std::string& name = node->name();
    m_errorLine = node->line();
    m_errorColumn = node->column();

    checkIfAllowedDirective(name, parentContext);
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

void Validator::checkIfAllowedDirective(const std::string& name,
                                        const std::string& context)
{
    if (!Directives::isAllowedInContext(name, context))
        throw DirectiveContextException(m_errorLine, m_errorColumn, name,
                                        context);
}

void Validator::checkArguments(const std::string& name,
                               const std::vector<Argument>& args)
{
    if (!Directives::hasRightAmountOfArguments(name, args.size()))
        throw InvalidArgumentCountException(m_errorLine, m_errorColumn, name);

    // for (Argument arg : args)
    //     if (arg.type() !=)
}

// Create a map <directive_name, args_validation_function>
// Create a function for each directive
// Create a class for each possible data type
// Use appropriate string -> class conversions and see the results

// map
// function
// class
// convert
// validate