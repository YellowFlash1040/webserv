#pragma once

#ifndef BLOCKDIRECTIVE_HPP
# define BLOCKDIRECTIVE_HPP

# include <utility>
# include <memory>

# include "Directive.hpp"

class BlockDirective : public Directive
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
    const std::vector<std::unique_ptr<Directive>>& directives() const;
    // Methods
    void addDirective(std::unique_ptr<Directive>&& directive);

  private:
    // Properties
    std::vector<std::unique_ptr<Directive>> m_directives;
};

#endif
