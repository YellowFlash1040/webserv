#include "Server.hpp"
#include <iostream>
#include <string.h>
#include <csignal>

volatile std::sig_atomic_t g_running = true;

void handle_signal(int)
{
    g_running = false;
}

int main(void)
{
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    try
    {
        Server s(8081);
        s.run();
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error: " << e.what() << ": " << strerror(errno) << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
