#include <iostream>
#include "Config.hpp"

int main(void)
{
    const char* filepath = "./webserv.conf";
    try
    {
        Config config = Config::fromFile(filepath);

        RequestContext context
            = config.createRequestContext("server.com", "/kapouet/file");

        std::cout << "Well done :)"
                  << "\n";
    }
    catch (const ConfigException& e)
    {
        std::cerr << "\033[1m";
        std::cerr << filepath << ":";
        std::cerr << "\033[0m";

        std::cerr << e.what() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
