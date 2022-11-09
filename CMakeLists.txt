cmake_minimum_required(VERSION 3.22.0)

project(ps1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

if (UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
elseif(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(BUILD_DEBUG TRUE)
else()
    set(BUILD_RELEASE TRUE)
endif()

if (PLATFORM_WINDOWS)
    file (
        GLOB_RECURSE PROJECT_FILES

        main.cpp

        core/*.h
        core/*.cpp
    )

    add_executable(${PROJECT_NAME} ${PROJECT_FILES})
endif()