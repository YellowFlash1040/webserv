#include "MimeTypeRecognizer.hpp"

// ---------------------------METHODS-----------------------------

std::string MimeTypeRecognizer::getExtension(const std::string& mime)
{
    std::string ext = getImageExtension(mime);
    if (!ext.empty())
        return ext;

    ext = getVideoExtension(mime);
    if (!ext.empty())
        return ext;

    ext = getTextExtension(mime);
    if (!ext.empty())
        return ext;

    ext = getPdfExtension(mime);
    if (!ext.empty())
        return ext;

    ext = getAudioExtension(mime);
    if (!ext.empty())
        return ext;

    ext = getArchiveExtension(mime);
    if (!ext.empty())
        return ext;

    ext = getFontExtension(mime);
    if (!ext.empty())
        return ext;

    return "";
}

std::string MimeTypeRecognizer::getImageExtension(const std::string& mime)
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

std::string MimeTypeRecognizer::getVideoExtension(const std::string& mime)
{
    if (mime == "video/mp4")
        return ".mp4";
    if (mime == "video/webm")
        return ".webm";
    if (mime == "video/ogg")
        return ".ogv";
    if (mime == "video/avi")
        return ".avi";
    return "";
}

std::string MimeTypeRecognizer::getTextExtension(const std::string& mime)
{
    if (mime == "text/plain")
        return ".txt";
    if (mime == "text/html")
        return ".html";
    if (mime == "text/csv")
        return ".csv";
    if (mime == "application/json")
        return ".json";
    if (mime == "application/xml")
        return ".xml";
    if (mime == "text/calendar")
        return ".ics";
    return "";
}

std::string MimeTypeRecognizer::getPdfExtension(const std::string& mime)
{
    if (mime == "application/pdf")
        return ".pdf";
    return "";
}

std::string MimeTypeRecognizer::getAudioExtension(const std::string& mime)
{
    if (mime == "audio/mpeg")
        return ".mp3";
    if (mime == "audio/wav")
        return ".wav";
    if (mime == "audio/ogg")
        return ".ogg";
    if (mime == "audio/flac")
        return ".flac";
    return "";
}

std::string MimeTypeRecognizer::getArchiveExtension(const std::string& mime)
{
    if (mime == "application/zip")
        return ".zip";
    if (mime == "application/x-tar")
        return ".tar";
    if (mime == "application/gzip")
        return ".gz";
    if (mime == "application/x-7z-compressed")
        return ".7z";
    return "";
}

std::string MimeTypeRecognizer::getFontExtension(const std::string& mime)
{
    if (mime == "application/x-font-ttf" || mime == "font/ttf")
        return ".ttf";
    if (mime == "font/otf")
        return ".otf";
    if (mime == "font/woff")
        return ".woff";
    if (mime == "font/woff2")
        return ".woff2";
    if (mime == "application/vnd.ms-fontobject")
        return ".eot";
    return "";
}

bool MimeTypeRecognizer::isImage(const std::string& mime)
{
    return !getImageExtension(mime).empty();
}

bool MimeTypeRecognizer::isVideo(const std::string& mime)
{
    return !getVideoExtension(mime).empty();
}

bool MimeTypeRecognizer::isTextFile(const std::string& mime)
{
    return !getTextExtension(mime).empty();
}

bool MimeTypeRecognizer::isPdf(const std::string& mime)
{
    return !getPdfExtension(mime).empty();
}

bool MimeTypeRecognizer::isAudio(const std::string& mime)
{
    return !getAudioExtension(mime).empty();
}

bool MimeTypeRecognizer::isArchive(const std::string& mime)
{
    return !getArchiveExtension(mime).empty();
}

bool MimeTypeRecognizer::isFont(const std::string& mime)
{
    return !getFontExtension(mime).empty();
}
