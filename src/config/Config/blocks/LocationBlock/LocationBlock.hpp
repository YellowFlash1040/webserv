#pragma once

#ifndef LOCATIONBLOCK_HPP
# define LOCATIONBLOCK_HPP

# include <utility>
# include <string>
# include <vector>

# include "ConfigBlock.hpp"

# include "Property.hpp"

# include "HttpMethod.hpp"
# include "HttpRedirection.hpp"
# include "RequestContext.hpp"
# include "HttpStatusCode.hpp"

# include "EffectiveConfig.hpp"
# include "DirectiveAppliers.hpp"

struct LocationBlock : public ConfigBlock
{
    // Properties
    Property<std::string> path;
    Property<std::vector<ErrorPage>> errorPages;
    Property<size_t> clientMaxBodySize{};
    Property<std::vector<HttpMethod>> acceptedHttpMethods;
    Property<HttpRedirection> httpRedirection;
    Property<std::string> root;
    Property<std::string> alias;
    Property<bool> autoindex{};
    Property<std::vector<std::string>> index;
    Property<std::string> uploadStore;
    Property<std::map<std::string, std::string>> cgiPass;
    // Methods
    void applyTo(EffectiveConfig& ctx) const override;
};

#endif
