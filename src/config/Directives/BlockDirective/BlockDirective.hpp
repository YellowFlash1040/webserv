#pragma once

#ifndef BLOCKDIRECTIVE_HPP
# define BLOCKDIRECTIVE_HPP

# include <utility>

# include "ADirective.hpp"

class BlockDirective : public ADirective
{
    // Construction and destruction
  public:
    BlockDirective();
    BlockDirective(const BlockDirective& other);
    BlockDirective& operator=(const BlockDirective& other);
    BlockDirective(BlockDirective&& other) noexcept;
    BlockDirective& operator=(BlockDirective&& other) noexcept;
    ~BlockDirective();

    // Class specific features
  public:
    // Constants
    // Accessors
    // Methods
    void addDirective(ADirective directive);

  protected:
    // Properties
    // Methods

  private:
    // Properties
    std::vector<ADirective> m_directives;
    // Methods
};

#endif
