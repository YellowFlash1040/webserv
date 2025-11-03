#include "LocationBlock.hpp"

// ---------------------------METHODS-----------------------------

// void LocationBlock::applyTo(RequestContext& context) const
// {
//     applyIfSet(errorPages, context.error_pages, Rules::applyErrorPages);
//     applyIfSet(clientMaxBodySize, context.client_max_body_size,
//                Rules::Replace{});
//     applyIfSet(acceptedHttpMethods, context.allowed_methods,
//     Rules::Replace{}); applyIfSet(httpRedirection, context.redirection,
//     Rules::Replace{}); applyIfSet(root, context.resolved_path,
//     Rules::Replace{}); applyIfSet(alias, context.resolved_path,
//     Rules::Replace{}); applyIfSet(autoindex, context.autoindex_enabled,
//     Rules::Replace{}); applyIfSet(index, context.index_files,
//     Rules::AppendHead{}); applyIfSet(uploadStore, context.upload_store,
//     Rules::Replace{}); applyIfSet(cgiPass, context.cgi_pass,
//     Rules::MergeMap{});
// }

void LocationBlock::applyTo(EffectiveConfig& context) const
{
    context.matchedLocation = path;

    applyIfSet(errorPages, context.error_pages, Rules::AppendTail{});
    applyIfSet(clientMaxBodySize, context.client_max_body_size,
               Rules::Replace{});
    applyIfSet(acceptedHttpMethods, context.allowed_methods, Rules::Replace{});
    applyIfSet(root, context.root, Rules::Replace{});
    applyIfSet(alias, context.alias, Rules::Replace{});
    applyIfSet(autoindex, context.autoindex_enabled, Rules::Replace{});
    applyIfSet(index, context.index_files, Rules::Replace{});
    applyIfSet(uploadStore, context.upload_store, Rules::Replace{});
    applyIfSet(cgiPass, context.cgi_pass, Rules::MergeMap{});

    if (httpRedirection.isSet() && !context.redirection.isSet)
    {
        context.redirection = httpRedirection;
        context.redirection.isSet = true;
    }
}
