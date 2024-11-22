# Starter Kit for Experiments

This starter kit is to get you up and running with a cross platform sokol application.

It uses c99, sokol, glm.

# Build

The file `fft.bin` (PSX BIN) must be placed in the project folder.

- `./build.sh native` for Mac (Metal), Linux (OpenGL) 
- `./build.sh wasm` for the browser (WebGPU)

This will fetch `sokol-shdc` for your platform, compile the shader for your
backend, build the app and run it.

### Thanks

- [FFHacktics](https://ffhacktics.com) for everything
- [@mmatyas](https://github.com/mmatyas/) for sharing code and providing lots of pointers to docs and details.
- Yoshida Akihiko: Loading screen image



