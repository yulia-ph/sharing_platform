cmake_minimum_required(VERSION 4.0)
project(sharing_platform CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(httplib nlohmann_json)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_executable(sharing_platform ${SOURCES})

target_include_directories(sharing_platform PRIVATE
    src
    src/db
    src/domain
    src/http
    src/repositories
    src/services
    src/utils
    ${SODIUM_INCLUDE_DIRS}
)

target_link_directories(sharing_platform PRIVATE ${SODIUM_LIBRARY_DIRS})

target_link_libraries(sharing_platform PRIVATE
    SQLite3::SQLite3
    ${SODIUM_LIBRARIES}
    httplib::httplib
    nlohmann_json::nlohmann_json
    Threads::Threads
)

target_compile_options(sharing_platform PRIVATE
    -Wall -Wextra
)