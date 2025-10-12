// #pragma once

#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <utility>
#include <memory>
#include <vector>
#include <functional>

#include "ConfigExceptions.hpp"
#include "Directives.hpp"
#include "Converter.hpp"

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

  private:
    // Properties
    const std::unique_ptr<ADirective>& m_rootNode;
    size_t m_errorLine = static_cast<size_t>(-1);
    size_t m_errorColumn = static_cast<size_t>(-1);
    // Methods
    void validateNode(const std::unique_ptr<ADirective>& node,
                      const std::string& parentContext);
    void validateChildren(const BlockDirective& block,
                          const std::string& parentContext);
    void checkIfAllowedDirective(const std::string& name,
                                 const std::string& context);
    void validateArguments(const std::string& name,
                           const std::vector<Argument>& args);
    void validateArgumentGroup(const std::string& name,
                               const std::vector<Argument>& args,
                               const Directives::ArgumentSpec& spec, size_t& i,
                               size_t& lastThrowIndex);
    static void validateArgument(const std::vector<ArgumentType>& possibleTypes,
                                 const std::string& value);
    void checkArgumentCount(const std::string& name,
                            const Directives::ArgumentSpec& spec,
                            std::size_t count) const;

    static void validateUrl(const std::string& s);
    static void validateInteger(const std::string& s);
    static void validateStatusCode(const std::string& s);
    static void validateDataSize(const std::string& s);
    static void validateOnOff(const std::string& s);
    static void validateFilePath(const std::string& s);
    static void validateNetworkEndpoint(const std::string& s);
    static void validateHttpMethod(const std::string& s);
    static void validateString(const std::string& s);
    static void validateUri(const std::string& s);
    // Accessors
    static const std::map<ArgumentType,
                          std::function<void(const std::string&)>>&
    validators();
};

#endif
