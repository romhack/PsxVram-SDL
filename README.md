PsxVram-SDL
============
Viewer for displaying vram in uncompressed ePSXe savestates or from straight video ram dumps.



Usage:
-----------
```
PsxVram-SDL [DumpFileName]
```
***DumpFileName*** - This can be either PSX's 1MB Video RAM dump (which can be obtained from PCSX dump function) or unzipped ePSXe savestate. Utility will search for default "vram.bin" in current directory, then check for command line argument and then wait for drag-n-drop input if nothing was found.

Hotkeys
-----------

- w, s, a, d: move window
- shift: move for 1 pixel
- 1: 4 BPP mode 
- 2: 8 BPP mode
- 9: 15 BPP mode
- 0: 24 BPP (MDEC) mode
- ~: Reverse colors - works for 4 and 8 BPP modes in grayscale
- shift + (1 or 2): CLUT mode
- up, down, left, right: CLUT starting coordinates only for CLUT modes
- enter reloads source dump



For license information please see LICENSE.md

Original idea by Agemo http://www.romhacking.net/community/737/
Code was totally rewritten from Windows API to crossplatform SDL.

To do
============
- Probably, mouse move should show coords

============
Griever.Magicteam.net