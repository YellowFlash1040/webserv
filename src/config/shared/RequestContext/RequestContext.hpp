#pragma once

#ifndef REQUESTCONTEXT_HPP
# define REQUESTCONTEXT_HPP

# include <string>
# include <vector>
# include <map>

# include "../../../http/HttpMethod/HttpMethod.hpp"
# include "HttpRedirection.hpp"
# include "ErrorPage.hpp"
// # include "HttpStatusCode.hpp"

// struct RequestContext
// {
//     size_t client_max_body_size = 1024 * 1024; // in bytes
//     // std::map<HttpStatusCode, std::string> error_pages{};
//     std::vector<ErrorPage> error_pages{}; // {status_code -> file path}
//     std::string root = "/var/www";        // filesystem root
//     std::string alias{};                  // if alias is used
//     bool autoindex_enabled = false;       // true if autoindex on
//     std::vector<std::string> index_files
//         = {"index.html"};       // ["index.html", "index.txt"]
//     std::string upload_store{}; // directory where uploads go
//     std::string cgi_pass{};     // program to run
//     std::vector<HttpMethod> allowed_methods
//         = {HttpMethod::GET, HttpMethod::POST,
//            HttpMethod::DELETE};    // GET, POST, etc.
//     bool has_return = false;       // true if return directive matched
//     HttpRedirection redirection{}; // e.g. 301 "http://.../"
//     std::string matched_location
//         = "/"; // path prefix of location, or "" if none
// };

// struct RequestContext
// {
//     HttpRedirection redirection{};
//     std::vector<HttpMethod> allowed_methods
//         = {HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE};
//     size_t client_max_body_size = 1024 * 1024;
//     std::string resolved_path{};
//     std::vector<std::string> index_files = {"index.html"};
//     bool autoindex_enabled = false;
//     std::map<HttpStatusCode, std::string> error_pages{};
//     std::string upload_store{};
//     std::map<std::string, std::string> cgi_pass{};
// };

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
    std::string matched_location{};
};

#endif
