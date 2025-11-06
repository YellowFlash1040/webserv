#include "CGI.hpp"

#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <map>

std::string cleanShebang(const std::string& line)
{
    if (line.size() < 3 || line[0] != '#' || line[1] != '!')
        return "";

    size_t start = 2;

    while (start < line.size() && std::isspace(line[start]))
        start++;

    if (start >= line.size())
        return "";

    size_t end = line.find_first_of(" \t\n", start);
    if (end == std::string::npos)
        end = line.size();

    return line.substr(start, end - start);
}

std::string getShebangInterpreter(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Failed to open script for reading shebang: " + path);

    std::string line;
    std::getline(f, line);
    std::string interpreter = cleanShebang(line);

    if (interpreter.empty() || interpreter[0] != '/')
        throw std::runtime_error("Invalid or missing shebang in script: " + path);

    struct stat st;
    if (stat(interpreter.c_str(), &st) == -1 || !(st.st_mode & S_IXUSR))
        throw std::runtime_error("Interpreter in shebang not found or not executable: " + interpreter);

    return interpreter;
}

std::string validateScriptPath(const std::string& scriptPath, const std::string& rootDir)
{
    struct stat st;
    if (stat(scriptPath.c_str(), &st) == -1)
        throw std::runtime_error("CGI script not found: " + scriptPath);

    if (S_ISDIR(st.st_mode))
        throw std::runtime_error("CGI script is a directory: " + scriptPath);

    char realScript[PATH_MAX];
    char realRoot[PATH_MAX];
    if (!realpath(scriptPath.c_str(), realScript) || !realpath(rootDir.c_str(), realRoot))
        throw std::runtime_error("Failed to resolve real path");

    std::string realScriptStr(realScript);
    std::string realRootStr(realRoot);
    if (realScriptStr.find(realRootStr) != 0)
        throw std::runtime_error("Path traversal detected: " + scriptPath);

    if (st.st_mode & S_IXUSR)
        return scriptPath;

    std::string interpreter = getShebangInterpreter(scriptPath);
    if (!interpreter.empty())
        return interpreter;

    throw std::runtime_error("CGI script is not executable and has no shebang: " + scriptPath);
}

std::string CGI::execute(const std::string& scriptPath,
                    const std::vector<std::string>& args,
                    const std::vector<std::string>& env,
                    const std::string& input,
                    const std::string& rootDir)
{
    std::string execPath = validateScriptPath(scriptPath, rootDir);

    int pipe_in[2], pipe_out[2];

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        close(pipe_in[0]);
        close(pipe_in[1]);
        throw std::runtime_error("Failed to create pipes");
    }

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("Fork failed");

    if (pid == 0)
    {
        if (dup2(pipe_in[0], STDIN_FILENO) == -1 ||
            dup2(pipe_out[1], STDOUT_FILENO) == -1 ||
            dup2(pipe_out[1], STDERR_FILENO) == -1)
        {
            perror("dup2 failed");
            _exit(1);
        }

        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);

        std::vector<char*> c_args;
        if (execPath != scriptPath)
        {
            c_args.push_back(const_cast<char*>(execPath.c_str()));
            c_args.push_back(const_cast<char*>(scriptPath.c_str()));
        }
        else
        {
            c_args.push_back(const_cast<char*>(scriptPath.c_str()));
        }
        
        for (auto &a : args)
            c_args.push_back(const_cast<char*>(a.c_str()));
        c_args.push_back(nullptr);

        std::vector<char*> c_env;
        for (auto &e : env)
            c_env.push_back(const_cast<char*>(e.c_str()));
        c_env.push_back(nullptr);

        execve(execPath.c_str(), c_args.data(), c_env.data());
        perror("execve failed");
        _exit(1);
    }
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);

        size_t total = 0;
        if (!input.empty())
        {
            while (total < input.size()) 
            {
                size_t n = write(pipe_in[1], input.c_str() + total, input.size() - total);
                if (n <= 0) throw std::runtime_error("write failed");
                total += n;
            }
        }

        close(pipe_in[1]);

        std::string cgi_output;
        char buffer[1024];
        ssize_t n;

        while ((n = read(pipe_out[0], buffer, sizeof(buffer))) != 0)
        {
            if (n < 0)
            {
                if (errno == EINTR) continue;
                throw std::runtime_error("read failed");
            }
            cgi_output.append(buffer, n);
        }
        close(pipe_out[0]);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            int code = WEXITSTATUS(status);
            if (code != 0)
                std::cerr << "CGI exited with code " << code << std::endl;
        }
        else if (WIFSIGNALED(status))
        {
            std::cerr << "CGI killed by signal " << WTERMSIG(status) << std::endl;
        }

        return cgi_output;
    }
}

std::string methodToString(HttpMethodEnum method)
{
    switch (method)
    {
        case HttpMethodEnum::GET:    return "GET";
        case HttpMethodEnum::POST:   return "POST";
        case HttpMethodEnum::PUT:    return "PUT";
        case HttpMethodEnum::DELETE: return "DELETE";
        default:                     return "NONE";
    }
}

std::vector<std::string> CGI::buildEnvFromRequest(const RequestData& req)
{
    std::vector<std::string> env;

    env.push_back("REQUEST_METHOD=" + methodToString(req.method));
    env.push_back("QUERY_STRING=" + req.query);
    env.push_back("CONTENT_LENGTH=" + std::to_string(req.body.size()));

    std::unordered_map<std::string, std::string>::const_iterator it = req.headers.find("content-type");
    if (it != req.headers.end())
        env.push_back("CONTENT_TYPE=" + it->second);

    env.push_back("SCRIPT_NAME=" + req.uri);
    env.push_back("SERVER_PROTOCOL=" + req.httpVersion);
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");

    for (std::unordered_map<std::string, std::string>::const_iterator it2 = req.headers.begin();
         it2 != req.headers.end(); ++it2)
    {
        std::string headerName = "HTTP_" + it2->first;
        for (std::string::iterator ch = headerName.begin(); ch != headerName.end(); ++ch)
        {
            if (*ch == '-')
                *ch = '_';
            else
                *ch = static_cast<char>(std::toupper(*ch));
        }
        env.push_back(headerName + "=" + it2->second);
    }

    return env;
}
