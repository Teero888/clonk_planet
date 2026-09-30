# Clonk Planet Linux Port

This is a port of Clonk Planet (originally released in 1999-2000 by RedWolf Design) to Linux (and any modern OS really). It includes the C4Engine and a Qt6-based launcher (thank you RedWolf that you published **ONLY** the engine source and not the launcher, so i had to create it from 0). All gameplay features work as far as I have tested, so playing normally should work fine. Things might be broken - if you find any crashes or other issues please open an issue.

## Screenshots
<p align="center">
  <img width="48%" alt="FrameTee Editor View" src="https://github.com/user-attachments/assets/b2fa7914-18f6-45ee-bbcb-7d6417425e0c" />
  <img width="48%" alt="Skin Browser" src="https://github.com/user-attachments/assets/684b09a2-88a3-4f6a-b10b-de7b6a76262b" />
  <img width="48%" alt="Controls" src="https://github.com/user-attachments/assets/df896463-5ffa-4e4f-9561-2b18dad4416b" />
</p>

## Download

Ready to play builds for Linux and Windows are on the [releases page](../../releases): extract the archive
and run `clonk_launcher` (`clonk_launcher.exe`). The game folder is written to (players, savegames), so
extract it somewhere writable. The Linux build needs glibc 2.35 or newer (Ubuntu 22.04, Debian 12, ...).

## Status

* Engine: Ported to OpenGL, GLFW, and miniaudio. Runs on Linux and Windows (64 bit, built with MinGW-w64).
* Launcher: Recreated from the original Planet.exe: main window with player and developer view, new / rename / delete / drag & drop, scenario properties, options, network games, registration, quick start screen and the help file.
* Networking: Works, you can open lobbies and join servers. A simple masterserver is also included since the official one from [clonk.de](http://www.clonk.de/) doesn't work anymore.
* Audio: DirectSound interface is wrapped to miniaudio. MIDI playback is synthesized using a standalone helper binary (clonk_midi) with TinySoundFont and TinyMidiLoader, using the FluidR3 General MIDI soundfont (Ogg Vorbis compressed SF3 from MuseScore, see `planet_data/FluidR3Mono_License.md`).
* Joysticks: Implemented via GLFW events in standard/src/StdJoystick.cpp, configured on the Gamepad page of the launcher options.
* Console / Editor: Currently not implemented since porting the windows UI requires some effort.

## Building on Linux

You need a C++17 compiler, CMake, OpenGL, GLFW3, GLEW, Freetype2, and Qt6 (Widgets and Multimedia modules).

On Ubuntu/Debian:

```sh
apt-get install cmake build-essential libgl1-mesa-dev libglew-dev libglfw3-dev libfreetype-dev qt6-base-dev qt6-multimedia-dev
```

On Arch Linux:

```sh
pacman -S --needed base-devel cmake glew glfw-x11 freetype2 qt6-base qt6-multimedia
```

Build:
```
cmake -B build -S .
cmake --build build -j$(nproc)
```

To run the tests: `ctest --test-dir build`

## Building on Windows

Windows builds use the MinGW-w64 toolchain of [MSYS2](https://www.msys2.org/) (MSVC is not supported: the engine relies on the POSIX functions of MinGW). In the MSYS2 UCRT64 shell:

```sh
pacman -S --needed zip mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,glfw,glew,freetype,zlib,qt6-base,qt6-multimedia-wmf,qt6-multimedia}
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Creating a release

`cmake --install build --prefix <folder>` creates a self-contained game folder: the programs, the game data and the libraries they need (Qt, GLFW, GLEW, ...; on Linux in `lib/`, core system libraries like glibc, OpenGL and X11 are expected on the system). `-DCLONK_BUILD_LAUNCHER=OFF` builds only the engine.

The GitHub workflow builds and tests both platforms on every push and uploads the game folders; pushing a tag `v*` (e.g. `git tag v1.0.0 && git push origin v1.0.0`) publishes them as a release.

## How to Run

* Run Launcher:
  `./build/clonk_launcher`

* Host a lobby:
  `./build/clonk "Knights.c4f/Dunkelfels.c4s" "Knights.c4d" "Objects.c4d" /Lobby`

* Join a server:
  `./build/clonk "Joki.c4p" "/Join:127.0.0.1"`
* Note on Port Forwarding:
  If hosting a game behind a router, you must forward these ports to the host machine:
  - Port 11111 (TCP) - Control connection
  - Port 11112 (TCP) - Input connection

## Configuration

Settings are stored per user in `clonk.ini` (the original used the registry, `HKCU\Software\RedWolf Design\Clonk 4`):

* Linux: `~/.config/clonk-planet/clonk.ini` (or `$XDG_CONFIG_HOME/clonk-planet`)
* Windows: `%APPDATA%\Clonk Planet\clonk.ini`
* macOS: `~/Library/Application Support/Clonk Planet/clonk.ini`

It is created on the first start from the defaults in `planet_data/clonk.ini`. The environment variable `CLONK_CONFIG` selects a different file. Each registry key is a section, e.g.:

```ini
[General]
Language=US

[Network]
LocalName=Clonk
```

## Registration codes
```
Clonk Planet 4.65 Freeware

To enable all features of the game, enter the following player name and code
in 'Options' / 'Registration'.

Player name: Freeware Player
Code: 3400356577

Name and code have to be entered exactly as displayed above.

Notice: use of a freeware code does not entitle you to any upgrades, support,
or use of additional web offers.

Developer password: Siedlerclonk
```
