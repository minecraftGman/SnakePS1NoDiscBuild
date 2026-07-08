# Snake — disc-free BIOS-shell build

This is a trimmed-down version of the PSX_Snake_Alpha source, modified to
run with **no CD required** — suitable for embedding directly into a
PS1 BIOS shell patch.

## What changed from the original

- **All CD access removed.** `ReadCD.c` is no longer used. Textures and
  models are compiled directly into the executable as byte arrays
  (`Assets.c`), generated from the original `.TIM`/`.TMD` files.
- **Jungle scenery dropped.** Ground/rocks/trees/plants (models + textures)
  were decorative only and didn't fit the budget — removed along with the
  `RenderObject()` calls that drew them.
- **Loading/control splash images dropped.** `LOADING.TIM` (66KB) and
  `CONTROL.TIM` (131KB) alone would have blown the entire size budget.
  Replaced with a plain text "SNAKE / PRESS X TO START" screen drawn with
  the game's own bitmap font.
- **Sound effects and music dropped.** The `.VAG` sound effects added
  ~45–275KB depending on which ones you keep, and the music was literal
  CD-DA audio tracks — physically impossible to embed in a ROM chip, since
  it's not file data at all, it's audio read directly off a spinning disc.
  `audio_init()` (SPU init) is still called, so this is straightforward to
  add back in later if you want basic bite/die sound effects — there's
  budget room (see below).

## What's kept

- Full Snake gameplay (movement, growth, collision, speed, scoring)
- Apple model + texture
- Snake head/body/curve/tail models
- Custom bitmap font for the UI text

## Size budget

The BIOS shell patcher gives you **307,200 bytes** total (code + data +
relocation stub) to work with.

| Item | Size |
|---|---|
| Embedded assets (2 textures + 5 models) | 37,456 bytes |
| Compiled code (previous full build was ~170KB with CD/jungle/audio code included) | ~150–170KB estimated |
| **Estimated total** | **~190–210KB, well under the 307,200 byte limit** |

There's meaningful headroom left — enough to add back a few short sound
effects (bite/die, ~2–12KB each) if you want, without rewriting anything
structural.

## How to build

You need **PsyQ** (`ccpsx`/`cpe2x`) on your PATH — I don't have that
toolchain available to compile this myself. Run:

```
build.BAT
```

This links at `0x80040000` — chosen to sit clear of the shell patcher's
relocation stub at `0x80030000`. **Because you're compiling from source
this time, you don't need the binary address-relocation hack from
earlier at all** — the linker just puts it in the right place directly.

If you're using PSn00bSDK (GCC-based) instead of PsyQ, the API calls here
(`GsDOBJ2`, `GsSortObject4`, `GsMapModelingData`, etc.) are PsyQ-style;
PSn00bSDK provides a compatible GTE/GPU layer but you may need to adjust
include paths/library names.

## Building via GitHub Actions

`.github/workflows/build.yml` will build `main.exe` for you on push, using
a Windows runner (PsyQ's `ccpsx.exe`/`cpe2x.exe` are native Windows PE
binaries, so no Wine/emulation layer is needed).

**You need to supply PsyQ yourself as a repo secret** — it's proprietary
Sony-licensed tooling, so this workflow deliberately doesn't fetch it from
anywhere. Full instructions are in the comments at the top of the workflow
file; short version:

1. Zip your PsyQ install folder (containing `CCPSX.EXE`, `CPE2X.EXE`,
   `LIB/`, `INCLUDE/`).
2. Base64-encode the zip.
3. Add it as a repository secret named `PSYQ_TOOLCHAIN_B64`.

The workflow decodes it at build time, adds it to `PATH`, compiles, and
uploads `main.exe` as a build artifact. It does **not** attempt to run the
BIOS patcher — that step needs your own dumped BIOS, which shouldn't live
in CI or a repo either, so that stays a manual step on your machine with
`biosshellpatcher.html`.

I haven't been able to actually run this workflow (no PsyQ available to
me, and I can't create a PsyQ secret to test against), so treat the first
run as the real test — if `ccpsx`/`cpe2x`'s exact flags differ from what's
in `build.BAT`/`build.yml`, or your zip's internal folder layout doesn't
match the `Add PsyQ to PATH` step, you may need to adjust paths there.

## Before flashing to real hardware

Test `main.exe` in an emulator first (DuckStation supports loading a raw
`.exe` directly, no BIOS patch needed, via its "Boot EXE" option). Once
that runs cleanly, patch it into your BIOS dump with
`biosshellpatcher.html` and test the patched BIOS in the emulator before
flashing a physical chip.

I don't have a PS1 emulator available in this environment, so this build
is un-emulator-tested on my end — please treat the first boot as the real
test.
