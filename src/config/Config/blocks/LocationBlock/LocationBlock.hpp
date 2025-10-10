#pragma once

#ifndef LOCATIONBLOCK_HPP
# define LOCATIONBLOCK_HPP

# include <utility>
# include <string>
# include <vector>

# include "ConfigBlock.hpp"
# include "HttpMethod.hpp"
# include "HttpRedirection.hpp"
# include "ErrorPage.hpp"
# include "RequestContext.hpp"

# include "Property.hpp"

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
    Property<std::string> cgiPass;
    // Methods
    void applyTo(RequestContext& ctx) const override;
};

#endif
