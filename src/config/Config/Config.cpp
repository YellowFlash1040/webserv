#include "Config.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor

Config::Config(std::unique_ptr<Directive> ast)
{
    auto mainNode = dynamic_cast<BlockDirective*>(ast.get());
    if (!mainNode)
        throw std::invalid_argument("AST root node is not a block directive");

    m_httpBlock = buildHttpBlock(mainNode->directives()[0]);
    Validator::validate(m_httpBlock);
}

// Move constructor
Config::Config(Config&& other) noexcept
  : m_httpBlock(std::move(other.m_httpBlock))
{
}

// Move assignment operator
Config& Config::operator=(Config&& other) noexcept
{
    if (this != &other)
    {
        m_httpBlock = std::move(other.m_httpBlock);
    }
    return (*this);
}

// Destructor
Config::~Config() {}

// ---------------------------METHODS-----------------------------

Config Config::fromFile(const std::string& filepath)
{
    std::string content = FileReader::readFile(filepath);
    std::vector<Token> tokens = Lexer::tokenize(content);
    std::unique_ptr<Directive> ast = Parser::parse(tokens);
    Validator::validate(ast);

    return Config(std::move(ast));
}

std::vector<std::string> Config::getAllEnpoints()
{
    std::vector<std::string> endpoints;

    size_t totalListens = 0;
    for (const auto& server : m_httpBlock.servers)
        totalListens += server.listen->size();

    endpoints.reserve(totalListens);

    for (const auto& server : m_httpBlock.servers)
        for (const auto& listen : server.listen)
            endpoints.emplace_back(listen);

    return endpoints;
}

RequestContext Config::createRequestContext(const std::string& host,
                                            const std::string& uri)
{
    EffectiveConfig config = createEffectiveConfig(host, uri);

    return createContext(config, uri);
}

EffectiveConfig Config::createEffectiveConfig(const std::string& host,
                                              const std::string& uri)
{
    EffectiveConfig config;

    HttpBlock& httpBlock = m_httpBlock;
    const ServerBlock& serverBlock = httpBlock.matchServerBlock(host);
    const LocationBlock* locationBlock = serverBlock.matchLocationBlock(uri);

    httpBlock.applyTo(config);
    serverBlock.applyTo(config);
    if (locationBlock)
        locationBlock->applyTo(config);

    return config;
}

RequestContext Config::createContext(const EffectiveConfig& config,
                                     const std::string& uri)
{
    RequestContext context;

    context.allowed_methods = config.allowed_methods;
    context.autoindex_enabled = config.autoindex_enabled;
    context.cgi_pass = config.cgi_pass;
    context.client_max_body_size = config.client_max_body_size;
    context.error_pages = constructErrorPages(config.error_pages);
    context.index_files = config.index_files;
    context.resolved_path = resolvePath(config, uri);
    context.upload_store = config.upload_store;
    context.redirection = config.redirection;
    context.matched_location = config.matchedLocation;

    return context;
}

std::map<HttpStatusCode, std::string> Config::constructErrorPages(
    const std::vector<ErrorPage>& errorPages)
{
    std::map<HttpStatusCode, std::string> result;
    for (const auto& errorPage : errorPages)
    {
        for (const auto& statusCode : errorPage.statusCodes)
            result[statusCode] = errorPage.filePath;
    }

    return result;
}

// std::string Config::resolvePath(const EffectiveConfig& config,
//                                 const std::string& uri)
// {
//     if (config.alias.empty())
//         return config.root + uri;

//     if (config.alias.back() == '/')
//         return config.alias + uri.substr(config.matchedLocation.size());
//     return config.alias + "/" + uri.substr(config.matchedLocation.size());
// }

