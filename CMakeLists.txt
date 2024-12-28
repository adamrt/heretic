cmake_minimum_required(VERSION 3.5)
project(
  Heretic
  VERSION     0.1
  DESCRIPTION "Final Fantasy Tactics Toolkit"
  LANGUAGES   C CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3 -ggdb")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*,performance-*,-clang-analyzer-*")

# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

set(SHARED_INCLUDE_DIRS lib/sokol lib/sokol/util lib/dcimgui/src-docking)
set(SHARED_MAC_COMPILE_OPTIONS -x objective-c)
set(SHARED_MAC_FRAMEWORKS "-framework QuartzCore" "-framework Cocoa" "-framework MetalKit" "-framework Metal" "-framework OpenGL" "-framework AudioToolbox")
set(SHARED_LINUX_LIBS X11 Xi Xcursor GL asound dl m Threads::Threads)

function(add_shared_library lib_name source_file)
    add_library(${lib_name} STATIC ${source_file})
    target_include_directories(${lib_name} PUBLIC ${SHARED_INCLUDE_DIRS})

    if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        target_compile_options(${lib_name} PRIVATE ${SHARED_MAC_COMPILE_OPTIONS})
        target_link_libraries(${lib_name} ${SHARED_MAC_FRAMEWORKS})
    elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
        target_link_libraries(${lib_name} INTERFACE ${SHARED_LINUX_LIBS})
    endif()
endfunction()

# Add libraries using the helper function
add_shared_library(sokol lib/sokol.c)
add_library(cimgui
    lib/cimgui.c
    lib/dcimgui/src-docking/cimgui.cpp
    lib/dcimgui/src-docking/imgui.cpp
    lib/dcimgui/src-docking/imgui_widgets.cpp
    lib/dcimgui/src-docking/imgui_draw.cpp
    lib/dcimgui/src-docking/imgui_tables.cpp
    lib/dcimgui/src-docking/imgui_demo.cpp)

# Add Heretic executable
add_executable(heretic
    src/main.c
    src/background.c
    src/camera.c
    src/dialog.c
    src/event.c
    src/font.c
    src/game.c
    src/gfx.c
    src/gui.c
    src/instruction.c
    src/io.c
    src/lighting.c
    src/map.c
    src/map_record.c
    src/memory.c
    src/message.c
    src/model.c
    src/opcode.c
    src/mesh.c
    src/parse.c
    src/scenario.c
    src/scene.c
    src/span.c
    src/sprite.c
    src/texture.c
    src/time.c
    src/transition.c
    src/vm.c
    src/vm_func.c)

option(CGLM_SHARED "Shared build" OFF)
option(CGLM_STATIC "Static build"  ON)
option(CGLM_USE_C99 "" ON)
add_subdirectory(lib/cglm/ EXCLUDE_FROM_ALL)

target_link_libraries(heretic sokol cimgui cglm_headers)

# Emscripten specific settings
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(heretic PUBLIC --shell-file ../lib/shell.html)
    target_link_options(heretic PUBLIC
        -sUSE_WEBGL2=1
        -sNO_FILESYSTEM=0
        -sASSERTIONS=1
        -sMALLOC=emmalloc
        -sTOTAL_STACK=768mb
        -sTOTAL_MEMORY=1024mb
        -sEXPORTED_FUNCTIONS=_main,_data_init
        -sEXPORTED_RUNTIME_METHODS=ccall
        --preload-file=../imgui.ini@imgui.ini
        --preload-file=../res/loading.jpg@loading.jpg
        --closure=1)
endif()

if (NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
target_compile_options(heretic PUBLIC
    -Wall
    -g
    -O0
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
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(heretic PRIVATE LINKER:-dead_strip -fsanitize=address)

elseif (CMAKE_C_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(heretic PRIVATE -Wl, --gc-sections)
endif()
