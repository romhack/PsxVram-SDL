PsxVram-SDL
============
Viewer for displaying vram in uncompressed ePSXe savestates or from straight video ram dumps.

Features:
-----------
- Code was totally rewritten from Windows API to crossplatform SDL
- CLUT possible positions are lessened


Usage:
-----------
```
PsxVram-SDL [DumpFileName]
```
***DumpFileName*** - This can be either PSX's 1MB Video RAM dump (which can be obtained from PCSX dump function) or unzipped ePSXe savestate. Utility will search for default "vram.bin" in current directory, then check for command line argument and then wait for drag-n-drop input if nothing was found.

Hotkeys
-----------

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

Issues
------

### High-DPI on Windows 10

To get 1:1 scaling back, do the following:

- in *Windows Explorer*
  - right-click the application executable and click *Properties*
- in *Settings* section
  - click *Change high DPI settings*
- in *High DPI scaling override* section
  - enable *Override high DPI scale behavior*
  - set *Scaling performed by:* to *Application*
- restart the application

For license information please see LICENSE.md
Original idea by Agemo http://www.romhacking.net/community/737/



============
Griever.Magicteam.net
