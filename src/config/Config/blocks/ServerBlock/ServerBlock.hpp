#pragma once

#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include <utility>
# include <string>
# include <vector>

# include "ConfigBlock.hpp"
# include "ErrorPage.hpp"
# include "LocationBlock.hpp"
# include "HttpRedirection.hpp"
# include "RequestContext.hpp"
// # include "NetworkEndpoint.hpp"

# include "Property.hpp"

struct ServerBlock : public ConfigBlock
{
    // Properties
    Property<std::vector<LocationBlock>> locations;
    Property<std::string> listen;
    Property<std::string> serverName;
    Property<std::string> root;
    Property<std::string> alias;
    Property<std::vector<ErrorPage>> errorPages;
    Property<size_t> clientMaxBodySize{};
    Property<HttpRedirection> httpRedirection;
    Property<bool> autoindex{};
    Property<std::vector<std::string>> index;
    // Methods
    void applyTo(RequestContext& context) const override;
    const LocationBlock* matchLocationBlock(const std::string& uri) const;
};

#endif
