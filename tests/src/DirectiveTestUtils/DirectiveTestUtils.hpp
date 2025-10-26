#pragma once

#ifndef DIRECTIVETESTUTILS_HPP
# define DIRECTIVETESTUTILS_HPP

# include <memory>

# include "Directive.hpp"
# include "BlockDirective.hpp"

std::unique_ptr<Directive> createSimpleDirective(
    const std::string& name, const std::vector<std::string>& args = {});

std::unique_ptr<BlockDirective> createBlockDirective(
    const std::string& name, const std::vector<std::string>& args = {});

#endif
