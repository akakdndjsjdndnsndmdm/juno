//
// Created by margo on 15/11/2025.
//

#pragma once

#include <string>
#include <string_view>

namespace constants
{
    constexpr std::string_view APP_NAME = "juno";
    constexpr std::string_view APP_VERSION = "0.1.0";
    constexpr std::string_view APP_COMMIT { "@GIT_COMMIT_HASH@" };
    constexpr std::string_view COMPILER_INFO = "@CMAKE_CXX_COMPILER_ID@ @CMAKE_CXX_COMPILER_VERSION@";
    constexpr std::string_view BUILD_ARCH = "@CMAKE_SYSTEM_PROCESSOR@";
}