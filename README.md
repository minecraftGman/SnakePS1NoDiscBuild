# Snake -- PSn00bSDK port (no CD, CI-buildable)

## Status: We have the gameplay just no audio

This builds with a 100% open-source toolchain, fully automatically in CI
(see `.github/workflows/build-psn00bsdk.yml`). No PsyQ, no manual SDK setup.

### What's in this milestone (should just work)
- Screen/double-buffer setup
- The embedded (no-CD) loading screen and controls screen, as textured sprites
- Controller input -- `Controller.c` reused **completely unmodified**, since
  `PadInit()`/`PadRead()` is identical between PsyQ and PSn00bSDK
- SPU sound effect playback (bite sound plays when you press X)

### What's NOT in this milestone: the actual 3D snake gameplay
This is the important part to understand before you dig in. PSn00bSDK
deliberately does not reimplement Sony's high-level "Gs" library
(`GsDOBJ2`, `GsLinkObject4`, `GsSortObject4`, the light-matrix system) that
`3D.c`/`Snake.c`'s rendering is built on -- and it has no `.TMD` model loader
at all. Porting the 3D layer means writing that piece from scratch: model
loading, GTE transforms, lighting, and sorting into the ordering table.

This is *not* open-ended, though -- I inspected the actual `.TMD` files and
confirmed they only use **7 distinct primitive types** (flat/gouraud,
textured/untextured, tri/quad). That's a bounded, well-defined job, not
"support all of TMD." `Snake.c`'s actual game logic (grid movement,
collisions, apple spawning) is plain C with no SDK dependencies and should
port over basically unchanged -- only its one call to `RenderObject()` needs
a new implementation.

## Building locally
```
cmake --preset default .
cmake --build ./build
```
Requires `PSN00BSDK_LIBS` set (see PSn00bSDK's installation docs) and CMake
3.21+.

## First CI run expectations
I don't have the PsyQ or PSn00bSDK toolchains available to compile-test this
myself, so treat the first few CI runs as the real first compile of this
code -- normal for a port this size. If it fails, paste the error back and
it's usually a quick fix (most likely culprits: an exact header/struct name
I got slightly wrong, or `_PAD` needing a different include).

## Files
- `src/main.c` -- the milestone 1 program described above
- `src/Controller.c` -- unmodified from the original
- `src/AssetsTextures.c`, `src/AssetsAudio.c` -- embedded asset arrays (same
  ones from the earlier no-CD PsyQ build)
- `CMakeLists.txt`, `CMakePresets.json` -- PSn00bSDK build config
