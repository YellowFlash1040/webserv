#pragma once

#ifndef MIMETYPERECOGNIZER_HPP
# define MIMETYPERECOGNIZER_HPP

# include <string>

class MimeTypeRecognizer
{
    // Construction and destruction
  public:
    MimeTypeRecognizer() = delete;
    MimeTypeRecognizer(const MimeTypeRecognizer& other) = delete;
    MimeTypeRecognizer& operator=(const MimeTypeRecognizer& other) = delete;
    MimeTypeRecognizer(MimeTypeRecognizer&& other) noexcept = delete;
    MimeTypeRecognizer& operator=(MimeTypeRecognizer&& other) noexcept = delete;
    ~MimeTypeRecognizer() = delete;

    // Class specific features
  public:
    // Methods
    static std::string getExtension(const std::string& mime);
    static bool isImage(const std::string& mime);
    static bool isVideo(const std::string& mime);
    static bool isTextFile(const std::string& mime);
    static bool isPdf(const std::string& mime);
    static bool isAudio(const std::string& mime);
    static bool isArchive(const std::string& mime);
    static bool isFont(const std::string& mime);

  private:
    static std::string getImageExtension(const std::string& mime);
    static std::string getVideoExtension(const std::string& mime);
    static std::string getTextExtension(const std::string& mime);
    static std::string getPdfExtension(const std::string& mime);
    static std::string getAudioExtension(const std::string& mime);
    static std::string getArchiveExtension(const std::string& mime);
    static std::string getFontExtension(const std::string& mime);
};

#endif
