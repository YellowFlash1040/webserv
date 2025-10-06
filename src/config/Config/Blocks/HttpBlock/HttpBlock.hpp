#pragma once

#ifndef HTTPBLOCK_HPP
# define HTTPBLOCK_HPP

# include <vector>

# include "ConfigBlock.hpp"
# include "ServerBlock.hpp"
# include "ErrorPage.hpp"
# include "RequestContext.hpp"

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
    void applyTo(RequestContext& context) const override;
    const ServerBlock& matchServerBlock(const std::string& host) const;
};

#endif
