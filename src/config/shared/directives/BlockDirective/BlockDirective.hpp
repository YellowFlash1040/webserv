#pragma once

#ifndef BLOCKDIRECTIVE_HPP
# define BLOCKDIRECTIVE_HPP

# include <utility>
# include <memory>

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
    // Accessors
    const std::vector<std::unique_ptr<ADirective>>& directives() const;
    // Methods
    void addDirective(std::unique_ptr<ADirective>&& directive);
    void setDirectives(std::vector<std::unique_ptr<ADirective>>&& directives);

  private:
    // Properties
    std::vector<std::unique_ptr<ADirective>> m_directives;
};

#endif
