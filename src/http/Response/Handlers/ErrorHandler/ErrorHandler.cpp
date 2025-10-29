#include "ErrorHandler.hpp"

namespace ErrorHandler
{
    std::string getErrorPageFilePath(int code, const std::vector<ErrorPage>& errorPages)
    {
        for (std::vector<ErrorPage>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
        {
            for (size_t i = 0; i < it->statusCodes.size(); ++i)
            {
                if (static_cast<int>(it->statusCodes[i]) == code)
                    return it->filePath;
            }
        }
        return "";
    }

    std::string generateErrorBody(int code, const std::vector<ErrorPage>& errorPages)
    {
        std::string errorBodyPath = getErrorPageFilePath(code, errorPages);
        std::string htmlBody;

        if (!errorBodyPath.empty())
        {
            std::ifstream file(errorBodyPath);
            if (file)
            {
                std::ostringstream ss;
                ss << file.rdbuf();
                htmlBody = ss.str();
            }
        }

        // Fallback if file not found or no custom error page
        if (htmlBody.empty())
        {
            htmlBody =
                "<html>\n"
                "<head><title>" + std::to_string(code) + " Error</title></head>\n"
                "<body>\n"
                "<center><h1>" + std::to_string(code) + " Error</h1></center>\n"
                "<hr><center>APT-Server/1.0</center>\n"
                "</body>\n"
                "</html>\n";
        }

        return htmlBody;
    }
}