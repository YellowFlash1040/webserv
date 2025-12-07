#include "LocationBlock.hpp"

// ---------------------------METHODS-----------------------------

void LocationBlock::applyTo(EffectiveConfig& config) const
{
    using namespace DirectiveAppliers;

    config.matched_location = path;

    applyIfSet(errorPages, config.error_pages, AppendTail{});
    applyIfSet(clientMaxBodySize, config.client_max_body_size, Replace{});
    applyIfSet(acceptedHttpMethods, config.allowed_methods, Replace{});
    applyIfSet(root, config.root, Replace{});
    applyIfSet(alias, config.alias, Replace{});
    applyIfSet(autoindex, config.autoindex_enabled, Replace{});
    applyIfSet(index, config.index_files, Replace{});
    applyIfSet(uploadStore, config.upload_store, Replace{});
    applyIfSet(cgiPass, config.cgi_pass, MergeMap{});

    if (httpRedirection.isSet() && !config.redirection.isSet)
    {
        config.redirection = httpRedirection;
        config.redirection.isSet = true;
    }
}
