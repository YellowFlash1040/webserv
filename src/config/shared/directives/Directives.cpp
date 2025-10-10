#include "Directives.hpp"

// clang-format off

namespace Directives
{

const std::map<std::string, DirectiveSpec> directives = {
    {HTTP, {
        Type::BLOCK,
        {GLOBAL_CONTEXT},
        {}
    }},
    {SERVER, {
        Type::BLOCK,
        {HTTP},
        {}
    }},
    {SERVER_NAME, {
        Type::SIMPLE,
        {SERVER},
        {{ArgumentType::URL, 1, UNLIMITED}}
    }},
    {LISTEN, {
        Type::SIMPLE,
        {SERVER},
        {{ArgumentType::NetworkEndpoint, 1, 1}}
    }},
    {ERROR_PAGE, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {
            {ArgumentType::StatusCode, 1, UNLIMITED},
            {ArgumentType::URL, 1, 1}
        }
    }},
    {CLIENT_MAX_BODY_SIZE, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{ArgumentType::DataSize, 1, 1}}
    }},
    {LOCATION, {
        Type::BLOCK,
        {SERVER},
        {{ArgumentType::URL, 1, 1}}
    }},
    {ACCEPTED_METHODS, {
        Type::SIMPLE,
        {LOCATION},
        {{ArgumentType::HttpMethod, 1, 4}}
    }},
    {RETURN, {
        Type::SIMPLE,
        {SERVER, LOCATION},
        {
            {ArgumentType::StatusCode, 1, 1},
            {ArgumentType::URL, 1, 1}
        }
    }},
    {ROOT, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{ArgumentType::FilePath, 1, 1}}
    }},
    {ALIAS, {
        Type::SIMPLE,
        {LOCATION},
        {{ArgumentType::FilePath, 1, 1}}
    }},
    {AUTOINDEX, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{ArgumentType::OnOff, 1, 1}}
    }},
    {INDEX, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{ArgumentType::FilePath, 1, UNLIMITED}}
    }},
    {UPLOAD_STORE, {
        Type::SIMPLE,
        {LOCATION},
        {{ArgumentType::FilePath, 1, 1}}
    }},
    {CGI_PASS, {
        Type::SIMPLE,
        {LOCATION},
        {{ArgumentType::FilePath, 1, 1}}
    }}
};

bool isKnownDirective(const std::string& name)
{
    return directives.count(name) > 0;
}

Type getDirectiveType(const std::string& name)
{
    if (isSimpleDirective(name))
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

bool isSimpleDirective(const std::string& name)
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
        throw std::logic_error("Unknow directive '" + name + "'");

    const DirectiveSpec& specs = it->second;
    return specs.allowedIn;
}

const std::vector<ArgumentSpecs>& getArgSpecs(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        throw std::logic_error("Unknow directive '" + name + "'");

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
