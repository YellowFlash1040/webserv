#include "Directives.hpp"

// clang-format off

namespace Directives
{

constexpr size_t UNLIMITED = static_cast<size_t>(-1);

enum ArgCountType
{
    FixedCount,
    Between,
    AtLeast,
    Any
};

struct ArgSpec
{
    ArgumentType type;
    size_t minCount;
    size_t maxCount;
}

struct DirectiveSpec
{
    DirectiveType type; // "simple" or "block"
    std::set<std::string> allowedIn;
    std::vector<ArgSpec> argumentSpecs;
};

const std::map<std::string, DirectiveSpec> directives = {
    {HTTP, {
        DirectiveType::BLOCK,
        {GLOBAL_CONTEXT},
        {}
    }},
    {SERVER, {
        DirectiveType::BLOCK,
        {HTTP},
        {}
    }},
    {SERVER_NAME, {
        DirectiveType::SIMPLE,
        {SERVER},
        {{ArgumentType::URL, 1, UNLIMITED}}
    }},
    {LISTEN, {
        DirectiveType::SIMPLE,
        {SERVER},
        {{ArgumentType::NetworkEndpoint, 1, 1}}
    }},
    {ERROR_PAGE, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {
            {ArgumentType::StatusCode, 1, UNLIMITED},
            {ArgumentType::URL, 1, 1}
        }
    }},
    {CLIENT_MAX_BODY_SIZE, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{ArgumentType::DataSize, 1, 1}}
    }},
    {LOCATION, {
        DirectiveType::BLOCK,
        {SERVER},
        {{ArgumentType::URL, 1, 1}}
    }},
    {ACCEPTED_METHODS, {
        DirectiveType::SIMPLE,
        {LOCATION},
        {{ArgumentType::HttpMethod, 1, 4}}
    }},
    {RETURN, {
        DirectiveType::SIMPLE,
        {SERVER, LOCATION},
        {
            {ArgumentType::StatusCode, 1, 1},
            {ArgumentType::URL, 1, 1}
        }
    }},
    {ROOT, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{ArgumentType::FilePath, 1, 1}}
    }},
    {ALIAS, {
        DirectiveType::SIMPLE,
        {LOCATION},
        {{ArgumentType::FilePath, 1, 1}}
    }},
    {AUTOINDEX, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{ArgumentType::OnOff, 1, 1}}
    }},
    {INDEX, {
        DirectiveType::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{ArgumentType::FilePath, 1, UNLIMITED}}
    }},
    {UPLOAD_STORE, {
        DirectiveType::SIMPLE,
        {LOCATION},
        {{ArgumentType::FilePath, 1, 1}}
    }},
    {CGI_PASS, {
        DirectiveType::SIMPLE,
        {LOCATION},
        {{ArgumentType::FilePath, 1, 1}}
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

const std::vector<ArgSpec>& getArgSpecs(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        return {};

    const DirectiveSpec& directiveSpecs = it->second;

    return directiveSpecs.argSpecs;
}

// clang-format on

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
