#include "network.hpp"

int main(void)
{
    int serverSocket = init(8080);
    run(serverSocket);
}
