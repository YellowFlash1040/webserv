#include "ServerBlock.hpp"

// ---------------------------METHODS-----------------------------

void ServerBlock::applyTo(EffectiveConfig& config) const
{
    using namespace DirectiveAppliers;

    applyIfSet(clientMaxBodySize, config.client_max_body_size, Replace{});
    applyIfSet(errorPages, config.error_pages, AppendTail{});
    applyIfSet(root, config.root, Replace{});
    applyIfSet(alias, config.alias, Replace{});
    applyIfSet(autoindex, config.autoindex_enabled, Replace{});
    applyIfSet(index, config.index_files, Replace{});

    if (httpRedirection.isSet() && !config.redirection.isSet)
    {
        config.redirection = httpRedirection;
        config.redirection.isSet = true;
    }
}

const LocationBlock* ServerBlock::matchLocationBlock(
    const std::string& uri) const
{
    const LocationBlock* bestMatch = nullptr;
    std::size_t bestLength = 0;

    for (const LocationBlock& location : locations)
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
