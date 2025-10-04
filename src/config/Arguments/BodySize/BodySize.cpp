#include "BodySize.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

const std::string BodySize::ALLOWED_LETTERS = "kKmMgG";

// Default constructor
BodySize::BodySize() {}

BodySize::BodySize(const std::string& value)
{
    m_value = parseValue(value);
}

// Copy constructor
BodySize::BodySize(const BodySize& other)
  : m_value(other.m_value)
{
}

// Copy assignment operator
BodySize& BodySize::operator=(const BodySize& other)
{
    if (this != &other)
    {
        m_value = other.m_value;
    }
    return (*this);
}

// Move constructor
BodySize::BodySize(BodySize&& other) noexcept
  : m_value(other.m_value)
{
}

// Move assignment operator
BodySize& BodySize::operator=(BodySize&& other) noexcept
{
    if (this != &other)
    {
        m_value = other.m_value;
    }
    return (*this);
}

// Destructor
BodySize::~BodySize() {}

// ---------------------------ACCESSORS-----------------------------

size_t BodySize::value() const
{
    return (m_value);
}

// ---------------------------METHODS-----------------------------

size_t BodySize::parseValue(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument(s);

    std::size_t len = s.size();
    char last = s.back();
    bool hasLetter = false;

    // Check if last character is a letter
    if (std::isalpha(static_cast<unsigned char>(last)))
    {
        if (!isAllowedLetter(last))
            throw std::invalid_argument(s);
        hasLetter = true;
    }

    // Check all digits in the numeric part
    std::size_t numLength = hasLetter ? len - 1 : len;
    if (!allDigits(s, numLength))
        throw std::invalid_argument(s);

    size_t number = std::stoul(s.substr(0, numLength));

    if (hasLetter)
    {
        char letter = last;
        number = applyLetterMultiplier(number, letter);
    }

    return (number);
}

bool BodySize::allDigits(const std::string& s, std::size_t end)
{
    return std::all_of(s.begin(), s.begin() + end, ::isdigit);
}

bool BodySize::isAllowedLetter(char c)
{
    return std::any_of(std::begin(ALLOWED_LETTERS), std::end(ALLOWED_LETTERS),
                       [c](char l) { return l == c; });
}

// Returns 10^power depending on the letter's position
size_t BodySize::applyLetterMultiplier(size_t value, char letter)
{
    // Find the index of the letter in ALLOWED_LETTERS
    auto it = std::find(std::begin(ALLOWED_LETTERS), std::end(ALLOWED_LETTERS),
                        letter);

    std::size_t index = it - std::begin(ALLOWED_LETTERS);

    // Every 2 letters (lower + upper) increases the multiplier by 1024
    std::size_t power = index / 2;

    size_t multiplier = 1;
    for (std::size_t i = 0; i <= power; ++i)
        multiplier *= 1024;

    return (value * multiplier);
}
