#include <iostream>
#include "Config.hpp"

int main(int argc, char** argv)
{
    // const char* filepath = "./webserv.conf";
    // const char* filepath = "./noneexistent.conf";
    // const char* filepath = "./assets/nopermissions.conf";
    // const char* filepath = "./assets/empty.conf";
    // const char* filepath = "./assets/invalid.conf";

    const char* filepath;
    if (argc > 1)
        filepath = argv[1];
    else
        filepath = "webserv.conf";

    try
    {
        Config config = Config::fromFile(filepath);

        RequestContext context = config.createRequestContext(
            NetworkEndpoint(8080), "server.com", "/kapouet/file");

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
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}
