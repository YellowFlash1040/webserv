#include "utils.hpp"

void printReqContext(const RequestContext& ctx)
{
    DBG("\n" << TEAL << "---- RequestContext ----" << RESET);

    DBG("client_max_body_size: " << ctx.client_max_body_size);

    DBG("error_pages:");
    for (std::map<HttpStatusCode, std::string>::const_iterator it = ctx.error_pages.begin();
         it != ctx.error_pages.end(); ++it)
    {
        DBG("  statusCode: " << static_cast<int>(it->first)
            << " -> filePath: " << it->second);
    }

    DBG("resolved_path: " << ctx.resolved_path);
    DBG("autoindex_enabled: " << (ctx.autoindex_enabled ? "true" : "false"));

    DBG("index_files:");
    for (size_t i = 0; i < ctx.index_files.size(); ++i)
        DBG("  " << ctx.index_files[i]);

    DBG("upload_store: " << ctx.upload_store);

    DBG("cgi_pass:");
    for (std::map<std::string, std::string>::const_iterator it = ctx.cgi_pass.begin();
         it != ctx.cgi_pass.end(); ++it)
    {
        DBG("  " << it->first << " -> " << it->second);
    }

    DBG("allowed_methods:");
    for (size_t i = 0; i < ctx.allowed_methods.size(); ++i)
        DBG("  " << RawResponse::httpMethodToString(ctx.allowed_methods[i]));

    DBG("redirection: " << static_cast<int>(ctx.redirection.statusCode)
        << " " << ctx.redirection.url);

    DBG("matched_location: " << ctx.matched_location);

    DBG(TEAL << "------------------------" << RESET << "\n");
}

void printAllResponses(const ClientState& clientState)
{
    DBG("=== Response Queue (" << clientState.getResponseQueue().size() << " items) ===");

    std::queue<ResponseData> tempQueue = clientState.getResponseQueue();
    size_t index = 0;
    (void)index; // prevent unused-variable warning in case DBG is disabled

    while (!tempQueue.empty())
    {
        const ResponseData& resp = tempQueue.front();

        DBG("---- Response " << index++ << " ----");
        DBG("Status: " << resp.statusCode << " " << resp.statusText);
        DBG("Should Close: " << std::boolalpha << resp.shouldClose);

        DBG("Headers:");
        std::string contentType;
        for (const auto& [key, value] : resp.headers)
        {
            DBG("  " << key << ": " << value);
            if (key == "Content-Type")
                contentType = value;
        }

        DBG("Body (length=" << resp.body.size() << "):");
        if (contentType.find("text") != std::string::npos || contentType.empty())
        {
            DBG(resp.body);
        }
        else
        {
            DBG("[binary content, not displayed]");
        }

        tempQueue.pop();
    }

    DBG("=== End of Queue ===");
}

