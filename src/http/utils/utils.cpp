#include "utils.hpp"

void printReqContext(const RequestContext& ctx)
{
    std::cout << TEAL << "---- RequestContext ----" << RESET << "\n";
    std::cout << "client_max_body_size: " << ctx.client_max_body_size << "\n";

    std::cout << "error_pages:\n";
	for (std::map<HttpStatusCode, std::string>::const_iterator it = ctx.error_pages.begin();
		it != ctx.error_pages.end(); ++it)
	{
		std::cout << "  statusCode: " << static_cast<int>(it->first)
				<< " -> filePath: " << it->second << "\n";
	}

    std::cout << "resolved_path: " << ctx.resolved_path << "\n";
    std::cout << "autoindex_enabled: " << (ctx.autoindex_enabled ? "true" : "false") << "\n";

    std::cout << "index_files:\n";
    for (size_t i = 0; i < ctx.index_files.size(); ++i)
        std::cout << "  " << ctx.index_files[i] << "\n";

    std::cout << "upload_store: " << ctx.upload_store << "\n";
    std::cout << "cgi_pass:\n";
	for (std::map<std::string, std::string>::const_iterator it = ctx.cgi_pass.begin();
		it != ctx.cgi_pass.end(); ++it)
	{
		std::cout << "  " << it->first << " -> " << it->second << "\n";
	}

    std::cout << "allowed_methods:\n";
    for (size_t i = 0; i < ctx.allowed_methods.size(); ++i)
		std::cout << "  " << RawResponse::httpMethodToString(ctx.allowed_methods[i]) << "\n";

    std::cout << "redirection: " << static_cast<int>(ctx.redirection.statusCode) << " " << ctx.redirection.url << "\n";

    std::cout << "matched_location: " << ctx.matched_location << "\n";
    std::cout << TEAL << "------------------------" << RESET "\n";
}

void printAllResponses(const ClientState& clientState)
{
    const std::queue<ResponseData>& queue = clientState.getResponseQueue();
    std::cout << "=== Response Queue (" << queue.size() << " items) ===\n";

    std::queue<ResponseData> tempQueue = queue; // copy for iteration
    size_t index = 0;

    while (!tempQueue.empty())
    {
        const ResponseData& resp = tempQueue.front();
        std::cout << "---- Response " << index++ << " ----\n";
        std::cout << "Status: " << resp.statusCode << " " << resp.statusText << "\n";
        std::cout << "Should Close: " << std::boolalpha << resp.shouldClose << "\n";

        std::cout << "Headers:\n";
        std::string contentType;
        for (const auto& [key, value] : resp.headers)
        {
            std::cout << "  " << key << ": " << value << "\n";
            if (key == "Content-Type")
                contentType = value;
        }

        std::cout << "Body (length=" << resp.body.size() << "):\n";
        if (contentType.find("text") != std::string::npos || contentType.empty())
        {
            std::cout << resp.body << "\n";
        }
        else
        {
            std::cout << "[binary content, not displayed]\n";
        }

        tempQueue.pop();
    }

    std::cout << "=== End of Queue ===\n";
}

