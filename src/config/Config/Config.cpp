#include "Config.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor

Config::Config(std::unique_ptr<ADirective> ast)
{
    auto rootNode = dynamic_cast<BlockDirective*>(ast.get());
    if (!rootNode)
        throw std::invalid_argument("AST root node is not a block directive");

    m_httpBlock = buildHttpBlock(rootNode->directives()[0]);
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

// ---------------------------ACCESSORS-----------------------------

const HttpBlock& Config::httpBlock() const
{
    return m_httpBlock;
}

// ---------------------------METHODS-----------------------------

Config Config::fromFile(const std::string& filepath)
{
    std::string content = FileReader::readFile(filepath);
    std::vector<Token> tokens = Lexer::tokenize(content);
    std::unique_ptr<ADirective> ast = Parser::parse(tokens);
    Validator::validate(ast);

    return Config(std::move(ast));
}

std::vector<std::string> Config::getAllEnpoints()
{
    std::vector<std::string> endpoints;
    endpoints.reserve(m_httpBlock.servers->size());
    for (const auto& server : m_httpBlock.servers)
        endpoints.emplace_back(server.listen);

    return endpoints;
}

RequestContext Config::createRequestContext(const std::string& host,
                                            const std::string& uri)
{
    RequestContext requestContext;

    HttpBlock& httpBlock = m_httpBlock;
    const ServerBlock& serverBlock = httpBlock.matchServerBlock(host);
    const LocationBlock* locationBlock = serverBlock.matchLocationBlock(uri);

    httpBlock.applyTo(requestContext);
    serverBlock.applyTo(requestContext);
    locationBlock->applyTo(requestContext);

    return requestContext;
}

///----------------------------///
///----------------------------///
///----------------------------///

HttpBlock Config::buildHttpBlock(const std::unique_ptr<ADirective>& httpNode)
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
    }

    return httpBlock;
}

ServerBlock Config::buildServerBlock(
    const std::unique_ptr<ADirective>& serverNode)
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
        else if (name == Directives::ALIAS)
            assign(serverBlock.alias, args);
        else if (name == Directives::ERROR_PAGE)
            assign(serverBlock.errorPages, args);
        else if (name == Directives::CLIENT_MAX_BODY_SIZE)
            assign(serverBlock.clientMaxBodySize, args);
        else if (name == Directives::AUTOINDEX)
            assign(serverBlock.autoindex, args);
    }

    return serverBlock;
}

LocationBlock Config::buildLocationBlock(
    const std::unique_ptr<ADirective>& locationNode)
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
        else if (name == Directives::ACCEPTED_METHODS)
            assign(locationBlock.acceptedHttpMethods, args);
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
    try
    {
        property = args[0];
    }
    catch (const std::exception& ex)
    {
        const Argument& arg = args[0];
        throw ConfigException(arg.line(), arg.column(), ex.what());
    }
}

void Config::assign(Property<bool>& property, const std::vector<Argument>& args)
{
    try
    {
        property = Converter::toBool(args[0]);
    }
    catch (const std::exception& ex)
    {
        const Argument& arg = args[0];
        throw ConfigException(arg.line(), arg.column(), ex.what());
    }
}

void Config::assign(Property<size_t>& property,
                    const std::vector<Argument>& args)
{
    try
    {
        property = Converter::toBodySize(args[0]);
    }
    catch (const std::exception& ex)
    {
        const Argument& arg = args[0];
        throw ConfigException(arg.line(), arg.column(), ex.what());
    }
}

void Config::assign(Property<std::vector<ErrorPage>>& errorPages,
                    const std::vector<Argument>& args)
{
    std::vector<HttpStatusCode> statusCodes;

    for (size_t i = 0; i < args.size() - 1; ++i)
    {
        try
        {
            statusCodes.push_back(Converter::toHttpStatusCode(args[i]));
        }
        catch (const std::exception& ex)
        {
            const Argument& arg = args[i];
            throw ConfigException(arg.line(), arg.column(), ex.what());
        }
    }

    const std::string& filePath = args.back();
    errorPages->emplace_back(statusCodes, filePath);

    errorPages.isSet() = true;
}

void Config::assign(Property<std::vector<HttpMethod>>& httpMethods,
                    const std::vector<Argument>& args)
{
    for (Argument arg : args)
    {
        try
        {
            httpMethods->push_back(Converter::toHttpMethod(arg));
        }
        catch (const std::exception& ex)
        {
            throw ConfigException(arg.line(), arg.column(), ex.what());
        }
    }

    httpMethods.isSet() = true;
}

void Config::assign(Property<std::vector<std::string>>& property,
                    const std::vector<Argument>& args)
{
    for (Argument arg : args)
    {
        try
        {
            property->emplace_back(arg);
        }
        catch (const std::exception& ex)
        {
            throw ConfigException(arg.line(), arg.column(), ex.what());
        }
    }

    property.isSet() = true;
}

void Config::setDefaultHttpMethods(std::vector<HttpMethod>& httpMethods)
{
    httpMethods.push_back(HttpMethod::GET);
    httpMethods.push_back(HttpMethod::POST);
    httpMethods.push_back(HttpMethod::DELETE);
}
