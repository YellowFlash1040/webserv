#include "UploadModule.hpp"

using FormField = UploadModule::FormField;

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
Content-Disposition: form-data; name=\"files[]\"; filename=\"file.txt\"\r\n
Content-Type:text/plain\r\n\r\n

Some data inside txt file\nand a second line\n\r\n
------WebKitFormBoundary7sXEyrliWNq0uCE6\r\n
Content-Disposition: form-data; name=\"files[]\"; filename=\"file2.txt\"\r\n
Content-Type:text/plain\r\n\r\n

Data inside second file\n\r\n
------WebKitFormBoundary7sXEyrliWNq0uCE6--
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
    const std::string boundary = extractBoundary(req.getHeader("Content-Type"));
    if (boundary.empty())
        return;

    parseFormData(req.body, boundary);
    (void)uploadStore;
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

        formFields.push_back(
            {headers, body.substr(bodyStart, bodyEnd - bodyStart)});

        pos = nextBoundary; // continue to next part
    }

    return formFields;
}

void doSomething()
{
    // Find next boundary
    // Check for end boundary
    // Skip CRLF after boundary
    // Find separator between header and body
    // Extract headers
    // Find next boundary to get the body range
    // Trim the trailing \r\n before the next boundary
    // Extract body
    // Continue to next part
}

bool extractFormField(const std::string& body, const std::string& boundary,
                      size_t& pos, FormField& field)
{
    const std::string boundaryMarker = "--" + boundary;

    pos = body.find(boundaryMarker, pos) + boundaryMarker.size();
    if (body.compare(pos, 2, "--") == 0)
        return false;
    pos += strlen("\r\n");

    std::string headers = extractHeaders(body, pos);

    size_t bodyStart = pos;
    pos = body.find(boundaryMarker, pos);
    pos -= strlen("\r\n");
    size_t bodyEnd = pos;

    std::string fieldBody = body.substr(bodyStart, bodyEnd - bodyStart);

    field = {headers, fieldBody};

    return true;
}

std::string extractHeaders(const std::string& body, size_t& pos)
{
    size_t headersEnd = body.find("\r\n\r\n", pos);
    std::string headers = body.substr(pos, headersEnd - pos);
    pos = headersEnd + strlen("\r\n\r\n");
    return headers;
}

// void UploadModule::processFormBody(const std::string& body,
//                                    const std::string& delimiter,
//                                    const std::string& uploadStore)
// {
//     std::istringstream iss(body);
//     std::string line;
//     std::string filename;

//     while (std::getline(iss, line))
//     {
//         if (line.find("filename=") != std::string::npos)
//         {
//             filename = extractFileName(line);
//             break;
//         }
//     }
//     if (filename.empty())
//         return;

//     return;
// }

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
