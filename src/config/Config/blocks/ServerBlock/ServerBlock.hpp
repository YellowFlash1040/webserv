#pragma once

#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include <utility>
# include <string>
# include <vector>

# include "ConfigBlock.hpp"
# include "LocationBlock.hpp"

# include "Property.hpp"

# include "ErrorPage.hpp"
# include "HttpRedirection.hpp"
# include "RequestContext.hpp"
# include "NetworkEndpoint.hpp"

# include "EffectiveConfig.hpp"
# include "DirectiveAppliers.hpp"

struct ServerBlock : public ConfigBlock
{
    // Properties
    Property<std::vector<LocationBlock>> locations;
    Property<std::vector<NetworkEndpoint>> listen;
    Property<std::vector<std::string>> serverName;
    Property<std::string> root;
    Property<std::string> alias;
    Property<std::vector<ErrorPage>> errorPages;
    Property<size_t> clientMaxBodySize{};
    Property<HttpRedirection> httpRedirection;
    Property<bool> autoindex{};
    Property<std::vector<std::string>> index;
    Property<std::string> uploadStore;
    // Methods
    void applyTo(EffectiveConfig& context) const override;
};

#endif
