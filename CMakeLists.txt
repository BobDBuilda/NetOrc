cmake_minimum_required(VERSION 3.10)
project(NetOrc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

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

# Thread support and SQLite3
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

include_directories(${SQLITE3_INCLUDE_DIRS})
target_link_libraries(NetOrc PRIVATE Threads::Threads ${SQLITE3_LIBRARIES})

# Validator unit tests
add_executable(validator_tests
    tests/validator_tests.cpp
)

target_include_directories(validator_tests PRIVATE
    src
    src/api/SouthBoundInterface/Classes
    src/api/SouthBoundInterface/Interfaces
    src/core/Classes
)

add_test(NAME validator_tests COMMAND $<TARGET_FILE:validator_tests>)

add_executable(app_integration_tests
    tests/app_integration_tests.cpp
    src/core/Classes/App.cpp
    src/api/SouthBoundInterface/Classes/SouthBoundInterface.cpp
)

target_include_directories(app_integration_tests PRIVATE
    src
    src/api/SouthBoundInterface/Classes
    src/api/SouthBoundInterface/Interfaces
    src/core/Classes
)

target_link_libraries(app_integration_tests PRIVATE Threads::Threads ${SQLITE3_LIBRARIES})
add_test(NAME app_integration_tests COMMAND $<TARGET_FILE:app_integration_tests>)
