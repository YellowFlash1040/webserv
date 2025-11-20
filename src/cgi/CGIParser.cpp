#include "CGIParser.hpp"
#include <stdexcept>
#include <algorithm>
#include <cctype>

CGIParser::CGIParser(std::string_view raw) : _raw(raw) {}

ParsedCGI CGIParser::parse(std::string_view cgi)
{
    CGIParser parser(cgi);
    return parser.run();
}

ParsedCGI CGIParser::run()
{
    ParsedCGI out;

    size_t sep = findSeparator();
    std::string_view header_part = _raw.substr(0, sep);
    out.body = std::string(_raw.substr(sep + _delimiter_len));

    parseHeaders(header_part, out);
    validate(out);

    return out;
}

size_t CGIParser::findSeparator()
{
    size_t pos = _raw.find("\r\n\r\n");
    if (pos != std::string_view::npos) {
        _delimiter_len = 4;
        return pos;
    }

    pos = _raw.find("\n\n");
    if (pos != std::string_view::npos) {
        _delimiter_len = 2;
        return pos;
    }

    throw std::runtime_error("CGI response missing header-body separator");
}

std::string CGIParser::trim(std::string_view sv)
{
    size_t start = 0;
    while (start < sv.size() && std::isspace((unsigned char)sv[start]))
        start++;

    size_t end = sv.size();
    while (end > start && std::isspace((unsigned char)sv[end - 1]))
        end--;

    return std::string(sv.substr(start, end - start));
}

void CGIParser::parseHeaders(std::string_view header_part, ParsedCGI &out)
{
    while (!header_part.empty()) {
        size_t line_end = header_part.find('\n');
        std::string_view line =
            (line_end == std::string_view::npos)
                ? header_part
                : header_part.substr(0, line_end);

        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);

        if (line.empty()) break;

        if (line_end == std::string_view::npos)
            header_part = {};
        else
            header_part = header_part.substr(line_end + 1);

        size_t colon = line.find(':');
        if (colon == std::string_view::npos)
            throw std::runtime_error("Invalid CGI header line");

        std::string name = std::string(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));

        if (name == "Status") {
            out.status = std::stoi(value);
        }
        else if (name == "Location") {
            out.headers["Location"] = value;
            out.is_redirect = true;
            if (out.status == 200)
                out.status = 302;
        }
        else {
            out.headers[name] = value;
        }
    }
}

void CGIParser::validate(ParsedCGI &out)
{
    if (!out.is_redirect && !out.headers.count("Content-Type"))
        throw std::runtime_error("CGI missing required Content-Type");
}
