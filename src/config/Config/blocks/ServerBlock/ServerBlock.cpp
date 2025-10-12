#include "ServerBlock.hpp"

// ---------------------------METHODS-----------------------------

void ServerBlock::applyTo(RequestContext& context) const
{
    applyIfSet(clientMaxBodySize, context.client_max_body_size);
    applyIfSet(errorPages, context.error_pages);
    applyIfSet(root, context.root);
    applyIfSet(alias, context.alias);
    applyIfSet(autoindex, context.autoindex_enabled);
    applyIfSet(index, context.index_files);
    applyIfSet(httpRedirection, context.redirection);
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

// /list/file.png
// /list/

// GET server.com/books

// GET server.com/list/

// GET server.com/list

// /var/www/files/list

// POST server.com/cgi-bin/pdf_splitter.php
// FILE attched

// POST server.com/upload/users/new
// File attached