std::string Config::resolvePath(const EffectiveConfig& config,
                                const std::string& uri)
{
    const std::string* base = &config.root; // default to root
    std::string subpath;

    if (!config.alias.empty())
    {
        base = &config.alias;
        // remove the matched location part from the URI for alias
        if (uri.size() >= config.matchedLocation.size())
            subpath = uri.substr(config.matchedLocation.size());
        else
            subpath.clear();
    }
    else
        subpath = uri;

    // Ensure exactly one slash between base and subpath
    bool baseEndsWithSlash = !base->empty() && base->back() == '/';
    bool subpathStartsWithSlash = !subpath.empty() && subpath.front() == '/';

    if (baseEndsWithSlash && subpathStartsWithSlash)
        return *base + subpath.substr(1); // remove duplicate slash
    else if (!baseEndsWithSlash && !subpathStartsWithSlash)
        return *base + "/" + subpath; // add missing slash
    else
        return *base + subpath; // exactly one slash already
}

///----------------------------///
///----------------------------///
///----------------------------///

HttpBlock Config::buildHttpBlock(const std::unique_ptr<Directive>& httpNode)
{
    HttpBlock httpBlock;

    auto httpDirective = dynamic_cast<BlockDirective*>(httpNode.get());
    for (const auto& directive : httpDirective->directives())
    {
        const std::string& name = directive->name();
        const std::vector<Argument>& args = directive->args();
        if (name == Directives::SERVER)
        {
            httpBlock.servers->emplace_back(buildServerBlock(directive));
            httpBlock.servers.isSet() = true;
        }
        else if (name == Directives::CLIENT_MAX_BODY_SIZE)
            assign(httpBlock.clientMaxBodySize, args);
        else if (name == Directives::ERROR_PAGE)
            assign(httpBlock.errorPages, args);
        else if (name == Directives::ROOT)
            assign(httpBlock.root, args);
        else if (name == Directives::INDEX)
            assign(httpBlock.index, args);
        else if (name == Directives::AUTOINDEX)
            assign(httpBlock.autoindex, args);
    }

    return httpBlock;
}

ServerBlock Config::buildServerBlock(
    const std::unique_ptr<Directive>& serverNode)
{
    ServerBlock serverBlock;

    auto serverDirective = dynamic_cast<BlockDirective*>(serverNode.get());
    for (const auto& directive : serverDirective->directives())
    {
        const std::string& name = directive->name();
        const std::vector<Argument>& args = directive->args();
        if (name == Directives::LOCATION)
        {
            serverBlock.locations->emplace_back(buildLocationBlock(directive));
            serverBlock.locations.isSet() = true;
        }
        else if (name == Directives::LISTEN)
            assign(serverBlock.listen, args);
        else if (name == Directives::SERVER_NAME)
            assign(serverBlock.serverName, args);
        else if (name == Directives::ROOT)
            assign(serverBlock.root, args);
        else if (name == Directives::ERROR_PAGE)
            assign(serverBlock.errorPages, args);
        else if (name == Directives::CLIENT_MAX_BODY_SIZE)
            assign(serverBlock.clientMaxBodySize, args);
        else if (name == Directives::INDEX)
            assign(serverBlock.index, args);
        else if (name == Directives::AUTOINDEX)
            assign(serverBlock.autoindex, args);
        else if (name == Directives::RETURN)
            assign(serverBlock.httpRedirection, args);
    }

    if (serverBlock.listen->empty())
        serverBlock.listen->emplace_back("8080");

    if (serverBlock.serverName->empty())
        serverBlock.serverName->emplace_back("");

    return serverBlock;
}

