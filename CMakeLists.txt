cmake_minimum_required(VERSION 3.25)
project(light_game LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -- Find/Fetch RayLib ----
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET)

if(NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SYSTEM
    )
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(raylib)
endif()

# -- Find/Fetch RayLib ----

# -- Find/Fetch nlohmann/json ----
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nlohmann_json)

# -- Find/Fetch nlohmann/json ----

# -- Source Library ----
add_library(src STATIC
    src/Game.cpp)

target_include_directories(src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(src PUBLIC raylib nlohmann_json)

# -- Source Library ----

# -- Game ----
add_executable(${PROJECT_NAME} main.cpp)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_options(src PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(src PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE src)

# target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Copy assets next to the binary
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)

# -- Testing ----
option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# -- Testing ----