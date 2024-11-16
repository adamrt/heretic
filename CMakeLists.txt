cmake_minimum_required(VERSION 3.5)
project(
  Heretic
  VERSION     0.1
  DESCRIPTION "Final Fantasy Tactics Toolkit"
  LANGUAGES   C)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3 -ggdb")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*,performance-*,-clang-analyzer-*")

# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Enable compile commands export for tools like clang-tidy

# Add Sokol and nuklear libraries
add_library(libs STATIC lib/libs.c)
target_include_directories(libs PUBLIC lib/sokol lib/sokol/util)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    # Compile libs.c as Objective-C
    target_compile_options(libs PRIVATE -x objective-c)
    target_link_libraries(libs
        "-framework QuartzCore"
        "-framework Cocoa"
        "-framework MetalKit"
        "-framework Metal"
        "-framework OpenGL"
        "-framework AudioToolbox")
elseif (CMAKE_SYSTEM_NAME STREQUAL Linux)
    # Linux specific settings
    target_link_libraries(libs INTERFACE X11 Xi Xcursor GL asound dl m Threads::Threads)
endif()

# Add Heretic executable
add_executable(heretic src/main.c src/bin.c src/camera.c src/game.c src/gui.c src/gfx.c src/model.c)

option(CGLM_SHARED "Shared build" OFF)
option(CGLM_STATIC "Static build"  ON)
option(CGLM_USE_C99 "" ON)
add_subdirectory(lib/cglm/ EXCLUDE_FROM_ALL)
target_include_directories(heretic PUBLIC lib/nuklear)

# Link sokol, nuklear and cglm library
target_link_libraries(heretic libs cglm_headers)

# Emscripten specific settings
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(heretic PUBLIC --shell-file ../lib/shell.html)
    target_link_options(heretic PUBLIC -sUSE_WEBGL2=1 -sNO_FILESYSTEM=1 -sASSERTIONS=0 -sMALLOC=emmalloc --closure=1)
endif()

target_compile_options(heretic PUBLIC
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wno-unused-function
    -Wshadow
    -Wunreachable-code
    -Wnull-dereference
    -Wformat=2
    -Wstrict-aliasing
    -Wimplicit-fallthrough
    -fsanitize=address
    -fsanitize=undefined
    -fno-omit-frame-pointer)

if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(heretic PRIVATE LINKER:-dead_strip -fsanitize=address)

elseif (CMAKE_C_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(heretic PRIVATE -Wl, --gc-sections)
endif()
