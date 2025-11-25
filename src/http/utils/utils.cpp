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
        DBG("  " << httpMethodToString(ctx.allowed_methods[i]));

    DBG("redirection: " << static_cast<int>(ctx.redirection.statusCode)
        << " " << ctx.redirection.url);

    DBG("matched_location: " << ctx.matched_location);

    DBG(TEAL << "------------------------" << RESET << "\n");
}



bool equalsIgnoreCase(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
            return false;
    return true;
}

void removeCarriageReturns(std::string& str)
{
	str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
}

void trimLeadingWhitespace(std::string& str)
{
	str.erase(
		str.begin(),
		std::find_if(str.begin(), str.end(),
					 [](unsigned char ch) { return !std::isspace(ch); })
	);
}

bool isHex(char c)
{
	return (c >= '0' && c <= '9') ||
		   (c >= 'A' && c <= 'F') ||
		   (c >= 'a' && c <= 'f');
}