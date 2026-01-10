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
        throw std::runtime_error("CGI script is not executable by this user: "
                                 + scriptPath);

    return scriptPath;
}

CGIData CGIManager::startCGI(const RequestData& req, Client& client,
                             const std::string& interpreter,
                             const std::string& scriptPath)
{

    DBG("[CGIManager] scriptPath = " << scriptPath
                                     << ", interpreter = " << interpreter);

    std::string execPath = validateScriptPath(scriptPath);

    int pipe_in[2], pipe_out[2];

    if (pipe(pipe_out) == -1)
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

        std::vector<std::string> envVec
            = buildEnvFromRequest(req, client, scriptPath);
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

        if (epoll_ctl(client.getEpollFd(), EPOLL_CTL_ADD, pipe_out[0], &ev_out)
            == -1)
            perror("epoll_ctl ADD pipe_out");

        if (req.method == HttpMethod::POST)
        {
            epoll_event ev_in;
            ev_in.data.fd = pipe_in[1];
            ev_in.events = EPOLLOUT;
            if (epoll_ctl(client.getEpollFd(), EPOLL_CTL_ADD, pipe_in[1],
                          &ev_in)
                == -1)
                perror("epoll_ctl ADD pipe_in");
        }
        else
        {
            close(pipe_in[1]);
            pipe_in[1] = -1;
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

std::vector<std::string> CGIManager::buildEnvFromRequest(
    const RequestData& req, Client& client, const std::string& scriptPath)
{
    std::vector<std::string> env;

    addEnv(env, "GATEWAY_INTERFACE", "CGI/1.1");
    addRequestInfo(env, client, req, scriptPath);
    addServerInfo(env, req, client);

    return env;
}

void CGIManager::addEnv(std::vector<std::string>& env, const std::string& key,
                        const std::string& value)
{
    env.push_back(key + "=" + value);
}

void CGIManager::addRequestInfo(std::vector<std::string>& env,
                                const Client& client, const RequestData& req,
                                const std::string& scriptPath)
{
    addEnv(env, "SCRIPT_NAME", req.uri);
    addEnv(env, "SCRIPT_FILENAME", scriptPath);

    addEnv(env, "REQUEST_METHOD", httpMethodToString(req.method));
    addEnv(env, "QUERY_STRING", req.query);
    addEnv(env, "CONTENT_LENGTH", std::to_string(req.body.size()));

    const std::string& contentType = req.getHeader("content-type");
    if (!contentType.empty())
        addEnv(env, "CONTENT_TYPE", contentType);

    addRemoteAddr(env, client);
}

void CGIManager::addReqHeaders(std::vector<std::string>& env,
                               const RequestData& req)
{
    for (auto& h : req.headers)
    {
        std::string key = "HTTP_" + h.first;
        for (auto& c : key)
        {
            if (c == '-')
                c = '_';
            else
                c = std::toupper(c);
        }
        const std::string& value = h.second;
        addEnv(env, key, value);
    }
}

void CGIManager::addServerInfo(std::vector<std::string>& env,
                               const RequestData& req, const Client& client)
{
    addServerName(env, req, client);
    addServerAddr(env, client);
}

void CGIManager::addServerName(std::vector<std::string>& env,
                               const RequestData& req, const Client& client)
{
    const std::string& hostName = req.getHeader("Host");
    const std::string& hostIp = client.getListeningEndpoint().ip();

    const std::string& serverName = hostName.empty() ? hostIp : hostName;

    addEnv(env, "SERVER_NAME", serverName);
}

void CGIManager::addServerAddr(std::vector<std::string>& env,
                               const Client& client)
{
    const NetworkEndpoint& endpoint = client.getListeningEndpoint();

    const std::string& ip = endpoint.ip();
    const std::string& port = std::to_string(endpoint.port());

    addEnv(env, "SERVER_ADDR", ip);
    addEnv(env, "SERVER_PORT", port);
}

void CGIManager::addRemoteAddr(std::vector<std::string>& env,
                               const Client& client)
{
    const sockaddr_in& addr = client.getAddress();

    const std::string& ip = NetworkInterface(ntohl(addr.sin_addr.s_addr));
    const std::string& port = std::to_string(ntohs(addr.sin_port));

    addEnv(env, "REMOTE_ADDR", ip);
    addEnv(env, "REMOTE_PORT", port);
}
