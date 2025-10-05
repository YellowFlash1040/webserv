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

ServerBlock& HttpBlock::matchServerBlock(const std::string& host)
{
    for (ServerBlock& serverBlock : servers)
        if (serverBlock.serverName.value() == host)
            return serverBlock;
    return servers.value()[0];
}
