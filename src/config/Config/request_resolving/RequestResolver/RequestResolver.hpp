#pragma once

#ifndef REQUESTRESOLVER_HPP
# define REQUESTRESOLVER_HPP

# include <utility>
# include <string>
# include <map>

# include "RequestContext.hpp"
# include "EffectiveConfig.hpp"
# include "NetworkEndpoint.hpp"
# include "ErrorPage.hpp"
# include "HttpBlock.hpp"

class RequestResolver
{
    // Construction and destruction
  public:
    RequestResolver() = delete;
    RequestResolver(const RequestResolver& other) = delete;
    RequestResolver& operator=(const RequestResolver& other) = delete;
    RequestResolver(RequestResolver&& other) noexcept = delete;
    RequestResolver& operator=(RequestResolver&& other) noexcept = delete;
    ~RequestResolver() = delete;

    // Class specific features
  public:
    // Methods
    static RequestContext resolve(const HttpBlock& httpBlock,
                                  const NetworkEndpoint& endpoint,
                                  const std::string& host,
                                  const std::string& uri);

  private:
    // Methods
    static EffectiveConfig createEffectiveConfig(
        const HttpBlock& httpBlock, const NetworkEndpoint& endpoint,
        const std::string& host, const std::string& uri);
    static RequestContext createContext(const EffectiveConfig& config,
                                        const std::string& uri);
    static const ServerBlock& matchServerBlock(
        const std::vector<ServerBlock>& servers,
        const NetworkEndpoint& endpoint, const std::string& host);
    static std::vector<const ServerBlock*> tryMatchByEndpoint(
        const std::vector<ServerBlock>& servers,
        const NetworkEndpoint& endpoint);
    static const LocationBlock* matchLocationBlock(const ServerBlock& server,
                                                   const std::string& uri);
    static std::map<HttpStatusCode, std::string> constructErrorPages(
        const std::vector<ErrorPage>& errorPages);
    static std::string resolvePath(const EffectiveConfig& config,
                                   const std::string& uri);
    static std::string handleAlias(const std::string& alias,
                                   const std::string& matched_location,
                                   const std::string& uri);
    static std::string handleRoot(const std::string& root,
                                  const std::string& uri);
};

#endif
