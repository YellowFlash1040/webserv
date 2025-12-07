#ifndef BODYTYPE_HPP
#define BODYTYPE_HPP

#include <string>

namespace BodyType
{
    // Enum for different HTTP body types
    enum Type
    {
        NO_BODY,   // No body present
        CHUNKED,   // Transfer-Encoding: chunked
        SIZED,     // Content-Length specified
        ERROR      // Invalid or unexpected body state
    };

    // Convert enum value to string (for debugging)
    inline std::string toString(Type type)
    {
        switch (type)
        {
            case NO_BODY:   return "NO_BODY";
            case CHUNKED:   return "CHUNKED";
            case SIZED:     return "SIZED";
            case ERROR:     return "ERROR";
        }
        return "UNKNOWN";
    }
}

#endif