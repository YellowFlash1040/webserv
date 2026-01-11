#pragma once
#include <string>

struct RequestResult
{
    bool spawnCgi = false;
    std::string cgiInterpreter;
    std::string cgiScriptPath;
    RequestData requestData; 
};
