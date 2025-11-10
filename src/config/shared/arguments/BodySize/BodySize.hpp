#pragma once

#ifndef BODYSIZE_HPP
# define BODYSIZE_HPP

# include <utility>
# include <string>
# include <algorithm>
# include <stdexcept>

class BodySize
{
    // Construction and destruction
  public:
    BodySize();
    explicit BodySize(const std::string& value);
    BodySize(const BodySize& other);
    BodySize& operator=(const BodySize& other);
    BodySize(BodySize&& other) noexcept;
    BodySize& operator=(BodySize&& other) noexcept;
    ~BodySize();

    // Class specific features
  public:
    // Constants
    static const std::string ALLOWED_LETTERS;
    // Accessors
    size_t value() const;
    // Operators
    operator size_t&();
    operator const size_t&() const;
    bool operator==(const size_t& other) const;
    bool operator!=(const size_t& other) const;

  private:
    // Properties
    size_t m_value = 0;
    // Methods
    static size_t parseValue(const std::string& s);
    static bool allDigits(const std::string& s, std::size_t end);
    static bool isAllowedLetter(char c);
    static size_t applyLetterMultiplier(size_t value, char letter);
};

#endif
