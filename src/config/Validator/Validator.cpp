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
    validateArguments(name, node->args());

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

void Validator::validateArguments(const std::string& name,
                                  const std::vector<Argument>& args)
{
    const std::vector<Directives::ArgumentSpec>& argSpecs
        = Directives::getArgSpecs(name);

    size_t i = 0;
    size_t lastThrowIndex = 0;
    for (const Directives::ArgumentSpec& spec : argSpecs)
    {
        size_t count = 0;
        bool hasThrown = false;
        while (i < args.size() && !hasThrown)
        {
            try
            {
                validateArgument(spec.possibleTypes, args[i]);
                ++count;
                ++i;
            }
            catch (const std::exception&)
            {
                if (lastThrowIndex == i)
                    // if it throws twice on the same index, it
                    // means that the argument is invalid
                    throw InvalidArgumentException(args[i]);
                lastThrowIndex = i;
                hasThrown = true;
            }
        }

        checkArgumentCount(name, spec, count);
    }

    if (i < args.size())
        throw InvalidArgumentException(args[i]);
}

void Validator::validateArgumentGroup(const std::string& name,
                                      const std::vector<Argument>& args,
                                      const Directives::ArgumentSpec& spec,
                                      size_t& i, size_t& lastThrowIndex)
{
    size_t count = 0;
    bool hasThrown = false;
    while (i < args.size() && !hasThrown)
    {
        try
        {
            validateArgument(spec.possibleTypes, args[i]);
            ++count;
            ++i;
        }
        catch (const std::exception&)
        {
            if (lastThrowIndex == i) // if it throws twice on the same index, it
                                     // means that the argument is invalid
                throw InvalidArgumentException(args[i]);
            lastThrowIndex = i;
            hasThrown = true;
        }
    }

    checkArgumentCount(name, spec, count);
}

void Validator::validateArgument(const std::vector<ArgumentType>& possibleTypes,
                                 const std::string& value)
{
    for (ArgumentType type : possibleTypes)
    {
        auto it = validators().find(type);
        if (it == validators().end())
            throw std::logic_error("No validator found");

        try
        {
            const auto& validateValue = it->second;
            validateValue(value); // if this succeeds, we're done
            return;
        }
        catch (const std::exception& ex)
        {
            // argument failed validation for this type, try next type
            continue;
        }
    }

    throw std::invalid_argument("Argument '" + value
                                + "' does not match any allowed type");
}

void Validator::checkArgumentCount(const std::string& name,
                                   const Directives::ArgumentSpec& spec,
                                   std::size_t count) const
{
    if (count < spec.minCount)
        throw NotEnoughArgumentsException(m_errorLine, m_errorColumn, name);

    if (count > spec.maxCount)
        throw TooManyArgumentsException(m_errorLine, m_errorColumn, name);
}

///----------------///
///----------------///
///----------------///

// clang-format off
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
            {ArgumentType::FolderPath, validateFolderPath},
            {ArgumentType::NetworkEndpoint, validateNetworkEndpoint},
            {ArgumentType::HttpMethod, validateHttpMethod},
            {ArgumentType::String, validateString},
            {ArgumentType::URI, validateUri}
        };
    return map;
}
// clang-format on

void Validator::validateStatusCode(const std::string& s)
{
    int code = std::stoi(s);
    if (code < 100 || code > 599)
        throw std::invalid_argument("Invalid status code: " + s);
}

void Validator::validateUrl(const std::string& s)
{
    if (s.empty() || s.find('.') == std::string::npos)
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
    // if (s.find('.') == std::string::npos)
    //     throw std::invalid_argument("File has to have an extension");
}

void Validator::validateFolderPath(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("File path cannot be empty");
    if (s[0] != '/')
        throw std::invalid_argument("Folder path has to start from a '/'");
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

void Validator::validateUri(const std::string& s)
{
    if (s.empty() || s[0] != '/')
        throw std::invalid_argument("Invalid URI: " + s);
}

//-------------------------THOUGHTS-------------------------------

// Create a map <directive_name, args_validation_function>
// Create a function for each directive
// Create a class for each possible data type
// Use appropriate string -> class conversions and see the results

// map
// function
// class
// convert
// validate
