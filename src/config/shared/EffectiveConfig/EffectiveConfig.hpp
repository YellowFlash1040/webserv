#pragma once

#ifndef EFFECTIVE_CONFIG_HPP
# define EFFECTIVE_CONFIG_HPP

# include <string>
# include <vector>
# include <map>

#include "../../../http/HttpMethod/HttpMethod.hpp"
# include "HttpRedirection.hpp"
# include "ErrorPage.hpp"

struct EffectiveConfig
{
    std::string matchedLocation = "/";
    size_t client_max_body_size = 1024 * 1024;
    std::vector<ErrorPage> error_pages{};
    std::string root = "/var/www";
    std::string alias{};
    bool autoindex_enabled = false;
    std::vector<std::string> index_files = {"index.html"};
    std::string upload_store{};
    std::map<std::string, std::string> cgi_pass{};
    std::vector<HttpMethod> allowed_methods
        = {HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE};
    HttpRedirection redirection{};
};

#endif
