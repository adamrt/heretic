# Starter Kit for Experiments

This starter kit is to get you up and running with a cross platform sokol application.

It uses C, CMake, Sokol.

- `./build.sh native` for Mac (Metal), Linux (OpenGL), Windows (DX11)
- `./build.sh wasm` for the browser (WebGPU)

This will fetch `sokol-shdc` for your platform, compile the shader for your
backend, build the application and run it.

**Windows is untested.** I don't have access to a Windows machine. The
`build.sh` script needs to be modified to get the Windows `sokol-shdc` and the
output format for DX11 needs to be added.

