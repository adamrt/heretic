cmake_minimum_required(VERSION 3.5)
project(starterkit)
set(CMAKE_C_STANDARD 99)

# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Enable compile commands export for tools like clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add Sokol library
add_library(sokol STATIC lib/sokol.c)
target_include_directories(sokol PUBLIC lib/sokol)

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
add_executable(starterkit src/main.c)

# Link Sokol library
target_link_libraries(starterkit sokol)

# Emscripten specific settings
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(starterkit PUBLIC --shell-file ../lib/shell.html)
    target_link_options(starterkit PUBLIC -sUSE_WEBGL2=1 -sNO_FILESYSTEM=1 -sASSERTIONS=0 -sMALLOC=emmalloc --closure=1)
endif()

target_compile_options(starterkit PUBLIC -Wall -Wextra -Wsign-compare)

# Explicitly strip dead code
if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(starterkit PRIVATE LINKER:-dead_strip)
elseif (CMAKE_C_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(starterkit PRIVATE -Wl,--gc-sections)
endif()
