PsxVram-SDL
===========

Viewer for displaying PSX VRAM dumps and save states from PCSX, ePSXe and NO$PSX.

Features
--------

- Cross platform SDL application
- Support of PCSX, ePSXe and NO$PSX save states 
- Lessened CLUT possible positions for faster navigation
  
Usage
-----

```
PsxVram-SDL [DumpFileName]
```

***DumpFileName*** can be one of the following:

- PCSX VRAM dump
- ePSXe unzipped savestate (7-Zip's context menu can unzip it)
- NO$PSX uncompressed savestate (configurable in NO$PSX's SAV/SNA File Format option)

Also, the application does the following:

- if a `vram.bin` file is found in current directory, it will be loaded immediately
- (else) if `DumpFileName` command line argument is set, it will be loaded instead
- (else) waits for the user to drop a file onto application's main window

Hotkeys
-------

- W/S/A/D: move window
- Ctrl + direction: move for 1 pixel
- Shift + direction: mode viewer resize
- 1: 4 BPP mode 
- 2: 8 BPP mode
- 9: 15 BPP mode
- 0: 24 BPP (MDEC) mode
- ~: Reverse colors (for 4 and 8 BPP grayscale modes)
- Shift + 1 or 2: CLUT mode
- Up/Down/Left/Right: CLUT starting coordinates (only for CLUT modes)
- Enter: reloads source dump
- Ctrl + mouse over mode viewer: show VRAM offset

Building
--------

On Windows, you will need vcpkg along SDL2, i.e. `vcpkg install sdl2:x86-windows sdl2:x64-windows`.
MSYS build on Windows is also possible, using make file with usual SDL2 installation above MSYS

Licence
-------

For license information please see LICENSE.md

Credits
-------

- Original idea by Agemo http://www.romhacking.net/community/737/
- https://github.com/romhack/
- https://github.com/aybe/

