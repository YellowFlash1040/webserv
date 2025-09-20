#pragma once

#ifndef VALIDATOR_HPP
# define VALIDATOR_HPP

# include <utility>
# include <vector>
# include <memory>

# include "Directives.hpp"

# include "DirectiveWrongParentException.hpp"
# include "DirectiveNotAllowedException.hpp"

class Validator
{
    // Construction and destruction
  public:
    Validator() = delete;
    Validator(const Validator& other) = delete;
    Validator& operator=(const Validator& other) = delete;
    Validator(Validator&& other) noexcept = delete;
    Validator& operator=(Validator&& other) noexcept = delete;
    ~Validator() = delete;

    // Class specific features
  public:
    // Methods
    static void validate(const std::unique_ptr<ADirective>& node);
    static void validateNode(const std::unique_ptr<ADirective>& node,
                             const std::string& parentContext);

  private:
    // Methods
    static void validateChildren(const BlockDirective& block,
                                 const std::string& parentContext);
    static void checkParentConstraint(const std::string& name,
                                      const std::string& parentContext);
    static void checkAllowedDirective(const std::string& name,
                                      const std::string& context);
    static void checkArguments(const std::string& name,
                               const std::vector<Argument>& args);
};

#endif
