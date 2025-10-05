#include "LocationBlock.hpp"

// ---------------------------METHODS-----------------------------

void LocationBlock::applyTo(RequestContext& context) const
{
    applyIfSet(path, context.matched_location);
    applyIfSet(errorPages, context.error_pages);
    applyIfSet(clientMaxBodySize, context.client_max_body_size);
    applyIfSet(acceptedHttpMethods, context.allowed_methods);
    applyIfSet(httpRedirection, context.redirection);
    applyIfSet(root, context.root);
    applyIfSet(alias, context.alias);
    applyIfSet(autoindex, context.autoindex_enabled);
    applyIfSet(index, context.index_files);
    applyIfSet(uploadStore, context.upload_store);
    applyIfSet(cgiPass, context.cgi_pass);

    if (!acceptedHttpMethods.isSet())
    {
        context.allowed_methods.emplace_back("GET");
        context.allowed_methods.emplace_back("POST");
        context.allowed_methods.emplace_back("DELETE");
    }
}
