#include <iostream>
#include <string.h>
#include "Server.hpp"
#include "Config.hpp"

bool validateArgumentsCount(int argc, char** argv);
bool initializeConfig(Config& config, const char* filepath);
void setupSignalHandlers();

volatile std::sig_atomic_t g_running = true;

void stopServer(int)
{
    g_running = false;
}

int main(int argc, char** argv)
{
    if (!validateArgumentsCount(argc, argv))
        return EXIT_FAILURE;

    const char* filepath = (argc == 2) ? argv[1] : "webserv.conf";

    Config config;
    if (!initializeConfig(config, filepath))
        return EXIT_FAILURE;

    setupSignalHandlers();
    while (true)
    {
        try
        {
            Server s(config);
            s.run();
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << "Error: " << e.what() << ": " << strerror(errno)
                      << "\n";
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }

        if (!g_running)
            break;
    }
    return EXIT_SUCCESS;
}

bool validateArgumentsCount(int argc, char** argv)
{
    if (argc != 1 && argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " [configuration file]\n";
        return false;
    }
    return true;
}

bool initializeConfig(Config& config, const char* filepath)
{
    try
    {
        config = Config::fromFile(filepath);
    }
    catch (const ConfigException& e)
    {
        std::cerr << "\033[1m" << filepath << ":\033[0m" << e.what() << '\n';
        return false;
    }
    return true;
}

void setupSignalHandlers()
{
    std::signal(SIGINT, stopServer);
    std::signal(SIGTERM, stopServer);
    std::signal(SIGPIPE, SIG_IGN);
}
