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

    const ServerBlock& serverBlock = httpBlock.matchServerBlock(endpoint, host);
    const LocationBlock* locationBlock = serverBlock.matchLocationBlock(uri);

    httpBlock.applyTo(config);
    serverBlock.applyTo(config);
    if (locationBlock)
        locationBlock->applyTo(config);

    return config;
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
