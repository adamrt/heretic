#!/usr/bin/env bash

set -eou pipefail

OS=$(uname)

# Validate parameters
if [[ $# -lt 1 || ! "$1" =~ ^(native|wasm)$ ]]; then
    echo "Usage: ./build.sh [native|wasm] [shader]"
    exit 1
fi

TARGET="$1"
COMPILE_SHADER_ONLY=${2:-} # Optional second argument to compile only the shader

# Check if submodules are populated by looking for a key file in each submodule
if [[ ! -f "lib/sokol/sokol_app.h" || ! -f "lib/cglm/cglm.h" || -f "lib/nuklear/nuklear.h" ]]; then
    echo "Submodules not initialized. Initializing and updating git submodules..."
    git submodule init
    git submodule update
fi

# Determine the number of CPU cores based on the OS
get_num_cores() {
    case "$OS" in
        Linux) NUMCORES=$(nproc) ;;
        Darwin) NUMCORES=$(sysctl -n hw.ncpu) ;;
        *) echo "Unsupported OS: $OS" && return 1 ;;
    esac
}

get_num_cores

# Function to download the sokol-shdc shader compiler if not present
if [[ ! -f "sokol-shdc" ]]; then
    echo "sokol-shdc not found. Downloading..."
    case "$OS" in
        Linux) OSPATH="linux" ;;
        Darwin) OSPATH="osx_arm64" ;;
        *) echo "Unsupported OS: $OS" && exit 1 ;;
    esac
    wget -q https://github.com/floooh/sokol-tools-bin/raw/master/bin/${OSPATH}/sokol-shdc
    chmod +x sokol-shdc
    echo "sokol-shdc downloaded and made executable."
fi

# Determine shader language based on target and OS
case "$TARGET" in
    wasm) SHADER_LANG="glsl300es" ;;
    native)
        case "$OS" in
            Linux) SHADER_LANG="glsl430" ;;
            Darwin) SHADER_LANG="metal_macos" ;;
            *) echo "Unsupported OS: $OS" && exit 1 ;;
        esac
        ;;
    *) echo "Invalid target: $TARGET" && exit 1 ;;
esac

# Compile the shader (this happens always)
echo "Compiling shader..."
./sokol-shdc -i src/shader.glsl -o src/shader.glsl.h -l "$SHADER_LANG"

# If the second argument is 'shader', skip the rest of the build
if [[ "$COMPILE_SHADER_ONLY" == "shader" ]]; then
    echo "Shader compilation complete. Skipping further build steps."
    exit 0
fi

# Create and enter the build directory
rm -rf build && mkdir -p build && cd build

# Build process
case "$TARGET" in
    wasm)
        echo "Building for WebAssembly..."
        emcmake cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=MinSizeRel ..
        cmake --build . -j"$NUMCORES"
        emrun starterkit.html
        ;;
    native)
        echo "Building natively..."
        cmake -DCMAKE_BUILD_TYPE=Debug ..
        cmake --build . -j"$NUMCORES"
        ./starterkit
        ;;
esac
