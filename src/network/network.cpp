#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define SUCCESS 0
#define ERROR -1

#define MAX_EVENTS 50

typedef struct sockaddr t_sockaddr;
typedef struct sockaddr_in t_sockaddr_in;
typedef struct epoll_event t_event;

int init(int port);
void fillServerAddressInfo(t_sockaddr_in& addr, int port);
int run(int serverSocket);
int registerServerSocket(int epfd, int serverSocket);
int acceptNewClient(int epfd, int serverSocket);
void processClient(int clientSocket);
int setNonBlockingAndCloexec(int fd);

int try_(int result, char* msg)
{
    if (result == -1)
        perror(msg);
    return (result);
}

int init(int port)
{
    t_sockaddr_in serverAddress;

    fillServerAddressInfo(serverAddress, port);
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1)
        return (perror("socket"), ERROR);
    if (setNonBlockingAndCloexec(serverSocket) == -1)
        return (close(serverSocket), ERROR);
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))
        < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    int result = bind(serverSocket, (t_sockaddr*)(&serverAddress),
                      sizeof(serverAddress));
    if (result == -1)
        return (close(serverSocket), perror("bind"), ERROR);
    result = listen(serverSocket, 100);
    if (result == -1)
        return (close(serverSocket), perror("listen"), ERROR);
    return (serverSocket);
}

void fillServerAddressInfo(t_sockaddr_in& addr, int port)
{
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

int run(int serverSocket)
{
    int epfd = epoll_create(MAX_EVENTS);
    if (epfd == -1)
        return (perror("epoll_create"), -1);

    int result = registerServerSocket(epfd, serverSocket);
    if (result == -1)
        return (-1);

    t_event FDs[MAX_EVENTS];
    while (true)
    {
        int readyFDs = epoll_wait(epfd, FDs, MAX_EVENTS, -1);
        if (readyFDs == -1)
        {
            if (errno == EINTR)
                continue; // interrupted by signal, retry
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < readyFDs; ++i)
        {
            if (FDs[i].data.fd == serverSocket)
                result = acceptNewClient(epfd, serverSocket);
            else
                processClient(FDs[i].data.fd);
        }
    }

    close(epfd);
    close(serverSocket);
    return (SUCCESS);
}

int registerServerSocket(int epfd, int serverSocket)
{
    t_event e;
    e.data.fd = serverSocket;
    e.events = EPOLLIN;

    int result = epoll_ctl(epfd, EPOLL_CTL_ADD, serverSocket, &e);
    if (result == -1)
        return (perror("epoll_ctl: server_fd"), -1);
    return (SUCCESS);
}

int acceptNewClient(int epfd, int serverSocket)
{
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == -1)
        return (-1);

    int result = setNonBlockingAndCloexec(clientSocket);
    if (result == -1)
        return (result);

    t_event e;
    e.events = EPOLLIN;
    e.data.fd = clientSocket;
    result = epoll_ctl(epfd, EPOLL_CTL_ADD, clientSocket, &e);
    return (result);
}

void processClient(int clientSocket)
{
    // Data ready to read from client
    char buf[512];
    int n = read(clientSocket, buf, sizeof(buf) - 1);
    if (n <= 0)
    {
        // Client disconnected
        close(clientSocket);
        printf("Client disconnected: %d\n", clientSocket);
    }
    else
    {
        buf[n] = '\0';
        printf("Received: %s\n", buf);
    }
}

// Set O_NONBLOCK and FD_CLOEXEC on a socket
int setNonBlockingAndCloexec(int fd)
{
    // Non-blocking
    int status_flags = fcntl(fd, F_GETFL, 0);
    if (status_flags == -1)
        return (perror("fcntl getfl"), -1);

    int result = fcntl(fd, F_SETFL, status_flags | O_NONBLOCK);
    if (result == -1)
        return (perror("fcntl setfl"), -1);

    // Close-on-exec
    int fd_flags = fcntl(fd, F_GETFD, 0);
    if (fd_flags == -1)
        return (perror("fcntl getfd"), -1);

    result = fcntl(fd, F_SETFD, fd_flags | FD_CLOEXEC);
    if (result == -1)
        return (perror("fcntl setfd"), -1);

    return (SUCCESS);
}

// epoll
// event poll
// In this context, poll literally means “checking” or “asking repeatedly”
// — it’s a system call in Linux/Unix that checks multiple file descriptors
// (like sockets, pipes, or files) to see if they’re ready for I/O
// (read or write).
