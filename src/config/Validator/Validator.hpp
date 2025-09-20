#pragma once

#ifndef VALIDATOR_HPP
# define VALIDATOR_HPP

# include <utility>
# include <vector>
# include <memory>

# include "Directives.hpp"

# include "ConfigExceptions.hpp"

class Validator
{
    // Construction and destruction
  public:
    explicit Validator(const std::unique_ptr<ADirective>& rootNode);
    Validator(const Validator& other) = delete;
    Validator& operator=(const Validator& other) = delete;
    Validator(Validator&& other) noexcept = delete;
    Validator& operator=(Validator&& other) noexcept = delete;
    ~Validator();

    // Class specific features
  public:
    // Methods
    static void validate(const std::unique_ptr<ADirective>& node);
    void validate();
    void validateNode(const std::unique_ptr<ADirective>& node,
                      const std::string& parentContext);

  private:
    // Properties
    const std::unique_ptr<ADirective>& m_rootNode;
    size_t m_errorLine = static_cast<size_t>(-1);
    size_t m_errorColumn = static_cast<size_t>(-1);
    // Methods
    void validateChildren(const BlockDirective& block,
                          const std::string& parentContext);
    void checkParentConstraint(const std::string& name,
                               const std::string& parentContext);
    void checkAllowedDirective(const std::string& name,
                               const std::string& context);
    void checkArguments(const std::string& name,
                        const std::vector<Argument>& args);
};

#endif
