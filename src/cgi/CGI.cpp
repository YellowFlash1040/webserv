#include "CGI.hpp"
#include <sys/stat.h>
#include <limits.h>

static void validateScriptPath(const std::string &scriptPath, const std::string &rootDir)
{
    struct stat st;
    if (stat(scriptPath.c_str(), &st) == -1)
        throw std::runtime_error("CGI script not found: " + scriptPath);

    if (S_ISDIR(st.st_mode))
        throw std::runtime_error("CGI script is a directory: " + scriptPath);

    if (!(st.st_mode & S_IXUSR))
        throw std::runtime_error("CGI script is not executable: " + scriptPath);

    char realScript[PATH_MAX];
    char realRoot[PATH_MAX];
    if (!realpath(scriptPath.c_str(), realScript) || !realpath(rootDir.c_str(), realRoot))
        throw std::runtime_error("Failed to resolve real path");

    std::string realScriptStr(realScript);
    std::string realRootStr(realRoot);

    if (realScriptStr.find(realRootStr) != 0)
        throw std::runtime_error("Path traversal detected: " + scriptPath);
}

CGIResponse execute(const std::string &scriptPath, const std::vector<std::string> &args, const std::vector<std::string> &env, const std::string &input = "", const std::string &rootDir = "/var/www/cgi-bin/")
{
    validateScriptPath(scriptPath, rootDir);

    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
        throw std::runtime_error("Failed to create pipes");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("Fork failed");

    if (pid == 0) 
    {
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        dup2(pipe_out[1], STDERR_FILENO);

        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);

        std::vector<char*> c_args;
        
        c_args.push_back(const_cast<char*>(scriptPath.c_str()));
        for (auto &a : args)
            c_args.push_back(const_cast<char*>(a.c_str()));
        c_args.push_back(nullptr);

        std::vector<char*> c_env;
        for (auto &e : env)
            c_env.push_back(const_cast<char*>(e.c_str()));
        c_env.push_back(nullptr);

        execve(scriptPath.c_str(), c_args.data(), c_env.data());
        perror("execve failed");
        _exit(1);
    } 
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);

        if (!input.empty()) {
            write(pipe_in[1], input.c_str(), input.size());
        }
        close(pipe_in[1]);

        std::string raw;
        char buffer[1024];
        ssize_t n;
        while ((n = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
            raw.append(buffer, n);
        close(pipe_out[0]);

        waitpid(pid, nullptr, 0);

        CGIResponse resp;
        size_t pos = raw.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            resp.headers = raw.substr(0, pos);
            resp.body = raw.substr(pos + 4);
        }
        else
        {
            resp.body = raw;
        }
        return resp;
    }
}