# 2D Game Engine (NAME WIP)

Zero code 2d Game engine using the LDtk Editor.

The engine provides pre-built LDtk Entities in the editor which form the building blocks of the game. Hopefully this will be flexible enough to support many different kinds of games from "topdown" to "platformers."

## Features


Users:

- Tilemap and TilemapLayer based rendering
- Sprite Rendering (WIP)
- Zero code game logic components (WIP)
- Built-in game logic components configured through LDtk Editor (WIP)
- Simple asset pipeline
  - Textures: libktx only
  - Shaders: spirv only
  - Asset Bundling: libdwarfs
- Multiple rendering api backends:
  - OpenGL (Requires OpenGL 4.6)
  - Vulkan (TBD)

Developers:

- Virtual Filesystem Support: A medium agnostic filesystem abstraction able to expose multiple locations as a single location, as well as from `dwarfs` bundles.
- Plugin System for backends
- Custom Logging system that takes inspiration from QT `QDebug` with categories.

- Backends:
  - OpenGL: Requires 4.6 for Direct State Access (DSA)

## Building

1. configure `cmake --preset=ninja-multi`
2. building `cmake --build build --config=release`
3. installing `cmake --install build --prefix=install_prefix`

or

1. configure+build+test+package `cmake --workflow --preset=release`

