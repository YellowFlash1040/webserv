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

HttpBlock& Config::httpBlock()
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

// std::vector<std::string> Config::getAllEnpoints()
// {
//     std::vector<ADirective*> listenDirs = findAll("listen", m_rootNode);

//     std::vector<std::string> endpoints;
//     endpoints.reserve(listenDirs.size());
//     for (const auto directive : listenDirs)
//         endpoints.push_back(directive->args()[0].value());

//     return endpoints;
// }

std::vector<ADirective*> Config::findAll(const std::string& directiveName,
                                         BlockDirective* block)
{
    std::vector<ADirective*> result;
    findAll(directiveName, block, result);
    return result;
}

void Config::findAll(const std::string& directiveName, BlockDirective* block,
                     std::vector<ADirective*>& result)
{
    for (const auto& directive : block->directives())
    {
        if (directive->name() == directiveName)
            result.push_back(directive.get());
        else if (auto childBlock
                 = dynamic_cast<BlockDirective*>(directive.get()))
            findAll(directiveName, childBlock, result);
    }
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
        if (directive->name() == Directives::SERVER)
        {
            httpBlock.servers->emplace_back(buildServerBlock(directive));
            httpBlock.servers.isSet() = true;
        }
        else if (directive->name() == Directives::CLIENT_MAX_BODY_SIZE)
        {
            size_t size = BodySize(directive->args()[0].value()).value();
            httpBlock.clientMaxBodySize = size;
        }
        else if (directive->name() == Directives::ERROR_PAGE)
        {
            auto statusCode = static_cast<HttpStatusCode>(
                std::stoi(directive->args()[0].value()));
            std::string filePath = directive->args()[1].value();

            std::vector<HttpStatusCode> statusCodes{};
            statusCodes.push_back(statusCode);

            httpBlock.errorPages->emplace_back(statusCodes, filePath);
            httpBlock.errorPages.isSet() = true;
        }
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
        if (directive->name() == Directives::LOCATION)
        {
            serverBlock.locations->emplace_back(buildLocationBlock(directive));
            serverBlock.locations.isSet() = true;
        }
        else if (directive->name() == Directives::LISTEN)
        {
            std::string networkEndpoint = directive->args()[0].value();
            serverBlock.listen = networkEndpoint;
        }
        else if (directive->name() == Directives::SERVER_NAME)
        {
            std::string serverName = directive->args()[0].value();
            serverBlock.serverName = serverName;
        }
        else if (directive->name() == Directives::ROOT)
        {
            std::string root = directive->args()[0].value();
            serverBlock.root = root;
        }
        else if (directive->name() == Directives::ALIAS)
        {
            std::string alias = directive->args()[0].value();
            serverBlock.alias = alias;
        }
        else if (directive->name() == Directives::ERROR_PAGE)
        {
            auto statusCode = static_cast<HttpStatusCode>(
                std::stoi(directive->args()[0].value()));
            std::string filePath = directive->args()[1].value();

            std::vector<HttpStatusCode> statusCodes{};
            statusCodes.push_back(statusCode);

            serverBlock.errorPages->emplace_back(statusCodes, filePath);
            serverBlock.errorPages.isSet() = true;
        }
        else if (directive->name() == Directives::CLIENT_MAX_BODY_SIZE)
        {
            size_t size = std::stoul(directive->args()[0].value());
            serverBlock.clientMaxBodySize = size;
        }
        else if (directive->name() == Directives::AUTOINDEX)
        {
            std::string value = directive->args()[0].value();

            if (value == "on")
                serverBlock.autoindex = true;
            else
                serverBlock.autoindex = false;
        }
    }

    return serverBlock;
}

LocationBlock Config::buildLocationBlock(
    const std::unique_ptr<ADirective>& locationNode)
{
    LocationBlock locationBlock;

    auto locationDirective = dynamic_cast<BlockDirective*>(locationNode.get());

    std::string path = locationDirective->args()[0].value();
    locationBlock.path = path;

    for (const auto& directive : locationDirective->directives())
    {
        if (directive->name() == Directives::ROOT)
        {
            std::string root = directive->args()[0].value();
            locationBlock.root = root;
        }
        else if (directive->name() == Directives::ALIAS)
        {
            std::string alias = directive->args()[0].value();
            locationBlock.alias = alias;
        }
        else if (directive->name() == Directives::AUTOINDEX)
        {
            std::string value = directive->args()[0].value();

            if (value == "on")
                locationBlock.autoindex = true;
            else
                locationBlock.autoindex = false;
        }
        else if (directive->name() == Directives::INDEX)
        {
            for (Argument arg : directive->args())
                locationBlock.index->emplace_back(arg.value());
            locationBlock.index.isSet() = true;
        }
        else if (directive->name() == Directives::ACCEPTED_METHODS)
        {
            for (Argument arg : directive->args())
                locationBlock.acceptedHttpMethods->emplace_back(arg.value());
            locationBlock.acceptedHttpMethods.isSet() = true;
        }
    }

    if (locationBlock.acceptedHttpMethods->empty())
    {
        locationBlock.acceptedHttpMethods->emplace_back("GET");
        locationBlock.acceptedHttpMethods->emplace_back("POST");
        locationBlock.acceptedHttpMethods->emplace_back("DELETE");
    }

    return locationBlock;
}

void Config::assign(std::string& property, const std::vector<std::string>& args)
{
    property = args[0];
}

void Config::assign(bool& property, const std::vector<std::string>& args)
{
    property = (args[0] == "on");
}

void Config::assign(size_t& property, const std::vector<std::string>& args)
{
    property = std::stoul(args[0]);
}

void Config::assign(std::vector<ErrorPage>& property,
                    const std::vector<std::string>& args)
{
    auto statusCode = static_cast<HttpStatusCode>(std::stoi(args[0]));
    property.emplace_back(std::vector{statusCode}, args[1]);
}

void Config::assign(std::vector<std::string>& property,
                    const std::vector<std::string>& args)
{
    property.insert(property.end(), args.begin(), args.end());
}

///----------------///
///----------------///
///----------------///

RequestContext Config::createRequestContext(const std::string& host,
                                            const std::string& url)
{
    RequestContext requestContext;

    HttpBlock& httpBlock = m_httpBlock;
    ServerBlock& serverBlock = httpBlock.matchServerBlock(host);
    LocationBlock* locationBlock = serverBlock.matchLocationBlock(url);

    httpBlock.applyTo(requestContext);
    serverBlock.applyTo(requestContext);
    locationBlock->applyTo(requestContext);

    return requestContext;
}
