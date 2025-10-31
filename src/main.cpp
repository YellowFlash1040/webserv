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

    const char* filepath;
    if (argc > 1)
        filepath = argv[1];
    else
        filepath = "webserv.conf";

    try
    {
        Config config = Config::fromFile(filepath);

        std::vector<std::string> endpoints = config.getAllEnpoints();

        Server s;

        for (std::vector<std::string>::const_iterator it = endpoints.begin();
             it != endpoints.end(); ++it)
        {
            int port = std::atoi(it->c_str());
            s.addEndpoint({std::string("127.0.0.1"), port});
        }
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

// RequestContext context
//     = config.createRequestContext("server.com", "/kapouet/file");
// RequestContext context
// = config.createRequestContext("server.com", "/images/file");
// RequestContext context = config.createRequestContext("", "");
