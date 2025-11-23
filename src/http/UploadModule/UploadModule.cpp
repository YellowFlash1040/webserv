#include "UploadModule.hpp"

using FormField = UploadModule::FormField;
using FileField = UploadModule::FileField;

// ---------------------------METHODS-----------------------------

//"multipart/form-data; boundary=----WebKitFormBoundary7sXEyrliWNq0uCE6"

/*
------WebKitFormBoundary7sXEyrliWNq0uCE6\r\n
Content-Disposition: form-data; name=\"files[]\"; filename=\"file.txt\"\r\n
Content-Type:text/plain\r\n\r\n

Some data inside txt file\nand a second line\n\r\n
------WebKitFormBoundary7sXEyrliWNq0uCE6\r\n
Content-Disposition: form-data; name=\"files[]\"; filename=\"file2.txt\"\r\n
Content-Type:text/plain\r\n\r\n

Data inside second file\n\r\n
------WebKitFormBoundary7sXEyrliWNq0uCE6--
*/

void UploadModule::processUpload(RequestData& req, RequestContext& ctx,
                                 RawResponse& resp)
{
    if (isMultipartFormData(req))
    {
        processMultipartFormData(req, ctx.upload_store);
        return create201Response(resp);
    }

    std::string extension = extensionFromMime(req.getHeader("Content-Type"));
    if (!extension.empty())
    {
        saveFile({generateRandomName() + extension, req.body},
                 ctx.upload_store);
        return create201Response(resp);
    }

    return create415Response(resp);
}

bool UploadModule::isMultipartFormData(const RequestData& req)
{
    const std::string& headerValue = req.getHeader("Content-Type");
    const std::string& prefix = "multipart/form-data";

    return headerValue.compare(0, prefix.length(), prefix) == 0;
}

void UploadModule::processMultipartFormData(const RequestData& req,
                                            const std::string& uploadStore)
{
    const std::string boundary = extractBoundary(req.getHeader("Content-Type"));
    if (boundary.empty())
        return;

    std::vector<FormField> formFields = parseFormData(req.body, boundary);
    std::vector<FileField> files = searchForFiles(formFields);
    for (FileField file : files)
        saveFile(file, uploadStore);
}

std::string UploadModule::extractBoundary(const std::string& contentTypeHeader)
{
    const std::string prefix = "multipart/form-data; boundary=";
    if (contentTypeHeader.compare(0, prefix.length(), prefix) != 0)
        return {};

    return contentTypeHeader.substr(prefix.length());
}

std::vector<FormField> UploadModule::parseFormData(const std::string& body,
                                                   const std::string& boundary)
{
    std::vector<FormField> formFields;

    FormField field{};
    size_t pos = 0;
    while (extractFormField(body, boundary, pos, field))
        formFields.push_back(field);

    return formFields;
}

bool UploadModule::extractFormField(const std::string& body,
                                    const std::string& boundary, size_t& pos,
                                    FormField& outField)
{
    const std::string boundaryMarker = "--" + boundary;

    size_t start = findNextBoundary(body, pos, boundaryMarker);
    if (start == std::string::npos
        || isClosingBoundary(body, start, boundaryMarker))
        return false;

    pos = skipBoundary(start, boundaryMarker);
    std::string headers = extractHeaders(body, pos);

    size_t bodyStart = pos;
    size_t bodyEnd
        = findNextBoundary(body, pos, boundaryMarker) - strlen("\r\n");
    if (bodyEnd == std::string::npos)
        return false;

    outField = buildField(body, headers, bodyStart, bodyEnd);

    return true;
}

size_t UploadModule::findNextBoundary(const std::string& body, size_t& pos,
                                      const std::string& boundaryMarker)
{
    return body.find(boundaryMarker, pos);
}

bool UploadModule::isClosingBoundary(const std::string& body, size_t start,
                                     const std::string& boundary)
{
    size_t pos = start + boundary.size();
    return body.compare(pos, 2, "--") == 0;
}

size_t UploadModule::skipBoundary(size_t start, const std::string& boundary)
{
    size_t pos = start + boundary.size() + strlen("\r\n");
    return pos;
}

std::string UploadModule::extractHeaders(const std::string& body, size_t& pos)
{
    size_t headersEnd = body.find("\r\n\r\n", pos);
    std::string headers = body.substr(pos, headersEnd - pos);
    pos = headersEnd + strlen("\r\n\r\n");
    return headers;
}

FormField UploadModule::buildField(const std::string& body,
                                   const std::string& headers, size_t start,
                                   size_t end)
{
    FormField f;
    f.headers = headers;
    f.body = body.substr(start, end - start);
    return f;
}

std::vector<FileField> UploadModule::searchForFiles(
    const std::vector<FormField>& formFields)
{
    std::vector<FileField> files;
    std::string filename;
    for (FormField field : formFields)
    {
        filename = extractFileName(field.headers);
        if (!filename.empty())
            files.push_back({filename, field.body});
    }
    return files;
}

std::string UploadModule::extractFileName(const std::string& headers)
{
    const std::string& prefix = "filename=\"";
    size_t prefixPos = headers.find(prefix);
    if (prefixPos == std::string::npos)
        return {};

    size_t openBracePos = prefixPos + prefix.length();
    size_t closeBracePos = headers.find('"', openBracePos);
    size_t filenameSize = closeBracePos - openBracePos;

    return headers.substr(openBracePos, filenameSize);
}

void UploadModule::saveFile(const FileField& file,
                            const std::string& uploadStore)
{
    std::ofstream os(uploadStore + "/" + file.fileName, std::ios::binary);
    if (!os)
        throw std::runtime_error("Failed to open file '" + file.fileName
                                 + "' for writing");
    os.write(file.contents.c_str(), file.contents.size());
}

std::string UploadModule::extensionFromMime(const std::string& mime)
{
    if (mime == "image/jpeg" || mime == "image/jpg")
        return ".jpg";
    if (mime == "image/png")
        return ".png";
    if (mime == "image/gif")
        return ".gif";
    if (mime == "image/webp")
        return ".webp";
    if (mime == "image/bmp")
        return ".bmp";
    return "";
}

std::string UploadModule::generateRandomName(size_t length)
{
    static const char charset[] = "0123456789"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz";

    std::mt19937 rng((unsigned)std::random_device{}());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; i++)
        result.push_back(charset[dist(rng)]);

    return result;
}

void UploadModule::create201Response(RawResponse& resp)
{
    resp.setStatus(HttpStatusCode::Created);
    resp.setDefaultHeaders();
    resp.addHeader("Content-Type", "application/json");
    resp.setBody(R"({
  "message": "File(s) uploaded successfully",
})");
}

void UploadModule::create415Response(RawResponse& resp)
{
    resp.setStatus(HttpStatusCode::UnsupportedMediaType);
    resp.setDefaultHeaders();
    resp.addHeader("Accept-Post", "multipart/form-data");
}
