cmake_minimum_required(VERSION 3.20)
project(sharing_platform LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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

set(SQLITE3_DIR ${CMAKE_BINARY_DIR}/sqlite-amalgamation-3460100)
if(NOT EXISTS ${SQLITE3_DIR}/sqlite3.c)
    set(SQLITE3_ARCHIVE ${CMAKE_BINARY_DIR}/sqlite-amalgamation-3460100.zip)
    file(DOWNLOAD
        https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
        ${SQLITE3_ARCHIVE}
        EXPECTED_HASH SHA256=77823cb110929c2bcb0f5d48e4833b5c59a8a6e40cdea3936b99e199dbbe5784
    )
    file(ARCHIVE_EXTRACT INPUT ${SQLITE3_ARCHIVE} DESTINATION ${CMAKE_BINARY_DIR})
endif()

add_library(sqlite3 STATIC ${SQLITE3_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${SQLITE3_DIR})
target_compile_definitions(sqlite3 PRIVATE SQLITE_ENABLE_FTS5)
target_compile_options(sqlite3 PRIVATE -w)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_executable(sharing_platform ${SOURCES} src/db/spellfix.c)

set_source_files_properties(src/db/spellfix.c PROPERTIES COMPILE_FLAGS "-DSQLITE_CORE")

target_include_directories(sharing_platform PRIVATE
    src
    src/db
    src/domain
    src/http
    src/repositories
    src/services
    src/utils
    ${SODIUM_INCLUDE_DIRS}
    ${SQLITE3_DIR} 
)

target_link_directories(sharing_platform PRIVATE ${SODIUM_LIBRARY_DIRS})

target_link_libraries(sharing_platform PRIVATE
    sqlite3 
    ${SODIUM_LIBRARIES}
    httplib::httplib
    nlohmann_json::nlohmann_json
    Threads::Threads
)

target_compile_options(sharing_platform PRIVATE
    -Wall -Wextra
)