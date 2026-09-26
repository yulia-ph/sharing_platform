cmake_minimum_required(VERSION 4.0)
project(sharing_platform CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_executable(sharing_platform ${SOURCES})

target_include_directories(sharing_platform PRIVATE
    src
    ${SODIUM_INCLUDE_DIRS}
)

target_link_libraries(sharing_platform PRIVATE
    SQLite3::SQLite3
    ${SODIUM_LIBRARIES}
)

target_compile_options(sharing_platform PRIVATE
    -Wall -Wextra
)