#include "ReqContextPrinter.hpp"
#include <iostream>

void printReqContext(const RequestContext& ctx)
{
    std::cout << TEAL << "---- RequestContext ----" << RESET << "\n";
    std::cout << "server_name: " << ctx.server_name << "\n";
    std::cout << "client_max_body_size: " << ctx.client_max_body_size << "\n";

    std::cout << "error_pages:\n";
    for (std::vector<ErrorPage>::const_iterator it = ctx.error_pages.begin();
         it != ctx.error_pages.end(); ++it)
    {
        std::cout << "  filePath: " << it->filePath << " -> statusCodes: ";
        for (size_t i = 0; i < it->statusCodes.size(); ++i)
        {
            std::cout << static_cast<int>(it->statusCodes[i]);
            if (i + 1 < it->statusCodes.size()) std::cout << ", ";
        }
        std::cout << "\n";
    }

    std::cout << "root: " << ctx.root << "\n";
    std::cout << "alias: " << ctx.alias << "\n";
    std::cout << "autoindex_enabled: " << (ctx.autoindex_enabled ? "true" : "false") << "\n";

    std::cout << "index_files:\n";
    for (size_t i = 0; i < ctx.index_files.size(); ++i)
        std::cout << "  " << ctx.index_files[i] << "\n";

    std::cout << "upload_store: " << ctx.upload_store << "\n";
    std::cout << "cgi_pass: " << ctx.cgi_pass << "\n";

    std::cout << "allowed_methods:\n";
    for (size_t i = 0; i < ctx.allowed_methods.size(); ++i)
        std::cout << "  " << ctx.allowed_methods[i].toString() << "\n";

    std::cout << "has_return: " << (ctx.has_return ? "true" : "false") << "\n";
    if (ctx.has_return)
        std::cout << "redirection: " << static_cast<int>(ctx.redirection.statusCode) << " " << ctx.redirection.url << "\n";

    std::cout << "matched_location: " << ctx.matched_location << "\n";
    std::cout << "------------------------\n";
}
