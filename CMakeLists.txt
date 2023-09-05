cmake_minimum_required(VERSION 3.22.0)

project(ps1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

if (UNIX AND NOT APPLE)
    set(PS1_LINUX TRUE)
	list(APPEND CPP_DEFINITIONS PS1_LINUX)
elseif(WIN32)
    set(PS1_WINDOWS TRUE)
	list(APPEND CPP_DEFINITIONS PS1_WINDOWS)
elseif(APPLE)
    set(PS1_MACOS TRUE)
	list(APPEND CPP_DEFINITIONS PS1_MACOS)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_DEBUG TRUE)
	list(APPEND CPP_DEFINITIONS PS1_DEBUG)
else()
    set(BUILD_RELEASE TRUE)
	list(APPEND CPP_DEFINITIONS PS1_RELEASE)
endif()

if (PS1_WINDOWS)
    add_subdirectory(libs)
    include_directories(core)

    file (
        GLOB_RECURSE PROJECT_FILES

        main.cpp

        core/*.h
        core/*.cpp
    )

    add_executable(${PROJECT_NAME} ${PROJECT_FILES})
    target_link_libraries(${PROJECT_NAME} ps1_libs)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${CPP_DEFINITIONS})
elseif (PS1_LINUX)
    add_subdirectory(libs)
    include_directories(core)

    file (
        GLOB_RECURSE PROJECT_FILES

        main.cpp

        core/*.h
        core/*.cpp
    )
    add_executable(${PROJECT_NAME} ${PROJECT_FILES})
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${CPP_DEFINITIONS})
    target_link_libraries(${PROJECT_NAME} ps1_libs)
    target_link_libraries(${PROJECT_NAME} -lGL -lGLEW -lglfw)
endif()