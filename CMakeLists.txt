cmake_minimum_required(VERSION 3.10)
project(NetOrc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    src
    src/api/SouthBoundInterface/Classes
    src/api/SouthBoundInterface/Interfaces
    src/core/Classes
)

# Source files
set(SOURCES
    src/index.cpp
    src/core/Classes/App.cpp
    src/api/SouthBoundInterface/Classes/SouthBoundInterface.cpp
    src/api/SouthBoundInterface/Utils/HandleData.cpp
)

# Executable
add_executable(NetOrc ${SOURCES})

# Thread support
find_package(Threads REQUIRED)
target_link_libraries(NetOrc PRIVATE Threads::Threads)
