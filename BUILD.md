# Build Instructions

## Overview

This project builds a `Homefront` freecam mod for Windows x86.

Main build outputs:

- `homefront_freecam.dll`
- `injector.exe`

`x64` is not supported. `CMakeLists.txt` explicitly fails configuration unless the target is `Win32`.

## Dependencies

- Windows
- CMake 3.20 or newer
- Visual Studio 2022
- `Desktop development with C++` workload
- MSVC x86 toolset
- CMake support for Windows

The project depends on `MinHook`, which is expected at `third_party/minhook`.

If you cloned the repository without submodules, initialize them first:

```powershell
git submodule update --init --recursive
```

Expected minimum layout:

- `third_party/minhook/include/MinHook.h`
- `third_party/minhook/src/buffer.c`
- `third_party/minhook/src/hook.c`
- `third_party/minhook/src/trampoline.c`
- `third_party/minhook/src/hde/hde32.c`

## Build From Developer PowerShell

From the repository root:

```powershell
cmake -S . -B build -A Win32
cmake --build build --config Release
```

For a debug build:

```powershell
cmake --build build --config Debug
```

These steps were verified locally and produce both the DLL and the injector successfully.

## Build From Visual Studio

1. Open the project folder in Visual Studio, or open `build/homefront_freecam.sln` after running CMake generation.
2. Select the `Win32` target platform.
3. Select the `Release` configuration.
4. Build `homefront_freecam` and `injector`, or just build `ALL_BUILD`.

## Output Paths

Typical output paths are:

- `build/Release/homefront_freecam.dll`
- `build/Release/injector.exe`

Depending on the generator, they may also appear under:

- `build/Win32/Release/homefront_freecam.dll`
- `build/Win32/Release/injector.exe`

If you want to confirm the locations:

```powershell
Get-ChildItem build -Recurse -Filter homefront_freecam.dll
Get-ChildItem build -Recurse -Filter injector.exe
```

## Preparing To Run

`injector.exe` expects `homefront_freecam.dll` to be in the same directory, because the DLL path is resolved relative to the injector executable.

Minimum runtime files:

- `injector.exe`
- `homefront_freecam.dll`

The simplest setup is to keep both files together in `build/Release`, or copy them together into a separate folder.

## Typical Run Flow

1. Start `Homefront`.
2. Run `injector.exe`.
3. If process access fails, run the injector as Administrator.
4. Wait for the `DLL loaded successfully` message.
5. Switch back to the game and press `F9` to enable freecam.
6. Press `F10` to unload the mod.

## Creating A Release

The repository includes a GitHub Actions workflow that publishes a release whenever you push a tag matching `v*`.

Typical release flow:

```powershell
git tag v0.1.0
git push origin master
git push origin v0.1.0
```

The workflow builds the `Win32` release on GitHub, packages:

- `homefront_freecam.dll`
- `injector.exe`
- `README.md`
- `BUILD.md`

and uploads:

- `Homefront-Free-Camera-v0.1.0-win32.zip`
- `SHA256SUMS.txt`

If GitHub Actions is disabled for the repository, the release workflow will not run until Actions is enabled again.

## Troubleshooting

### CMake fails because of architecture

Cause: the build was configured for something other than `Win32`.

Fix:

```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build -A Win32
```

### The injector says the DLL was not found

Cause: `homefront_freecam.dll` is not next to `injector.exe`.

Fix: place the DLL in the same directory as the injector.

### The injector cannot find the game process

Check that:

- the process name is actually `HOMEFRONT.exe`
- the game is already running
- the injector has enough privileges

### The DLL loads but freecam does not work

Most likely causes:

- a different `HOMEFRONT.exe` build is being used
- `GetActorEyesViewPoint` moved
- the pawn offsets changed
- the mod was not built for x86

In that case, re-check `VA_EYES` and the pawn offsets in [`src/hooks.cpp`](D:/proj/Fcam/src/hooks.cpp).
