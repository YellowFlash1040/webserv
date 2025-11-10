// Mine
#include "Server.hpp"
#include <iostream>
#include <string.h>
#include "Config.hpp"

volatile std::sig_atomic_t g_running = true;

void handle_signal(int)
{
    g_running = false;
}

int main(int argc, char** argv)
{
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    const char* filepath = (argc > 1) ? argv[1] : "webserv.conf";

    try
    {
        Config config = Config::fromFile(filepath);
        std::vector<NetworkEndpoint> endpoints = config.getAllEnpoints();

        Server s;

        for (const auto& endpoint : endpoints)
            s.addEndpoint(endpoint);

        s.run();
    }
    catch (const ConfigException& e)
    {
        std::cerr << "\033[1m" << filepath << ":\033[0m " << e.what() << '\n';
        return 1;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error: " << e.what() << ": " << strerror(errno) << "\n";
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
