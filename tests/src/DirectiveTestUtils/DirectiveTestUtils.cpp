#include "DirectiveTestUtils.hpp"

std::unique_ptr<Directive> createSimpleDirective(
    const std::string& name, const std::vector<std::string>& args)
{
    auto directive = std::make_unique<Directive>();
    directive->setName(std::string(name));

    std::vector<Argument> arguments(args.begin(), args.end());
    directive->setArgs(std::move(arguments));
    return directive;
}

std::unique_ptr<BlockDirective> createBlockDirective(
    const std::string& name, const std::vector<std::string>& args)
{
    auto directive = std::make_unique<BlockDirective>();
    directive->setName(std::string(name));

    std::vector<Argument> arguments(args.begin(), args.end());
    directive->setArgs(std::move(arguments));

    return directive;
}
