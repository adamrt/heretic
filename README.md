# Heretic: A Final Fantasy Tactics Toolkit

Heretic is a set of tools for looking at the contents of the Final Fantasy
Tactics PSX image.

It uses c99, sokol, imgui, cglm.

![Fort Zeakden](https://github.com/adamrt/heretic/blob/master/res/fort-night.png)

### Required

The PSX US Final Final Fantasy Tactics image. The file `fft.bin` must be placed in the project folder.

- serial: SCUS-94221 
- shasum: 2b5d4db3229cdc7bbd0358b95fcba33dddae8bba

### Build and run

To fetch dependencies, compile shaders and build for your platform, run:

- `./build.sh native` for Mac (Metal), Linux (OpenGL) 
- `./build.sh wasm` for the browser (WebGPU)

