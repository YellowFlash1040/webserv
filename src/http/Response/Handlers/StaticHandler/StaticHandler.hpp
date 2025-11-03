#ifndef STATICHANDLER_HPP
#define STATICHANDLER_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../../FileHandler/FileHandler.hpp"
#include "ErrorHandler.hpp"
#include "../../../Request/RequestData/RequestData.hpp"
#include "../../../../config/shared/RequestContext/RequestContext.hpp"

namespace StaticHandler
{
    std::string serve(FileHandler& fileHandler, const RequestContext& ctx, const RequestData& req);

    std::string serveStaticFile(FileHandler& fileHandler, const RequestContext& ctx, const std::string& path);
    std::string handleStaticDirectory(FileHandler& fileHandler, const RequestContext& ctx, const std::string& path);
    std::string handleStaticError(FileHandler& fileHandler, const RequestContext& ctx, const std::exception& e);
    std::string handleNotFound(const RequestContext& ctx);
}

#endif