#pragma once

#ifndef REQUESTCONTEXT_HPP
# define REQUESTCONTEXT_HPP

# include <string>
# include <vector>
# include <map>

# include "ErrorPage.hpp"
# include "HttpMethod.hpp"
# include "HttpRedirection.hpp"

struct RequestContext
{
    std::string server_name{};                 // host
    size_t client_max_body_size{};             // in bytes
    std::vector<ErrorPage> error_pages{};      // {status_code -> file path}
    std::string root{};                        // filesystem root
    std::string alias{};                       // if alias is used
    bool autoindex_enabled{};                  // true if autoindex on
    std::vector<std::string> index_files{};    // ["index.html", "index.txt"]
    std::string upload_store{};                // directory where uploads go
    std::string cgi_pass{};                    // program to run
    std::vector<HttpMethod> allowed_methods{}; // GET, POST, etc.
    bool has_return{};              // true if return directive matched
    HttpRedirection redirection{};  // e.g. 301 "http://.../"
    std::string matched_location{}; // path prefix of location, or "" if none
};

#endif
