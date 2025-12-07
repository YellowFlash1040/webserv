// #pragma once

#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <utility>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "ConfigExceptions.hpp"
#include "Directives.hpp"
#include "Converter.hpp"
#include "HttpBlock.hpp"

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
    static void validate(const std::unique_ptr<Directive>& node);
    static void validate(const HttpBlock& httpBlock);

  private:
    // Methods
    static void validateDirective(const std::unique_ptr<Directive>& directive,
                                  const std::string& parentContext);
    static void validateChildren(const BlockDirective* block);
    static void makeDuplicateCheck(std::set<std::string>& seenDirectives,
                                   const std::unique_ptr<Directive>& directive);
    static void checkBasicPreconditions(
        const std::unique_ptr<Directive>& directive,
        const std::vector<Directives::ArgumentSpec>& argSpecs);
    static void checkIfArgumentsAreAllowed(
        const std::unique_ptr<Directive>& directive,
        const std::vector<Directives::ArgumentSpec>& argSpecs);
    static void checkIfEnoughArguments(
        const std::unique_ptr<Directive>& directive,
        const std::vector<Directives::ArgumentSpec>& argSpecs);
    static void checkForDuplicateArguments(const std::vector<Argument>& args);
    static void makeConflictsCheck(std::set<std::string>& seenDirectives,
                                   const std::unique_ptr<Directive>& directive);
    static void checkForDuplicateLocationPaths(
        const BlockDirective* serverBlock);
    static void checkForDuplicateListen(const BlockDirective* serverBlock);
    static void expectRequiredDirective(const std::string& requiredDirective,
                                        const BlockDirective* context);
    static void checkIfAllowedDirective(
        const std::unique_ptr<Directive>& directive,
        const std::string& context);
    static void processArgumentsWithSpec(const Directives::ArgumentSpec& spec,
                                         const std::vector<Argument>& args,
                                         size_t& i);
    static void validateArguments(const std::unique_ptr<Directive>& directive);
    static bool validateArgument(const std::vector<ArgumentType>& possibleTypes,
                                 const std::string& value);

    static void validateInteger(const std::string& s);
    static void validateString(const std::string& s);
    static void validateUrl(const std::string& s);
    static void validateStatusCode(const std::string& s);
    static void validateReturnStatusCode(const std::string& s);
    static void validateDataSize(const std::string& s);
    static void validateOnOff(const std::string& s);
    static void validateFolderPath(const std::string& s);
    static void validateFilePath(const std::string& s);
    static void validateNetworkEndpoint(const std::string& s);
    static void validateIp(const std::string& s);
    static void validatePort(const std::string& s);
    static void validateHttpMethod(const std::string& s);
    static void validateUri(const std::string& s);
    static void validateName(const std::string& s);
    static void validateFile(const std::string& s);
    static void validateFileExtension(const std::string& s);
    static void validateBinaryPath(const std::string& s);
    // Accessors
    static const std::map<ArgumentType,
                          std::function<void(const std::string&)>>&
    validators();
};

#endif
