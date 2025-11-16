#include "UploadModule.hpp"

// ---------------------------METHODS-----------------------------

void UploadModule::processUpload(RequestData& req, RequestContext& ctx,
                                 RawResponse& resp)
{
    (void)resp;

    if (isMultipartFormData(req))
        processMultipartFormData(req, ctx.upload_store);
}

//"multipart/form-data; boundary=----WebKitFormBoundaryrVWI6fref2latTof"

/*
------WebKitFormBoundary7sXEyrliWNq0uCE6\r\n
Content-Disposition: form-data; name=\"file\"; filename=\"file.txt\"\r\n
Content-Type:text/plain\r\n\r\n

Some data inside txt file\nand a second line\n\r\n
------WebKitFormBoundaryrVWI6fref2latTof--
*/

bool UploadModule::isMultipartFormData(const RequestData& req)
{
    const std::string& headerValue = req.getHeader("Content-Type");
    const std::string& prefix = "multipart/form-data";

    return headerValue.compare(0, prefix.length(), prefix) == 0;
}

void UploadModule::processMultipartFormData(const RequestData& req,
                                            const std::string& uploadStore)
{
    const std::string delimiter
        = extractDelimiter(req.getHeader("Content-Type"));
    if (delimiter.empty())
        return;

    processFormBody(req.body, delimiter, uploadStore);
}

std::string UploadModule::extractDelimiter(const std::string& contentTypeHeader)
{
    const std::string prefix = "multipart/form-data; boundary=";
    if (contentTypeHeader.compare(0, prefix.length(), prefix) != 0)
        return {};

    return contentTypeHeader.substr(prefix.length());
}

void UploadModule::processFormBody(const std::string& body,
                                   const std::string& delimiter,
                                   const std::string& uploadStore)
{
    std::istringstream iss(body);
    std::string line;
    std::string filename;

    while (std::getline(iss, line))
    {
        if (line.find("filename=") != std::string::npos)
        {
            filename = extractFileName(line);
            break;
        }
    }
    if (filename.empty())
        return;

    skipHeaders(iss);
    // saveFile(iss, delimiter, filename, uploadStore);
    (void)delimiter;
    (void)uploadStore;
    return;
}

void UploadModule::skipHeaders(std::istringstream& iss)
{
    std::string prevLine;
    std::string line;
    while (std::getline(iss, line))
    {
        if (prevLine.back() == '\r' && line == "\r")
            break;
        prevLine = line;
    }
}

std::string UploadModule::extractFileName(const std::string& line)
{
    const std::string& prefix = "filename=\"";
    size_t openBracePos = line.find(prefix) + prefix.length();
    size_t closeBracePos = line.find('"', openBracePos);
    size_t filenameSize = closeBracePos - openBracePos;
    return line.substr(openBracePos, filenameSize);
}

// void UploadModule::saveFile(const std::string& body,
//                             const std::string& boundary,
//                             const std::string& filename,
//                             const std::string& uploadStore)
// {
//     const std::string startMarker = "\r\n\r\n"; // end of headers in part
//     size_t headersEnd = body.find(startMarker);
//     if (headersEnd == std::string::npos)
//         return;
//     size_t fileStart = headersEnd + startMarker.size();

//     const std::string endMarker = "\r\n--" + boundary + "--";
//     size_t fileEnd = body.find(endMarker, fileStart);
//     if (fileEnd == std::string::npos)
//         return;

//     size_t fileSize = fileEnd - fileStart;

//     std::ofstream os(uploadStore + "/" + filename, std::ios::binary);
//     os.write(body.data() + fileStart, fileSize);
// }

struct Part
{
    std::string headers;
    std::string body; // raw, exact bytes
};

std::vector<Part> parseMultipart(const std::string& body,
                                 const std::string& boundary)
{
    std::vector<Part> parts;

    const std::string boundaryMarker = "--" + boundary;
    const std::string endMarker = "--" + boundary + "--";

    size_t pos = 0;
    while (true)
    {
        // Find next boundary
        size_t start = body.find(boundaryMarker, pos);
        if (start == std::string::npos)
            break;
        start += boundaryMarker.size();

        // Check for end boundary
        bool isFinal = (body.compare(start, 2, "--") == 0);
        if (isFinal)
            break;

        // Skip CRLF after boundary
        if (body.compare(start, 2, "\r\n") == 0)
            start += 2;

        // Find separator between header and body
        size_t headersEnd = body.find("\r\n\r\n", start);
        if (headersEnd == std::string::npos)
            break;

        std::string headers = body.substr(start, headersEnd - start);

        // Find next boundary to get the body range
        size_t bodyStart = headersEnd + 4;
        size_t nextBoundary = body.find(boundaryMarker, bodyStart);

        if (nextBoundary == std::string::npos)
            break;

        // Trim the trailing \r\n before the next boundary
        size_t bodyEnd = nextBoundary;
        if (bodyEnd >= 2 && body.substr(bodyEnd - 2, 2) == "\r\n")
            bodyEnd -= 2;

        parts.push_back({headers, body.substr(bodyStart, bodyEnd - bodyStart)});

        pos = nextBoundary; // continue to next part
    }

    return parts;
}
