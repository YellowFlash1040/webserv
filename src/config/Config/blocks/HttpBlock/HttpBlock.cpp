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
