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
