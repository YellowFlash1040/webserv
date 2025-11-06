#pragma once

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <utility>
# include <vector>
# include <memory>

# include "ConfigException.hpp"
# include "Directives.hpp"
# include "Arguments.hpp"

# include "FileReader.hpp"
# include "Lexer.hpp"
# include "Parser.hpp"
# include "Validator.hpp"
# include "Converter.hpp"

# include "HttpBlock.hpp"
# include "ServerBlock.hpp"
# include "LocationBlock.hpp"

class Config
{
    // Construction and destruction
  public:
    explicit Config(std::unique_ptr<Directive> rootNode);
    Config(const Config& other) = delete;
    Config& operator=(const Config& other) = delete;
    Config(Config&& other) noexcept;
    Config& operator=(Config&& other) noexcept;
    ~Config();

    // Class specific features
  public:
    // Methods
    static Config fromFile(const std::string& filepath);
    std::vector<NetworkEndpoint> getAllEnpoints();
    RequestContext createRequestContext(const std::string& host,
                                        const std::string& uri);

  private:
    // Properties
    HttpBlock m_httpBlock;

    // Methods
    static HttpBlock buildHttpBlock(const std::unique_ptr<Directive>& httpNode);
    static ServerBlock buildServerBlock(
        const std::unique_ptr<Directive>& serverNode);
    static LocationBlock buildLocationBlock(
        const std::unique_ptr<Directive>& locationNode);

    static void assign(Property<std::string>& property,
                       const std::vector<Argument>& args);
    static void assign(Property<bool>& property,
                       const std::vector<Argument>& args);
    static void assign(Property<size_t>& property,
                       const std::vector<Argument>& args);
    static void assign(Property<std::vector<ErrorPage>>& property,
                       const std::vector<Argument>& args);
    static void assign(Property<std::vector<std::string>>& property,
                       const std::vector<Argument>& args);
    static void assign(Property<std::vector<HttpMethod>>& property,
                       const std::vector<Argument>& args);
    static void assign(Property<HttpRedirection>& property,
                       const std::vector<Argument>& args);
    static void assign(Property<std::map<std::string, std::string>>& cgiPass,
                       const std::vector<Argument>& args);
    static void assign(Property<std::vector<NetworkEndpoint>>& property,
                       const std::vector<Argument>& args);

    static void setDefaultHttpMethods(std::vector<HttpMethod>& httpMethods);

    EffectiveConfig createEffectiveConfig(const std::string& host,
                                          const std::string& uri);
    static RequestContext createContext(const EffectiveConfig& config,
                                        const std::string& uri);
    static std::map<HttpStatusCode, std::string> constructErrorPages(
        const std::vector<ErrorPage>& errorPages);
    static std::string resolvePath(const EffectiveConfig& config,
                                   const std::string& uri);
};

#endif
