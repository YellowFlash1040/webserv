#include "RequestResolver.hpp"

// ---------------------------METHODS-----------------------------

RequestContext RequestResolver::resolve(const HttpBlock& httpBlock,
                                        const NetworkEndpoint& endpoint,
                                        const std::string& host,
                                        const std::string& uri)
{
    EffectiveConfig config
        = createEffectiveConfig(httpBlock, endpoint, host, uri);
    return createContext(config, uri);
}

EffectiveConfig RequestResolver::createEffectiveConfig(
    const HttpBlock& httpBlock, const NetworkEndpoint& endpoint,
    const std::string& host, const std::string& uri)
{
    EffectiveConfig config;

    const ServerBlock& serverBlock
        = matchServerBlock(httpBlock.servers, endpoint, host);
    const LocationBlock* locationBlock = matchLocationBlock(serverBlock, uri);

    httpBlock.applyTo(config);
    serverBlock.applyTo(config);
    if (locationBlock)
        locationBlock->applyTo(config);

    return config;
}

const ServerBlock& RequestResolver::matchServerBlock(
    const std::vector<ServerBlock>& servers, const NetworkEndpoint& endpoint,
    const std::string& host)
{
    std::vector<const ServerBlock*> matchedServers
        = tryMatchByEndpoint(servers, endpoint);

    if (matchedServers.empty())
        throw std::runtime_error(
            "how did you even sent us a request? We don't listen on "
            + static_cast<std::string>(endpoint));

    if (matchedServers.size() == 1)
        return *matchedServers.front();

    // If multiple matches → match by host
    for (const ServerBlock* server : matchedServers)
        for (const std::string& serverName : server->serverName)
            if (serverName == host)
                return *server;

    // If none of the servers matched → return the first one
    return *matchedServers.front();
}

std::vector<const ServerBlock*> RequestResolver::tryMatchByEndpoint(
    const std::vector<ServerBlock>& servers, const NetworkEndpoint& endpoint)
{
    std::vector<const ServerBlock*> matchedServers;

    for (const ServerBlock& server : servers)
    {
        for (const NetworkEndpoint& listen : server.listen)
        {
            if (listen == endpoint)
            {
                matchedServers.push_back(&server);
                break; // no need to check more listens for this server
            }
        }
    }

    return matchedServers;
}

const LocationBlock* RequestResolver::matchLocationBlock(
    const ServerBlock& server, const std::string& uri)
{
    const LocationBlock* bestMatch = nullptr;
    std::size_t bestLength = 0;

    for (const LocationBlock& location : server.locations)
    {
        const std::string& path = location.path;

        if (uri.compare(0, path.size(), path) == 0 && path.size() > bestLength)
        {
            bestLength = path.size();
            bestMatch = &location;
        }
    }

    return bestMatch; // nullptr means "no matching location"
}

RequestContext RequestResolver::createContext(const EffectiveConfig& config,
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

    return context;
}

std::map<HttpStatusCode, std::string> RequestResolver::constructErrorPages(
    const std::vector<ErrorPage>& errorPages)
{
    std::map<HttpStatusCode, std::string> result;
    for (const auto& errorPage : errorPages)
        for (const auto& statusCode : errorPage.statusCodes)
            result[statusCode] = errorPage.filePath;

    return result;
}

std::string RequestResolver::resolvePath(const EffectiveConfig& config,
                                         const std::string& uri)
{
    if (!config.alias.empty())
        return handleAlias(config.alias, config.matched_location, uri);
    return handleRoot(config.root, uri);
}

std::string RequestResolver::handleAlias(const std::string& alias,
                                         const std::string& matched_location,
                                         const std::string& uri)
{
    if (alias.back() == '/' && matched_location.back() == '/')
        return alias + uri.substr(matched_location.size());
    return alias;
}

std::string RequestResolver::handleRoot(const std::string& root,
                                        const std::string& uri)
{
    if (root.back() == '/' && uri.front() == '/')
        return root + uri.substr(1); // remove duplicate slash
    else if (root.back() != '/' && uri.front() != '/')
        return root + "/" + uri; // add missing slash
    else
        return root + uri; // exactly one slash already
}
