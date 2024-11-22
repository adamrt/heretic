# Heretic: A Final Fantasy Tactics Toolkit

Heretic is a set of tools for looking at the contents of the Final Fantasy
Tactics PSX image.

It uses c99, sokol, nuklear, cglm.

# Build

**Required**: The PSX US Final Final Fantasy Tactics image
    - serial: SCUS-94221 
    - shasum: 2b5d4db3229cdc7bbd0358b95fcba33dddae8bba

The file `fft.bin` (PSX BIN) must be placed in the project folder.

- `./build.sh native` for Mac (Metal), Linux (OpenGL) 
- `./build.sh wasm` for the browser (WebGPU)

This will fetch dependencies, compile shaders and build for your platform and graphics API.

### Thanks

- [FFHacktics](https://ffhacktics.com) for everything
- [@mmatyas](https://github.com/mmatyas/) for sharing code and providing lots of pointers to docs and details.
- Yoshida Akihiko: Loading screen image



