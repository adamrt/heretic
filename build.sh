#!/usr/bin/env bash

set -eou pipefail

OS=$(uname)

# Check if a parameter was provided
if [ "$#" -ne 1 ]; then
    echo "Usage: ./build.sh [native|wasm]"
    exit 1
fi

# Download the sokol-shdc shader compiler for the current platform
download_shader_compiler() {
    if [[ "$OS" == "Linux" ]]; then
        OSPATH="linux"
    elif [[ "$OS" == "Darwin" ]]; then
        OSPATH="osx_arm64"
    else
        echo "Unsupported OS: $OS"
        return 1
    fi

    # Download the sokol-shdc shader compiler
    wget -q https://github.com/floooh/sokol-tools-bin/raw/master/bin/${OSPATH}/sokol-shdc
    if [[ $? -ne 0 ]]; then
        echo "Failed to download sokol-shdc"
        return 1
    fi

    chmod +x sokol-shdc
    echo "sokol-shdc downloaded and made executable."
}

# Check if the file exists before calling the function
if [[ ! -f "sokol-shdc" ]]; then
    echo "sokol-shdc not found. Downloading..."
    download_shader_compiler
fi

# compile the shader
if [[ "$1" = "wasm" ]]; then
    ./sokol-shdc -i src/shader.glsl -o src/shader.glsl.h -l glsl300es
elif [[ "$OS" == "Linux" ]]; then
    ./sokol-shdc -i src/shader.glsl -o src/shader.glsl.h -l glsl430
elif [[ "$OS" == "Darwin" ]]; then
    ./sokol-shdc -i src/shader.glsl -o src/shader.glsl.h -l metal_macos
else
    echo "Unsupported OS: $OS"
fi

# Remove the existing build directory and recreate it
rm -rf build
mkdir build
cd build

if [ "$1" = "wasm" ]; then
    echo "Building for WebAssembly..."
    emcmake cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=MinSizeRel ..
    cmake --build .
    emrun starterkit.html

elif [ "$1" = "native" ]; then
    echo "Building natively..."
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build .
    ./starterkit

else
    echo "Invalid build target. Use 'native' or 'wasm'."
    exit 1
fi

