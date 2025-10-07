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

std::string cleanShebang(const std::string &line)
{
    if (line.size() < 3 || line[0] != '#' || line[1] != '!')
        return "";

    size_t start = 2; // после #!
    // пропускаем пробелы в начале пути
    while (start < line.size() && std::isspace(line[start]))
        start++;

    if (start >= line.size())
        return "";

    size_t end = line.find_first_of(" \t\n", start);
    if (end == std::string::npos)
        end = line.size();

    return line.substr(start, end - start);
}

std::string getShebangInterpreter(const std::string &path)
{
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Failed to open script for reading shebang: " + path);

    std::string line;
    std::getline(f, line);
    std::string interpreter = cleanShebang(line);

    if (interpreter.empty() || interpreter[0] != '/')
        throw std::runtime_error("Invalid or missing shebang in script: " + path);

    // Проверяем существование интерпретатора
    struct stat st;
    if (stat(interpreter.c_str(), &st) == -1 || !(st.st_mode & S_IXUSR))
        throw std::runtime_error("Interpreter in shebang not found or not executable: " + interpreter);

    return interpreter;
}

// Проверка пути + shebang
std::string validateScriptPath(const std::string &scriptPath, const std::string &rootDir)
{
    struct stat st;
    if (stat(scriptPath.c_str(), &st) == -1)
        throw std::runtime_error("CGI script not found: " + scriptPath);

    if (S_ISDIR(st.st_mode))
        throw std::runtime_error("CGI script is a directory: " + scriptPath);

    // Защита от path traversal
    char realScript[PATH_MAX];
    char realRoot[PATH_MAX];
    if (!realpath(scriptPath.c_str(), realScript) || !realpath(rootDir.c_str(), realRoot))
        throw std::runtime_error("Failed to resolve real path");

    std::string realScriptStr(realScript);
    std::string realRootStr(realRoot);
    if (realScriptStr.find(realRootStr) != 0)
        throw std::runtime_error("Path traversal detected: " + scriptPath);

    // Если файл исполняемый, возвращаем путь к скрипту
    if (st.st_mode & S_IXUSR)
        return scriptPath;

    // Если не исполняемый, проверяем shebang
    std::string interpreter = getShebangInterpreter(scriptPath);
    if (!interpreter.empty())
        return interpreter; // будем вызывать через интерпретатор

    throw std::runtime_error("CGI script is not executable and has no shebang: " + scriptPath);
}

CGIResponse CGI::execute(const std::string &scriptPath,
                    const std::vector<std::string> &args,
                    const std::vector<std::string> &env,
                    const std::string &input,
                    const std::string &rootDir)
{
    std::string execPath = validateScriptPath(scriptPath, rootDir);

    int pipe_in[2], pipe_out[2];
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
        if (execPath != scriptPath)
        {
            // запускаем через интерпретатор
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

        if (!input.empty())
            write(pipe_in[1], input.c_str(), input.size());
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
