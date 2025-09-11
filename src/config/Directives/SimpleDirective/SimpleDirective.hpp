#pragma once

#ifndef SIMPLEDIRECTIVE_HPP
# define SIMPLEDIRECTIVE_HPP

# include <utility>

# include "ADirective.hpp"

class SimpleDirective : public ADirective
{
    // Construction and destruction
  public:
    SimpleDirective();
    SimpleDirective(const SimpleDirective& other);
    SimpleDirective& operator=(const SimpleDirective& other);
    SimpleDirective(SimpleDirective&& other) noexcept;
    SimpleDirective& operator=(SimpleDirective&& other) noexcept;
    ~SimpleDirective();

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
    // Methods
};

#endif
