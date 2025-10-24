#include "HttpBlock.hpp"

// ---------------------------METHODS-----------------------------

void HttpBlock::applyTo(RequestContext& context) const
{
    applyIfSet(clientMaxBodySize, context.client_max_body_size);
    applyIfSet(errorPages, context.error_pages);
    applyIfSet(root, context.root);
    applyIfSet(autoindex, context.autoindex_enabled);
    applyIfSet(index, context.index_files);
}

const ServerBlock& HttpBlock::matchServerBlock(const std::string& host) const
{
    for (const ServerBlock& serverBlock : servers)
    {
        for (const std::string& serverName : serverBlock.serverName)
            if (serverName == host)
                return serverBlock;
    }
    return servers->front();
}

const ServerBlock& HttpBlock::matchServerBlock(const std::string& endpoint,
                                               const std::string& host) const
{
    std::vector<const ServerBlock*> matchedServers;

    // Try match by endpoint
    for (const ServerBlock& server : servers)
    {
        for (const std::string& listen : server.listen)
        {
            if (listen == endpoint)
            {
                matchedServers.push_back(&server);
                break; // no need to check more listens for this server
            }
        }
    }

    if (matchedServers.size() == 1)
        return *matchedServers.front();

    // If multiple matches → match by host
    for (const ServerBlock* server : matchedServers)
    {
        for (const std::string& serverName : server->serverName)
        {
            if (serverName == host)
                return *server;
        }
    }

    // If none of the servers matched → return the first one
    return *matchedServers.front();
}
