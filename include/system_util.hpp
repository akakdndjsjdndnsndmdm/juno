//
// Created by margo on 15/11/2025.
//

#pragma once
#include <string>

namespace system_util
{
    static std::string get_system_platform( )
    {
#ifdef _WIN32
        return "win32";
#elif defined(_WIN64)
        return "win64";
#elif defined(__unix__) || defined(__unix)
        return "unix";
#else
        return "unknown";
#endif
    }
}