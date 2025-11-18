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
    struct FileField
    {
        std::string fileName;
        std::string contents;
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
    static bool extractFormField(const std::string& body,
                                 const std::string& boundary, size_t& pos,
                                 FormField& outField);
    static size_t findNextBoundary(const std::string& body, size_t& pos,
                                   const std::string& boundary);
    static bool isClosingBoundary(const std::string& body, size_t start,
                                  const std::string& boundary);
    static size_t skipBoundary(size_t start, const std::string& boundary);
    static std::string extractHeaders(const std::string& body, size_t& pos);
    static FormField buildField(const std::string& body,
                                const std::string& headers, size_t start,
                                size_t end);
    static std::vector<FileField> searchForFiles(
        const std::vector<FormField>& formFields);
    static std::string extractFileName(const std::string& headers);
    static void saveFile(const FileField& file, const std::string& uploadStore);

    ////
    static void create201Response(RawResponse& resp);
    static void create415Response(RawResponse& resp);
};

#endif
