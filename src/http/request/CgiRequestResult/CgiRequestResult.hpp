#pragma once

#include <string>
#include "RequestData.hpp"

struct CgiRequestResult
{
    bool spawnCgi = false;
    std::string cgiInterpreter;
    std::string cgiScriptPath;
    RequestData requestData; 
};
