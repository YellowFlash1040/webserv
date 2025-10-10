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
    if (!block || block->name() != "global")
        throw std::logic_error(
            "The root node has to be a block directive 'global'");

    if (block->directives().size() > 1)
        throw std::logic_error("duplicate 'http' directive");

    validateChildren(*block, name);
}

void Validator::validateChildren(const BlockDirective& block,
                                 const std::string& parentContext)
{
    for (const auto& directive : block.directives())
        validateNode(directive, parentContext);
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

void Validator::checkIfAllowedDirective(const std::string& name,
                                        const std::string& context)
{
    if (!Directives::isAllowedInContext(name, context))
        throw DirectiveContextException(m_errorLine, m_errorColumn, name,
                                        context);
}

// void Validator::checkArguments(const std::string& name,
//                                const std::vector<Argument>& args)
// {
// if (!Directives::hasRightAmountOfArguments(name, args.size()))
// throw InvalidArgumentCountException(m_errorLine, m_errorColumn, name);

// const std::vector<Directives::ArgumentSpecs> argSpecs
//     = Directives::getArgSpecs(name);
// (void)argSpecs;
// (void)args;

/*
error_page 400 403 404 clientError.html
*/

// if (args.size() > argSpecs.size())
//     throw InvalidArgumentCountException(m_errorLine, m_errorColumn,
//     name);

// size_t i = 0;
// for (ArgumentSpecs argSpec : argSpecs)
// {
//     ArgumentType requiredType = argSpec.type;

//     size_t count = 0;
//     while (i < args.size())
//     {
//         try
//         {
//             convert(args[i]);
//             ++count;
//         }
//         catch (const std::exception&)
//         {
//             if (count < argSpec.minCount)
//                 throw std::logic_error("");
//             else if (count > argSpec.maxCount)
//                 throw std::logic_error("");
//             break;
//         }
//         ++i;
//     }
// }
// }

// Create a map <directive_name, args_validation_function>
// Create a function for each directive
// Create a class for each possible data type
// Use appropriate string -> class conversions and see the results

// map
// function
// class
// convert
// validate

void Validator::checkArguments(const std::string& name,
                               const std::vector<Argument>& args)
{
    const std::vector<Directives::ArgumentSpecs> argSpecs
        = Directives::getArgSpecs(name);

    /*
    error_page 400 403 404 clientError.html
    */

    size_t i = 0;
    for (const Directives::ArgumentSpecs& argSpec : argSpecs)
    {
        size_t count = 0;
        while (i < args.size())
        {
            try
            {
                validateArgument(argSpec.type, args[i]);
                ++count;
            }
            catch (const std::exception&)
            {
                if (count < argSpec.minCount)
                    throw std::logic_error("not enough arguments");
                else if (count > argSpec.maxCount)
                    throw std::logic_error("too many arguments");
                break;
            }
            ++i;
        }
    }

    if (i < args.size())
        throw std::logic_error("extra arguments after parsing");
}

void Validator::validateArgument(ArgumentType requiredType,
                                 const std::string& value)
{
    auto it = validators().find(requiredType);
    if (it == validators().end())
        throw std::logic_error("Unknown argument type");

    const auto& validateValue = it->second;
    validateValue(value);
}

///----------------///
///----------------///
///----------------///

const std::map<ArgumentType, std::function<void(const std::string&)>>&
Validator::validators()
{
    static const std::map<ArgumentType, std::function<void(const std::string&)>>
        map = {
            {ArgumentType::URL, validateUrl},
            {ArgumentType::Integer, validateInteger},
            {ArgumentType::StatusCode, validateStatusCode},
            {ArgumentType::DataSize, validateDataSize},
            {ArgumentType::OnOff, validateOnOff},
            {ArgumentType::FilePath, validateFilePath},
            {ArgumentType::NetworkEndpoint, validateNetworkEndpoint},
            {ArgumentType::HttpMethod, validateHttpMethod},
            {ArgumentType::String, validateString},
        };
    return map;
}

void Validator::validateStatusCode(const std::string& s)
{
    int code = std::stoi(s);
    if (code < 100 || code > 599)
        throw std::invalid_argument("Invalid status code: " + s);
}

void Validator::validateUrl(const std::string& s)
{
    if (s.empty() || s[0] != '/')
        throw std::invalid_argument("Invalid URL: " + s);
}

void Validator::validateOnOff(const std::string& s)
{
    if (s != "on" && s != "off")
        throw std::invalid_argument("Expected 'on' or 'off', got: " + s);
}

void Validator::validateFilePath(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("File path cannot be empty");
}

void Validator::validateInteger(const std::string& s)
{
    std::stoi(s);
}

void Validator::validateDataSize(const std::string& s)
{
    Converter::toBodySize(s);
}

void Validator::validateNetworkEndpoint(const std::string& s)
{
    (void)s;
    return;
}

void Validator::validateHttpMethod(const std::string& s)
{
    Converter::toHttpMethod(s);
}

void Validator::validateString(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("Argument cannot be empty");
}
