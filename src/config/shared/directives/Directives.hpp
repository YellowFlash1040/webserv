#pragma once

#ifndef DIRECTIVES_HPP
# define DIRECTIVES_HPP

# include <set>
# include <map>
# include <string>
# include <stdexcept>
# include <limits>

# include "Directive.hpp"
# include "BlockDirective.hpp"

namespace Directives
{

constexpr const char* GLOBAL_CONTEXT = "global";
constexpr const char* HTTP = "http";
constexpr const char* SERVER = "server";
constexpr const char* SERVER_NAME = "server_name";
constexpr const char* LISTEN = "listen";
constexpr const char* ERROR_PAGE = "error_page";
constexpr const char* CLIENT_MAX_BODY_SIZE = "client_max_body_size";
constexpr const char* LOCATION = "location";
constexpr const char* LIMIT_EXCEPT = "limit_except";
constexpr const char* RETURN = "return";
constexpr const char* ROOT = "root";
constexpr const char* ALIAS = "alias";
constexpr const char* AUTOINDEX = "autoindex";
constexpr const char* INDEX = "index";
constexpr const char* UPLOAD_STORE = "upload_store";
constexpr const char* CGI_PASS = "cgi_pass";

constexpr size_t UNLIMITED = std::numeric_limits<size_t>::max();

enum class Type
{
    UNKNOWN,
    SIMPLE,
    BLOCK
};

struct ArgumentSpec
{
    std::vector<ArgumentType> possibleTypes;
    size_t minCount;
    size_t maxCount;
};

struct DirectiveSpec
{
    Type type;
    std::set<std::string> allowedIn;
    std::vector<ArgumentSpec> argSpecs;
    std::vector<std::string> conflictingDirectives;
    bool allowsDuplicates;
};

bool isKnownDirective(const std::string& name);
Type getDirectiveType(const std::string& name);
bool isBlockDirective(const std::string& name);
bool isDirective(const std::string& name);
bool isAllowedInContext(const std::string& name, const std::string& context);
const std::set<std::string>& getAllowedContextsFor(const std::string& name);
const std::vector<ArgumentSpec>& getArgSpecs(const std::string& name);
bool allowsDuplicates(const std::string& name);
const std::vector<std::string>& getConflictingDirectives(
    const std::string& name);

// clang-format off
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
        {SERVER, LOCATION},
        {
            {{ArgumentType::FileExtension}, 1, 1},
            {{ArgumentType::BinaryPath}, 1, 1}
        },
        {},
        true
    }}
};

} // namespace Directives

#endif
