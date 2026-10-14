# Immersive First Person for macOS (Apple Silicon)

A port of Immersive First Person v2.1.2 (AGPL-3.0) to the native macOS build of Cyberpunk 2077 (2.3.1, Steam, Apple Silicon). Port by codeman101 (skyman3215).

Everything macOS-specific is in this folder and in `#ifdef __APPLE__` blocks in `native/src/Main.cpp`. The Windows build is meant to be unchanged: `WINDOWS_CHECK.md` is the `unifdef -U__APPLE__` comparison against tag v2.1.2. What is not done yet: `NOT_YET.md`.

This folder sits next to `native/`, not under it, so `native/CMakeLists.txt` stays the tag file. The session launcher looks for `macos/build/ImmersiveFirstPerson.dylib`.

## Status

The dylib builds for arm64. It has not been run inside the game.

One native is registered on `gamePuppet`: `ImmersiveFirstPersonGetHeightGraphStatus`. It reads the root animated component's animgraph and reports whether the height-blend contract is loaded. The Lua side calls it and treats a missing native as unavailable.

## What you need to run it

- Cyberpunk 2077 2.3.1, the native macOS build (Steam), on Apple Silicon.
- A RED4ext loader for macOS that calls `ifp_registerNatives` after every plugin has loaded (at script initialisation). The game does not run RTTI register callbacks. The port was developed against the same loader setup as the macOS ports of TweakXL, ArchiveXL, Codeware and Mod Settings.
- Cyber Engine Tweaks, for the Lua modules. The height archive and `LookAt.reds` are the existing session-mod files. This dylib does not replace them.

No hooks. No address rows of its own. The address table is the macOS RED4ext SDK plus Codeware's `macos/MacSdkRows.inc`. An unknown hash goes to a trap that logs once and returns a stub.

## Build

```
cmake -S macos -B macos/build -DCMAKE_BUILD_TYPE=Release
cmake --build macos/build -j8
```

Needs the Xcode command line tools and CMake 3.21 or newer. Nothing from Homebrew is linked. The logger is the RED4ext logger pointer passed into `Main`.

The macOS RED4ext SDK is the `RED4ext.SDK-macos` checkout beside the ports tree (override with `-DRED4EXT_SDK_DIR=`). Codeware's Mac rows are the `cp2077-codeware` checkout beside it (override with `-DCODEWARE_DIR=`).

After the link, `fix-install-names.sh` rewrites a Homebrew install name to `@loader_path` when one is present, then signs ad hoc. `check-undefined.sh` checks the exports (`Main`, `Query`, `Supports`, `ifp_registerNatives`) and that no symbol is left for dynamic lookup.

## Install layout

`red4ext/plugins/ImmersiveFirstPerson/ImmersiveFirstPerson.dylib`

The modded-session launcher moves that file in while the Immersive First Person session mod is on, and moves it back when the session ends. If the dylib has not been built, the session stays Lua-only and the launcher says so.

## How the macOS build differs

- **SDK.** `CMakeLists.txt` writes `build/sdk-overlay`: the macOS SDK address table plus Codeware's `macos/MacSdkRows.inc`. An unknown hash resolves to `ifp_mac_trap_for_hash` (`MacTrap.cpp`), which logs once and returns a stub. The stub logs its first call and returns 0. The shared SDK checkout is not edited.
- **Names.** Upstream `Main.cpp` uses RED4ext API v1 names. `MacSdkCompat.hpp` is included only under `#ifdef __APPLE__` and aliases those names to the macOS SDK. `Supports()` still says `RED4EXT_API_VERSION_1` in the source. On Apple that macro is the loader's latest API version.
- **Natives.** `Main` does not add RTTI register callbacks. `ifp_registerNatives` runs `PostRegisterTypes` once. A second call logs and returns. If the puppet class is missing, `PostRegisterTypes` logs and returns.
- **Vault.** The SDK's allocator, id counters, and `IScriptable` slots are the hidden stand-ins in `MacVault.cpp` (the same shape as the other Mac ports). They are not exports. Async ids use band `0x7FFA0000`. The serializable counter starts at base `1<<62` through `1<<55`.
- **Fields.** The field offsets this plugin reads match the Windows SDK and the Mac binary. There is no Apple-only field override. `NOT_YET.md` notes the anim-variable container class-size tail.
