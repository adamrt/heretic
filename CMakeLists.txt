cmake_minimum_required(VERSION 3.5)
project(starterkit)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Enable compile commands export for tools like clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add Sokol library
add_library(sokol STATIC lib/sokol.c)
target_include_directories(sokol PUBLIC lib/sokol lib/sokol/util)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    # Compile sokol.c as Objective-C
    target_compile_options(sokol PRIVATE -x objective-c)
    target_link_libraries(sokol
        "-framework QuartzCore"
        "-framework Cocoa"
        "-framework MetalKit"
        "-framework Metal"
        "-framework OpenGL"
        "-framework AudioToolbox")
elseif (CMAKE_SYSTEM_NAME STREQUAL Linux)
    # Linux specific settings
    target_link_libraries(sokol INTERFACE X11 Xi Xcursor GL asound dl m Threads::Threads)
endif()

# Add Starterkit executable
add_executable(starterkit src/main.c src/bin.c src/camera.c src/game.c src/gui.c src/gfx.c src/model.c)

option(CGLM_SHARED "Shared build" OFF)
option(CGLM_STATIC "Static build"  ON)
option(CGLM_USE_C99 "" ON)
add_subdirectory(lib/cglm/ EXCLUDE_FROM_ALL)
target_include_directories(starterkit PUBLIC lib/nuklear)

# Link sokol and cglm library
target_link_libraries(starterkit sokol cglm_headers)

# Emscripten specific settings
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(starterkit PUBLIC --shell-file ../lib/shell.html)
    target_link_options(starterkit PUBLIC -sUSE_WEBGL2=1 -sNO_FILESYSTEM=1 -sASSERTIONS=0 -sMALLOC=emmalloc --closure=1)
endif()

target_compile_options(starterkit PUBLIC -Wall -Wextra -Wpedantic -Werror -Wno-unused-function -fsanitize=address -fno-omit-frame-pointer)

# Explicitly strip dead code
if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(starterkit PRIVATE LINKER:-dead_strip -fsanitize=address)
elseif (CMAKE_C_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(starterkit PRIVATE -Wl,--gc-sections -fsanitize=address)
endif()

