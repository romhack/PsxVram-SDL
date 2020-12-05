PsxVram-SDL
===========

Viewer for displaying vram in uncompressed ePSXe savestates or from straight video ram dumps.

Features
--------

- Support of PCSX, ePSXe and NO$PSX save states 
- Code was totally rewritten from Windows API to crossplatform SDL
- CLUT possible positions are lessened
  
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

- w, s, a, d: move window
- ctrl: move for 1 pixel
- shift: mode viewer resize
- 1: 4 BPP mode 
- 2: 8 BPP mode
- 9: 15 BPP mode
- 0: 24 BPP (MDEC) mode
- ~: Reverse colors - works for 4 and 8 BPP modes in grayscale
- shift + (1 or 2): CLUT mode
- up, down, left, right: CLUT starting coordinates only for CLUT modes
- enter reloads source dump
- ctrl + mouse movement over mode viewer window shows offset in VRAM

Building
--------

On Windows, you will need vcpkg along SDL2, i.e. `vcpkg install sdl2:x86-windows sdl2:x64-windows`.

Licence
-------

For license information please see LICENSE.md

Credits
-------

- Original idea by Agemo http://www.romhacking.net/community/737/
- Griever.Magicteam.net
- https://github.com/romhack/
- https://github.com/aybe/

