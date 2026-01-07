#include "CGIManager.hpp"
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/epoll.h>

std::string validateScriptPath(const std::string& scriptPath)
{
    struct stat st;
    if (stat(scriptPath.c_str(), &st) == -1)
        throw std::runtime_error("CGI script not found: " + scriptPath);

    if (S_ISDIR(st.st_mode))
        throw std::runtime_error("CGI script is a directory: " + scriptPath);

    if (!(st.st_mode & S_IXUSR))
        throw std::runtime_error("CGI script is not executable by this user: " + scriptPath);

    return scriptPath;
}

std::string methodToString(HttpMethod method)
{
    switch (method)
    {
    case HttpMethod::GET:
        return "GET";
    case HttpMethod::POST:
        return "POST";
    case HttpMethod::DELETE:
        return "DELETE";
    default:
        return "NONE";
    }
}

CGIManager::CGIData CGIManager::startCGI(const RequestData& req,
                                         Client& client,
                                         const std::string& interpreter,
                                         const std::string& scriptPath)
{

    DBG("[CGIManager] scriptPath = " << scriptPath
              << ", interpreter = " << interpreter);

    std::string execPath = validateScriptPath(scriptPath);
    
    int pipe_in[2], pipe_out[2];

    if ( pipe(pipe_out) == -1)
        throw std::runtime_error("Failed to create pipes");

    if (pipe(pipe_in) == -1)
        throw std::runtime_error("Failed to create pipes");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("fork failed for CGI");

    if (pid == 0)
    {
        if (dup2(pipe_in[0], STDIN_FILENO) == -1
            || dup2(pipe_out[1], STDOUT_FILENO) == -1
            || dup2(pipe_out[1], STDERR_FILENO) == -1)
        {
            perror("dup2 failed");
            _exit(1);
        }

        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);

        std::vector<char*> c_args;
        c_args.push_back(const_cast<char*>(interpreter.c_str()));
        c_args.push_back(const_cast<char*>(scriptPath.c_str()));
        c_args.push_back(nullptr);

        std::vector<std::string> envVec = buildEnvFromRequest(req, client, scriptPath);
        std::vector<char*> c_env;
        for (auto& e : envVec) 
            c_env.push_back(const_cast<char*>(e.c_str()));
        c_env.push_back(nullptr);

        execve(interpreter.c_str(), c_args.data(), c_env.data());
        perror("execve failed");
        _exit(1);
    }
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);

        int flags = fcntl(pipe_in[1], F_GETFL, 0);
        fcntl(pipe_in[1], F_SETFL, flags | O_NONBLOCK);

        flags = fcntl(pipe_out[0], F_GETFL, 0);
        fcntl(pipe_out[0], F_SETFL, flags | O_NONBLOCK);

        epoll_event ev_out;
        ev_out.data.fd = pipe_out[0];
        ev_out.events = EPOLLIN | EPOLLRDHUP;
        
        if (epoll_ctl(client.getEpollFd(), EPOLL_CTL_ADD, pipe_out[0], &ev_out) == -1)
            perror("epoll_ctl ADD pipe_out");

        if (methodToString(req.method) == "POST")
        {
            epoll_event ev_in;
            ev_in.data.fd = pipe_in[1];
            ev_in.events = EPOLLOUT;
            if (epoll_ctl(client.getEpollFd(), EPOLL_CTL_ADD, pipe_in[1], &ev_in) == -1)
                perror("epoll_ctl ADD pipe_in");
        }
        else
        {
            close(pipe_in[1]);
        }
        
        CGIData cgi;

        cgi.pid = pid;
        cgi.fd_stdin = pipe_in[1];
        cgi.fd_stdout = pipe_out[0];
        cgi.start_time = std::time(nullptr);
        cgi.input = req.body;
  
        return cgi;
    }
}

std::vector<std::string> CGIManager::buildEnvFromRequest(const RequestData& req, Client& client, const std::string& scriptPath)
{
    std::vector<std::string> env;

    env.push_back("REQUEST_METHOD=" + methodToString(req.method));
    env.push_back("QUERY_STRING=" + req.query);
    env.push_back("CONTENT_LENGTH=" + std::to_string(req.body.size()));

    std::string contentType = req.getHeader("content-type");
    if (!contentType.empty())
        env.push_back("CONTENT_TYPE=" + contentType);

    env.push_back("SCRIPT_NAME=" + req.uri);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("SERVER_PROTOCOL=" + req.httpVersion);
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");

    const NetworkEndpoint& endpoint = client.getListeningEndpoint();
    NetworkInterface localIp = endpoint.ip();
    int localPort = endpoint.port();

    env.push_back("SERVER_ADDR=" + static_cast<std::string>(localIp));
    env.push_back("SERVER_PORT=" + std::to_string(localPort));

    const sockaddr_in& clientAddr = client.getAddress();
    uint32_t ip = ntohl(clientAddr.sin_addr.s_addr);
    int remotePort = ntohs(clientAddr.sin_port);
    std::string remoteIp = std::to_string((ip >> 24) & 0xFF) + "." +
                           std::to_string((ip >> 16) & 0xFF) + "." +
                           std::to_string((ip >> 8) & 0xFF) + "." +
                           std::to_string(ip & 0xFF);
    env.push_back("REMOTE_ADDR=" + remoteIp);
    env.push_back("REMOTE_PORT=" + std::to_string(remotePort));

    std::string host = req.getHeader("Host");

    if (!host.empty())
        env.push_back("SERVER_NAME=" + host);
    else
        env.push_back("SERVER_NAME=" + static_cast<std::string>(localIp));

    for (auto& h : req.headers)
    {
        std::string name = "HTTP_" + h.first;
        for (auto& c : name)
        {
            if (c == '-')
                c = '_';
            else 
                c = std::toupper(c);
        }
        env.push_back(name + "=" + h.second);
    }

    return env;
}