LocationBlock Config::buildLocationBlock(
    const std::unique_ptr<Directive>& locationNode)
{
    LocationBlock locationBlock;

    auto locationDirective = dynamic_cast<BlockDirective*>(locationNode.get());

    locationBlock.path = locationDirective->args()[0];

    for (const auto& directive : locationDirective->directives())
    {
        const std::string& name = directive->name();
        const std::vector<Argument>& args = directive->args();
        if (name == Directives::ROOT)
            assign(locationBlock.root, args);
        else if (name == Directives::ALIAS)
            assign(locationBlock.alias, args);
        else if (name == Directives::AUTOINDEX)
            assign(locationBlock.autoindex, args);
        else if (name == Directives::INDEX)
            assign(locationBlock.index, args);
        else if (name == Directives::LIMIT_EXCEPT)
            assign(locationBlock.acceptedHttpMethods, args);
        else if (name == Directives::ERROR_PAGE)
            assign(locationBlock.errorPages, args);
        else if (name == Directives::RETURN)
            assign(locationBlock.httpRedirection, args);
        else if (name == Directives::CLIENT_MAX_BODY_SIZE)
            assign(locationBlock.clientMaxBodySize, args);
        else if (name == Directives::UPLOAD_STORE)
            assign(locationBlock.uploadStore, args);
        else if (name == Directives::CGI_PASS)
            assign(locationBlock.cgiPass, args);
    }

    if (locationBlock.acceptedHttpMethods->empty())
    {
        setDefaultHttpMethods(locationBlock.acceptedHttpMethods);
        locationBlock.acceptedHttpMethods.isSet() = true;
    }

    return locationBlock;
}

/*
    Maybe it makes sense to create somewhere in the code an
    array of function pointers and choose which function to
    apply
*/

///----------------///
///----------------///
///----------------///

void Config::assign(Property<std::string>& property,
                    const std::vector<Argument>& args)
{
    property = args[0];
}

void Config::assign(Property<bool>& property, const std::vector<Argument>& args)
{
    property = Converter::toBool(args[0]);
}

void Config::assign(Property<size_t>& property,
                    const std::vector<Argument>& args)
{
    property = Converter::toBodySize(args[0]);
}

void Config::assign(Property<std::vector<ErrorPage>>& errorPages,
                    const std::vector<Argument>& args)
{
    std::vector<HttpStatusCode> statusCodes;

    for (size_t i = 0; i < args.size() - 1; ++i)
        statusCodes.push_back(Converter::toHttpStatusCode(args[i]));

    const std::string& filePath = args.back();

    errorPages->emplace_back(statusCodes, filePath);

    errorPages.isSet() = true;
}

// void Config::assign(Property<std::map<HttpStatusCode, std::string>>&
// errorPages,
//                     const std::vector<Argument>& args)
// {
//     std::vector<HttpStatusCode> statusCodes;

//     for (size_t i = 0; i < args.size() - 1; ++i)
//     {
//         try
//         {
//             statusCodes.push_back(Converter::toHttpStatusCode(args[i]));
//         }
//         catch (const std::exception& ex)
//         {
//             const Argument& arg = args[i];
//             throw ConfigException(arg.line(), arg.column(), ex.what());
//         }
//     }

//     const std::string& filePath = args.back();

//     for (auto statusCode : statusCodes)
//         errorPages[statusCode] = filePath;

//     errorPages.isSet() = true;
// }

void Config::assign(Property<std::vector<HttpMethod>>& httpMethods,
                    const std::vector<Argument>& args)
{
    for (Argument arg : args)
        httpMethods->push_back(Converter::toHttpMethod(arg));

    httpMethods.isSet() = true;
}

void Config::assign(Property<std::vector<std::string>>& property,
                    const std::vector<Argument>& args)
{
    for (Argument arg : args)
        property->emplace_back(arg);

    property.isSet() = true;
}

void Config::assign(Property<HttpRedirection>& property,
                    const std::vector<Argument>& args)
{
    property->statusCode = Converter::toHttpStatusCode(args[0]);
    if (args.size() == 2)
        property->url = args[1];

    property.isSet() = true;
}

void Config::assign(Property<std::map<std::string, std::string>>& cgiPass,
                    const std::vector<Argument>& args)
{
    cgiPass[args[0]] = args[1];

    cgiPass.isSet() = true;
}

void Config::setDefaultHttpMethods(std::vector<HttpMethod>& httpMethods)
{
    httpMethods.push_back(HttpMethod::GET);
    httpMethods.push_back(HttpMethod::POST);
    httpMethods.push_back(HttpMethod::DELETE);
}
