  # Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(cjmines VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add the main executable
add_executable(${PROJECT_NAME} ${SOURCES})

find_package(Curses)
target_link_libraries(${PROJECT_NAME} Curses::Curses)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
