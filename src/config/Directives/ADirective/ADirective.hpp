#pragma once

#ifndef ADIRECTIVE_HPP
# define ADIRECTIVE_HPP

# include <utility>
# include <string>
# include <vector>

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
    // Constants
    // Accessors
    std::string name() const;
    void setName(const std::string& name);
    // Methods
    void addArgument(const std::string& arg);

  protected:
    // Properties
    std::string m_name;
    std::vector<std::string> m_args;
    // Methods

  private:
    // Properties
    // Methods
};

#endif
