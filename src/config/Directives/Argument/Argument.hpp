#pragma once

#ifndef ARGUMENT_HPP
# define ARGUMENT_HPP

# include <utility>
# include <string>

enum class ArgumentType
{
    URL,
    Integer,
    StatusCode,
    DataSize,
    OnOff,
    FilePath,
    NetworkEndpoint,
    HTTPMethod
};

class Argument
{
    // Construction and destruction
  public:
    explicit Argument(const std::string& value);
    Argument(const std::string& value, size_t line, size_t column);
    Argument(const Argument& other);
    Argument& operator=(const Argument& other);
    Argument(Argument&& other) noexcept;
    Argument& operator=(Argument&& other) noexcept;
    ~Argument();

    // Class specific features
  public:
    // Accessors
    const std::string& value();
    size_t line();
    size_t column();
    // Methods
    static ArgumentType getArgumentType(const Argument& arg);

  private:
    // Properties
    std::string m_value;
    size_t m_line = static_cast<size_t>(-1);
    size_t m_column = static_cast<size_t>(-1);
};

#endif
