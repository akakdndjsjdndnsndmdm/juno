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
#elif defined(__ANDROID__)
        return "android";
#elif defined(__linux__)
        return "linux";
#else
        return "unknown";
#endif
    }
}
