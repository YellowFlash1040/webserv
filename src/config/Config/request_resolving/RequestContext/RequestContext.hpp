#pragma once

#ifndef REQUESTCONTEXT_HPP
# define REQUESTCONTEXT_HPP

# include <string>
# include <vector>
# include <map>

# include "HttpMethod.hpp"
# include "HttpRedirection.hpp"
# include "HttpStatusCode.hpp"
# include "ErrorPage.hpp"

struct RequestContext
{
    HttpRedirection redirection{};
    std::vector<HttpMethod> allowed_methods{};
    size_t client_max_body_size{};
    std::string resolved_path{};
    std::vector<std::string> index_files{};
    bool autoindex_enabled{};
    std::map<HttpStatusCode, std::string> error_pages{};
    std::string upload_store{};
    std::map<std::string, std::string> cgi_pass{};
};

#endif
