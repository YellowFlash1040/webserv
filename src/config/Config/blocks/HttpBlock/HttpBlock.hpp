#pragma once

#ifndef HTTPBLOCK_HPP
# define HTTPBLOCK_HPP

# include <vector>
# include <string>

# include "ConfigBlock.hpp"
# include "ServerBlock.hpp"
# include "Property.hpp"
# include "ErrorPage.hpp"
# include "EffectiveConfig.hpp"
# include "DirectiveAppliers.hpp"

struct HttpBlock : public ConfigBlock
{
    // Accessors
    Property<std::vector<ServerBlock>> servers;
    Property<std::vector<ErrorPage>> errorPages;
    Property<size_t> clientMaxBodySize{};
    Property<std::string> root;
    Property<bool> autoindex{};
    Property<std::vector<std::string>> index;
    // Methods
    void applyTo(EffectiveConfig& config) const override;
    const ServerBlock& matchServerBlock(const std::string& host) const;
    const ServerBlock& matchServerBlock(const std::string& listen,
                                        const std::string& host) const;
};

#endif
