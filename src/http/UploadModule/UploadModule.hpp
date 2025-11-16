#pragma once

#ifndef UPLOADMODULE_HPP
# define UPLOADMODULE_HPP

# include <istream>
# include <vector>
# include <cstring>

# include "RequestContext.hpp"
# include "RequestData.hpp"
# include "RawResponse.hpp"

class UploadModule
{
    // Construction and destruction
  public:
    UploadModule() = delete;
    UploadModule(const UploadModule& other) = delete;
    UploadModule& operator=(const UploadModule& other) = delete;
    UploadModule(UploadModule&& other) noexcept = delete;
    UploadModule& operator=(UploadModule&& other) noexcept = delete;
    ~UploadModule() = delete;

    // Class specific features
  public:
    // Structs, Classes
    struct FormField
    {
        std::string headers;
        std::string body;
    };
    // Methods
    static void processUpload(RequestData& req, RequestContext& ctx,
                              RawResponse& resp);

  private:
    // Methods
    static bool isMultipartFormData(const RequestData& req);
    static void processMultipartFormData(const RequestData& req,
                                         const std::string& uploadStore);
    static std::string extractBoundary(const std::string& contentTypeHeader);
    static std::vector<FormField> parseFormData(const std::string& body,
                                                const std::string& boundary);

    static std::string extractFileName(const std::string& line);
};

bool extractFormField(const std::string& body, const std::string& boundary,
                      size_t& pos, UploadModule::FormField& field);
std::string extractHeaders(const std::string& body, size_t& pos);

#endif
