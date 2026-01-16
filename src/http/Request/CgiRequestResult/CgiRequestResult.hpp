#pragma once
#include <string>

struct CgiRequestResult
{
    bool spawnCgi = false;
    std::string cgiInterpreter;
    std::string cgiScriptPath;
    RequestData requestData; 
};
