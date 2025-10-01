#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <stdexcept>

struct CGIResponse {
    std::string headers;
    std::string body;
};

class CGI
{
public:
    static CGIResponse execute(const std::string &scriptPath, const std::vector<std::string> &args, const std::vector<std::string> &env, const std::string &input = "");
};
