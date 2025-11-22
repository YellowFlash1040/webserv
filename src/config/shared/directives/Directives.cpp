#include "Directives.hpp"

// clang-format off

namespace Directives
{

const std::map<std::string, DirectiveSpec> directives = {
    {HTTP, {
        Type::BLOCK,
        {GLOBAL_CONTEXT},
        {},
        {},
        false
    }},
    {SERVER, {
        Type::BLOCK,
        {HTTP},
        {},
        {},
        true
    }},
    {SERVER_NAME, {
        Type::SIMPLE,
        {SERVER},
        {{{ArgumentType::Name}, 1, UNLIMITED}},
        {},
        false
    }},
    {LISTEN, {
        Type::SIMPLE,
        {SERVER},
        {{{ArgumentType::NetworkEndpoint, ArgumentType::Port, ArgumentType::Ip}, 1, 1}},
        {},
        true
    }},
    {ERROR_PAGE, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {
            {{ArgumentType::StatusCode}, 1, UNLIMITED},
            {{ArgumentType::FilePath, ArgumentType::File}, 1, 1}
        },
        {},
        true
    }},
    {CLIENT_MAX_BODY_SIZE, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{{ArgumentType::DataSize}, 1, 1}},
        {},
        false
    }},
    {LOCATION, {
        Type::BLOCK,
        {SERVER},
        {{{ArgumentType::URI}, 1, 1}},
        {},
        true
    }},
    {LIMIT_EXCEPT, {
        Type::SIMPLE,
        {LOCATION},
        {{{ArgumentType::HttpMethod}, 1, UNLIMITED}},
        {},
        false
    }},
    {RETURN, {
        Type::SIMPLE,
        {SERVER, LOCATION},
        {
            {{ArgumentType::ReturnStatusCode}, 1, 1},
            {{ArgumentType::URL}, 0, 1},
        },
        {},
        false
    }},
    {ROOT, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{{ArgumentType::FolderPath}, 1, 1}},
        {ALIAS},
        false
    }},
    {ALIAS, {
        Type::SIMPLE,
        {LOCATION},
        {{{ArgumentType::FolderPath}, 1, 1}},
        {ROOT},
        false
    }},
    {AUTOINDEX, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{{ArgumentType::OnOff}, 1, 1}},
        {},
        false
    }},
    {INDEX, {
        Type::SIMPLE,
        {HTTP, SERVER, LOCATION},
        {{{ArgumentType::File}, 1, UNLIMITED}},
        {},
        false
    }},
    {UPLOAD_STORE, {
        Type::SIMPLE,
        {SERVER, LOCATION},
        {{{ArgumentType::FolderPath}, 1, 1}},
        {},
        false
    }},
    {CGI_PASS, {
        Type::SIMPLE,
        {LOCATION},
        {
            {{ArgumentType::FileExtension}, 1, 1},
            {{ArgumentType::BinaryPath}, 1, 1}
        },
        {},
        true
    }}
};

bool isKnownDirective(const std::string& name)
{
    return directives.count(name) > 0;
}

Type getDirectiveType(const std::string& name)
{
    if (isDirective(name))
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

bool isDirective(const std::string& name)
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
        throw std::logic_error("Unknown directive '" + name + "'");

    const DirectiveSpec& specs = it->second;
    return specs.allowedIn;
}

const std::vector<ArgumentSpec>& getArgSpecs(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        throw std::logic_error("Unknown directive '" + name + "'");

    const DirectiveSpec& directiveSpecs = it->second;
    return directiveSpecs.argSpecs;
}

bool allowsDuplicates(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        throw std::logic_error("Unknown directive '" + name + "'");

    const DirectiveSpec& specs = it->second;
    return specs.allowsDuplicates;
}

const std::vector<std::string>& getConflictingDirectives(const std::string& name)
{
    auto it = directives.find(name);
    if (it == directives.end())
        throw std::logic_error("Unknown directive '" + name + "'");

    const DirectiveSpec& specs = it->second;
    return specs.conflictingDirectives;
}

// clang-format on

} // namespace Directives
