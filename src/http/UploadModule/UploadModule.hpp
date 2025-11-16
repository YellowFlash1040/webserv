#pragma once

#ifndef UPLOADMODULE_HPP
# define UPLOADMODULE_HPP

# include "istream"
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
    // Methods
    static void processUpload(RequestData& req, RequestContext& ctx,
                              RawResponse& resp);

  private:
    // Methods
    static bool isMultipartFormData(const RequestData& req);
    static void processMultipartFormData(const RequestData& req,
                                         const std::string& uploadStore);
    static std::string extractDelimiter(const std::string& contentTypeHeader);
    static void processFormBody(const std::string& body,
                                const std::string& delimiter,
                                const std::string& uploadStore);
    static std::string extractFileName(const std::string& line);
    static void skipHeaders(std::istringstream& iss);
    static void saveFile(std::istringstream& iss, const std::string& delimiter,
                         const std::string& filename,
                         const std::string& uploadStore);
};

#endif
