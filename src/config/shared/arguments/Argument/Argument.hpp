#pragma once

#ifndef ARGUMENT_HPP
# define ARGUMENT_HPP

# include <utility>
# include <string>

enum class ArgumentType
{
    URL,             // https://profile.intra.42.fr
    Integer,         // 8080
    StatusCode,      // 200, 301, 404, 500
    DataSize,        // 20M
    OnOff,           // 'on' or 'off'
    FilePath,        // /var/www/images
    NetworkEndpoint, // IP + port
    HttpMethod,      // GET, POST, DELETE
    String,          // 'hello'
    URI,
    Port
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
    const std::string& value() const;
    ArgumentType type() const;
    size_t line() const;
    size_t column() const;
    // Methods
    static ArgumentType getArgumentType(const Argument& arg);
    // Operators
    operator std::string&();
    operator const std::string&() const;
    bool operator==(const std::string& other) const;
    bool operator!=(const std::string& other) const;

  private:
    // Properties
    std::string m_value;
    ArgumentType m_type = ArgumentType::String;
    size_t m_line = static_cast<size_t>(-1);
    size_t m_column = static_cast<size_t>(-1);
};

#endif
