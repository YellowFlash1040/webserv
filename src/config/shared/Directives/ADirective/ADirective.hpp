#pragma once

#ifndef ADIRECTIVE_HPP
# define ADIRECTIVE_HPP

# include <utility>
# include <string>
# include <vector>

# include "Argument.hpp"

class ADirective
{
    // ----------------------------
    // Construction and destruction
    // ----------------------------
  public:
    ADirective(const ADirective& other);
    ADirective& operator=(const ADirective& other);
    ADirective(ADirective&& other) noexcept;
    ADirective& operator=(ADirective&& other) noexcept;
    virtual ~ADirective();

  protected:
    ADirective();

    // -----------------------
    // Class specific features
    // -----------------------
  public:
    // Accessors
    const std::string& name() const;
    void setName(std::string&& name);
    const std::vector<Argument>& args();
    void setArgs(std::vector<Argument>&& args);
    void setPosition(size_t line, size_t column);
    size_t line();
    size_t column();

  protected:
    // Properties
    std::string m_name;
    std::vector<Argument> m_args;
    size_t m_line = static_cast<size_t>(-1);
    size_t m_column = static_cast<size_t>(-1);
};

#endif
