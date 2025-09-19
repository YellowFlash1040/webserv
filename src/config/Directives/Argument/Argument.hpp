#pragma once

#ifndef ARGUMENT_HPP
# define ARGUMENT_HPP

# include <utility>
# include <string>

class Argument
{
    // Construction and destruction
  public:
    Argument();
    Argument(const Argument& other);
    Argument& operator=(const Argument& other);
    Argument(Argument&& other) noexcept;
    Argument& operator=(Argument&& other) noexcept;
    ~Argument();

    // Class specific features
  public:
    // Constants
    // Accessors
    // Methods

  protected:
    // Properties
    // Methods

  private:
    // Properties
    size_t m_line;
    size_t m_column;
    std::string m_value;
    // Methods
};

#endif
