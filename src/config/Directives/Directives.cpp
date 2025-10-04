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
    std::set<std::string> allowedIn;
    ArgCountType argCount;
    size_t minArgs;
    size_t maxArgs;
    std::vector<ArgumentType> argTypes;
};

const std::map<std::string, DirectiveSpec> directives = {
    {HTTP, {
        DirectiveType::BLOCK,
        {GLOBAL_CONTEXT},
        FixedCount, 0, 0,
        {}
    }},
    {SERVER, {
        DirectiveType::BLOCK,
        {HTTP},
        FixedCount, 0, 0,
        {}
    }},
    {SERVER_NAME, {
        DirectiveType::SIMPLE,
        {SERVER},
        FixedCount, 1, 1,
        {ArgumentType::URL}
    }},
    {LISTEN, {
        DirectiveType::SIMPLE,
        {SERVER},
        FixedCount, 1, 1,
        {ArgumentType::NetworkEndpoint}
    }},
    {ERROR_PAGE, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        FixedCount, 2, 2,
        {ArgumentType::StatusCode, ArgumentType::URL}
    }},
    {CLIENT_MAX_BODY_SIZE, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        FixedCount, 1, 1,
        {ArgumentType::DataSize}
    }},
    {LOCATION, {
        DirectiveType::BLOCK,
        {SERVER},
        FixedCount, 1, 1,
        {ArgumentType::URL}
    }},
    {ACCEPTED_METHODS, {
        DirectiveType::SIMPLE,
        {LOCATION},
        Between, 1, 4,
        {ArgumentType::HttpMethod}
    }},
    {RETURN, {
        DirectiveType::SIMPLE,
        {SERVER, LOCATION},
        FixedCount, 2, 2,
        {ArgumentType::StatusCode, ArgumentType::URL}
    }},
    {ROOT, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        FixedCount, 1, 1,
        {ArgumentType::FilePath}
    }},
    {ALIAS, {
        DirectiveType::SIMPLE,
        {LOCATION},
        FixedCount, 1, 1,
        {ArgumentType::FilePath}
    }},
    {AUTOINDEX, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        FixedCount, 1, 1,
        {ArgumentType::OnOff}
    }},
    {INDEX, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        AtLeast, 1, static_cast<size_t>(-1),
        {ArgumentType::FilePath}
    }},
    {UPLOAD_STORE, {
        DirectiveType::SIMPLE,
        {LOCATION},
        FixedCount, 1, 1,
        {ArgumentType::FilePath}
    }},
    {CGI_PASS, {
        DirectiveType::SIMPLE,
        {LOCATION},
        FixedCount, 1, 1,
        {ArgumentType::FilePath}
    }}
};

bool isKnownDirective(const std::string& name)
{
    return directives.count(name) > 0;
}

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
    if (specs.allowedIn.count(context) == 0)
        return false;

    return true;
}

std::set<std::string> getAllowedContextsFor(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end()) // if we didn't find such a directive (which cannot happen)
        return {}; // Then it's not allowed anywhere

    DirectiveSpec specs = it->second;
    return specs.allowedIn;
}

bool hasRightAmountOfArguments(const std::string& name, size_t amount)
{
    auto it = directives.find(name);
    if (it == directives.end())
    return false;

    DirectiveSpec specs = it->second;

    if (specs.argCount == FixedCount
        && amount == specs.minArgs)
        return true;

    if (specs.argCount == AtLeast
        && amount >= specs.minArgs)
        return true;

    if (specs.argCount == Between
        && amount >= specs.minArgs
        && amount <= specs.maxArgs)
        return true;

    if (specs.argCount == Any)
        return true;
    
    return false;
}

// clang-format on

std::vector<ArgumentType> getDirectiveArgTypes(const std::string& directiveName)
{
    auto it = directives.find(directiveName);

    DirectiveSpec specs = it->second;
    return specs.argTypes;
}

// void validateDirectiveArgs(const std::string& name,
//                            const std::vector<Argument>& args)
// {
//     const DirectiveSpec& spec = directives.at(name);
//     std::vector<ArgumentType> argTypes = spec.argTypes;

//     if (!hasRightAmountOfArguments(name, args.size()))
//         throw InvalidArgumentCountException("error message");

//     for (size_t i = 0; i < args.size(); ++i)
//     {
//         if (args[i].type() != pattern[i])
//             throw std::logic_error("error message");
//     }
// }

} // namespace Directives
