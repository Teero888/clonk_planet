# Clonk Planet Linux Port (incomplete)

This is a port of Clonk Planet (originally released in 1999-2000 by RedWolf Design) to Linux (and any modern OS really). It includes the C4Engine and a Qt6-based launcher (thank you RedWolf that you published **ONLY** the engine source and not the launcher, so i had to create it from 0). All gameplay features work as far as I have tested, so playing normally should work fine.

## Screenshots
<p align="center">
  <img width="48%" alt="FrameTee Editor View" src="https://github.com/user-attachments/assets/b2fa7914-18f6-45ee-bbcb-7d6417425e0c" />
  <img width="48%" alt="Skin Browser" src="https://github.com/user-attachments/assets/684b09a2-88a3-4f6a-b10b-de7b6a76262b" />
  <img width="48%" alt="Controls" src="https://github.com/user-attachments/assets/df896463-5ffa-4e4f-9561-2b18dad4416b" />
</p>

## Status

* Engine: Ported to OpenGL, GLFW, and miniaudio.
* Launcher: It's not quite finished, a bunch of UI for customizing scenarios and networking is still missing.
* Networking: Works, you can open lobbies and join servers. A simple masterserver is also included since the official one from [clonk.de](http://www.clonk.de/) doesn't work anymore.
* Audio: DirectSound interface is wrapped to miniaudio. MIDI playback is synthesized using a standalone helper binary (clonk_midi) with TinySoundFont and TinyMidiLoader.
* Joysticks: Implemented via GLFW events in standard/src/StdJoystick.cpp. (Still no launcher UI though)
* Console / Editor: Currently not implemented since porting the windows UI requires some effort.

## Dependencies (Linux)

You need a C++17 compiler, CMake, OpenGL, GLFW3, GLEW, Freetype2, and Qt6 (Widgets and Multimedia modules).

On Ubuntu/Debian:

```sh
apt-get install cmake build-essential libgl1-mesa-dev libglew-dev libglfw3-dev libfreetype-dev qt6-base-dev qt6-multimedia-dev
```

On Arch Linux:

```sh
pacman -S base-devel cmake glew glfw-x11 freetype2 qt6-base qt6-multimedia
```

## How to Build
```
cmake -B build -S .
cmake --build build -j$(nproc)
```

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
