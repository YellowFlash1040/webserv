#include "HttpBlock.hpp"

// ---------------------------METHODS-----------------------------

void HttpBlock::applyTo(EffectiveConfig& config) const
{
    using namespace DirectiveAppliers;

    applyIfSet(clientMaxBodySize, config.client_max_body_size, Replace{});
    applyIfSet(errorPages, config.error_pages, AppendTail{});
    applyIfSet(root, config.root, Replace{});
    applyIfSet(autoindex, config.autoindex_enabled, Replace{});
    applyIfSet(index, config.index_files, Replace{});
}

const ServerBlock& HttpBlock::matchServerBlock(const NetworkEndpoint& endpoint,
                                               const std::string& host) const
{
    std::vector<const ServerBlock*> matchedServers;

    // Try match by endpoint
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
