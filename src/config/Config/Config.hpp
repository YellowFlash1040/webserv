#pragma once

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <utility>
# include <vector>
# include <memory>

# include "Directives.hpp"
# include "Arguments.hpp"

# include "FileReader.hpp"
# include "Lexer.hpp"
# include "Parser.hpp"
# include "Validator.hpp"

# include "HttpBlock.hpp"
# include "ServerBlock.hpp"
# include "LocationBlock.hpp"

class Config
{
    // Construction and destruction
  public:
    explicit Config(std::unique_ptr<ADirective> rootNode);
    Config(const Config& other) = delete;
    Config& operator=(const Config& other) = delete;
    Config(Config&& other) noexcept;
    Config& operator=(Config&& other) noexcept;
    ~Config();

    // Class specific features
  public:
    // Accessors
    // Methods

    static Config fromFile(const std::string& filepath);
    std::vector<std::string> getAllEnpoints();

    HttpBlock& httpBlock();
    RequestContext createRequestContext(const std::string& host,
                                        const std::string& url);

  private:
    // Properties
    HttpBlock m_httpBlock;

    // Methods
    std::vector<ADirective*> findAll(const std::string& directiveName,
                                     BlockDirective* block);
    void findAll(const std::string& directiveName, BlockDirective* block,
                 std::vector<ADirective*>& result);
    HttpBlock buildHttpBlock(const std::unique_ptr<ADirective>& httpNode);
    ServerBlock buildServerBlock(const std::unique_ptr<ADirective>& serverNode);
    LocationBlock buildLocationBlock(
        const std::unique_ptr<ADirective>& locationNode);

    static void assign(std::string& property,
                       const std::vector<std::string>& args);
    static void assign(bool& property, const std::vector<std::string>& args);
    static void assign(size_t& property, const std::vector<std::string>& args);
    static void assign(std::vector<ErrorPage>& property,
                       const std::vector<std::string>& args);
    static void assign(std::vector<std::string>& property,
                       const std::vector<std::string>& args);
}

#endif
