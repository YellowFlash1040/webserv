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
