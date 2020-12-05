/*
 * PsxVram-SDL.c
 *
 * 2013-08-31:	Griever.Magicteam
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include <string.h>
#if _MSC_VER
#define F_OK 0
#else
#include <unistd.h>
#endif

#define VRAM_WIDTH 1024
#define VRAM_HEIGHT 512
#define VRAM_WIDTH_24BPP ((VRAM_WIDTH*sizeof(u16))/3)
#define CYAN_PIXEL 0x0000FFFF
#define MAGENTA_PIXEL 0x00FF00FF
#define RECT_W_INIT 0x100
#define RECT_H_INIT 0x80
#define RECT_X_INIT ((VRAM_WIDTH-RECT_W_INIT)/2)
#define RECT_Y_INIT ((VRAM_HEIGHT-RECT_H_INIT)/2)
#define RECT_W_MAX 0x180
#define RECT_H_MAX (VRAM_HEIGHT - rectSpeed)

#define CLUT_SIZE_8BPP 0x100
#define CLUT_SIZE_4BPP 0x10

/*uncompressed savestate vram start offset*/
#define EPSXE_VRAM_START 0x2733DF
#define NO$PS_VRAM_START 0x289070
#define DEFAULT_FILENAME "vram.bin"

#define MAX_STR_LEN 0x100

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef struct {
	SDL_Point startCoord;
	int length;
} line;

 /*
  *GLOBALS
  */
int mode;			//current display mode
int reversedFlag;		//flag of reversed colors
int clutFlag;			//flag of clut select at indexed modes
int rectSpeed, clutSpeed;

inline int clamp(int x, int min, int max)	//for rect on-screen move
{
	return (x >= max) ? (max - 1) : ((x < min) ? min : x);
}

/*
*SDL2 cannot draw unfilled rects, so I'll make my own function:
*erase previous rect - blit surBlank at dst surface
*clamp rect coords and then draw new rect or clut line
*/
void drawRect(SDL_Surface * surBlank, SDL_Surface * sur, SDL_Rect * rect, line * clutLine)
{

	u32 *dst, *dstLo;
	int i, x, y, w, h, l, xl, yl;
	SDL_Rect rectBig;

	rect->w = clamp(rect->w, rectSpeed * 8, RECT_W_MAX + 1);
	rect->h = clamp(rect->h, rectSpeed * 8, RECT_H_MAX + 1);
	w = rect->w - 1;
	h = rect->h - 1;	//we're drawing an in-bound rect
	x = rect->x = clamp(rect->x, 0, VRAM_WIDTH - w);
	y = rect->y = clamp(rect->y, 0, VRAM_HEIGHT - h);

	rectBig.x = x - rectSpeed;
	rectBig.y = y - rectSpeed;
	rectBig.w = w + rectSpeed * 2 + 1;
	rectBig.h = h + rectSpeed * 2 + 1;
	/*
	 *clear previously drawn rect by a slightly larger rect
	 *that's why rect h or w cannot be equal to vram width or height
	 */

	SDL_BlitSurface(surBlank, &rectBig, sur, &rectBig);

	//clutLine
	clutSpeed = (mode == SDL_PIXELFORMAT_INDEX8) ? CLUT_SIZE_8BPP : CLUT_SIZE_4BPP;
	l = clutLine->length;
	xl = clutLine->startCoord.x = clamp((clutLine->startCoord.x & ~(clutSpeed - 1)), 0, VRAM_WIDTH - l + 1);
	yl = clutLine->startCoord.y = clamp(clutLine->startCoord.y, 0, VRAM_HEIGHT);
	//clear clutLine
	rectBig.x = xl - clutSpeed;
	rectBig.y = yl - rectSpeed;
	rectBig.w = CLUT_SIZE_8BPP + clutSpeed * 2 + 1;	//to overwrite 8bpp long clutLine
	rectBig.h = rectSpeed * 2 + 1;
	SDL_BlitSurface(surBlank, &rectBig, sur, &rectBig);

	//2 horizontal lines

	dst = (u32 *) sur->pixels + y * VRAM_WIDTH + x;
	dstLo = dst + h * VRAM_WIDTH;

	for (i = 0; i < w; i++) {
		*dst++ = *dstLo++ = CYAN_PIXEL;
	}

	//2 vertical lines
	dst = (u32 *) sur->pixels + y * VRAM_WIDTH + x;
	dstLo = dst + w;
	for (i = 0; i <= h; i++) {

		*dst = *dstLo = CYAN_PIXEL;
		dst += VRAM_WIDTH;
		dstLo += VRAM_WIDTH;
	}

	//draw CLUT line
	if (clutFlag != 0) {
		dst = (u32 *) sur->pixels + yl * VRAM_WIDTH + xl;
		for (i = 0; i < l; i++) {
			*dst++ = MAGENTA_PIXEL;
		}
	}
}

/*
*Draws rectangle contents of buffer to surface
*and then blits it to dest window surface.
*used at indexed modes - window is too large to fit monitor
*/
void drawInbound(SDL_Surface * sur, SDL_Window * win, SDL_Surface * winSur, u16 * pInBuffer, SDL_Rect * rect)
{

	u8 *src, *srcTmp, *dst;
	int i, j, x, y, w, h;

	dst = (u8 *) sur->pixels;	//dst must be an 8-bit surface
	w = rect->w;		//optimize
	h = rect->h;
	x = rect->x;
	y = rect->y;

	src = (u8 *) pInBuffer + (x + y * VRAM_WIDTH) * 2;
	if (mode == SDL_PIXELFORMAT_INDEX8) {
		for (i = 0; i < h; i++) {	//copy scanline from buffer. Fast!
			memcpy(dst, src, w * 2);
			src += VRAM_WIDTH * 2;
			dst += w * 2;
		}
		SDL_SetWindowSize(win, w * 2, h);
	} else if (mode == SDL_PIXELFORMAT_INDEX4MSB) {
		/*
		 *Bit   |F-C|B-8|7-4|3-0|
		 *desc. |I3 |I2 |I1 |I0 |
		 *0 is drawn to the left
		 */

		for (i = 0; i < h; i++) {
			srcTmp = src;
			for (j = 0; j < w; j++) {
				*dst++ = *srcTmp & 0xF;
				*dst++ = *srcTmp++ >> 4;
				*dst++ = *srcTmp & 0xF;	//unroll a little
				*dst++ = *srcTmp++ >> 4;
			}
			src += VRAM_WIDTH * 2;
		}
		SDL_SetWindowSize(win, w * 4, h);

	}
	winSur = SDL_GetWindowSurface(win);
	SDL_BlitSurface(sur, 0, winSur, 0);
}

void updateTitle(SDL_Window * win, SDL_Window * win2, SDL_Rect * rect, line * clutLine)
{
	char strBuf[MAX_STR_LEN] = "";
	char clutStrBuf[MAX_STR_LEN / 2] = "";	//for clut pos information
	char strBuf2[MAX_STR_LEN] = "";

	switch (mode) {		//first mandatory bitmode:
	case SDL_PIXELFORMAT_INDEX4MSB:
		strcpy(strBuf2, "4 BPP mode ");
		break;
	case SDL_PIXELFORMAT_INDEX8:
		strcpy(strBuf2, "8 BPP mode ");
		break;
	case SDL_PIXELFORMAT_ABGR1555:
		strcpy(strBuf2, "15 BPP mode ");
		break;
	case SDL_PIXELFORMAT_BGR888:
		strcpy(strBuf2, "24 BPP (MDEC) mode ");
		break;
	}
	//now optional clut mode
	if ((mode == SDL_PIXELFORMAT_INDEX4MSB) || (mode == SDL_PIXELFORMAT_INDEX8)) {
		if (clutFlag != 0) {
			strcat(strBuf2, "CLUT ");
			sprintf(clutStrBuf, "CLUT(X:%03d, Y:%03d, Offset:0x%05X)", clutLine->startCoord.x, clutLine->startCoord.y, (clutLine->startCoord.x + clutLine->startCoord.y * VRAM_WIDTH) * 2);
		} else {
			strcat(strBuf2, "grayscale ");
			if (reversedFlag != 0) {
				strcat(strBuf2, "reversed ");
			}
		}
	}
	//if it's not a clut mode clutStrBuf will be empty
	sprintf(strBuf, "(X:%03d, Y:%03d, Offset:0x%05X) [W:%03d, H:%03d] %s", rect->x, rect->y, (rect->x + rect->y * VRAM_WIDTH) * 2, rect->w, rect->h, clutStrBuf);

	SDL_SetWindowTitle(win, strBuf);
	SDL_SetWindowTitle(win2, strBuf2);
}

/*
*Updates pal on each clutline move or clut mode change
*color reverse is also here.
*/

void updatePal(SDL_Surface * sur, line * clutLine, SDL_Surface * surOut)
{
	SDL_Color colors[CLUT_SIZE_8BPP];	//heap var. Pal is clean each time.
	u16 *srcColor;
	int i, length, step;

	length = clutLine->length = (mode == SDL_PIXELFORMAT_INDEX8) ? CLUT_SIZE_8BPP : CLUT_SIZE_4BPP;	//4bpp has 0x10 colors in pal, 8bpp - 0x100

	if (clutFlag == 0) {	//fill greyscale values
		//4bpp will step 0x10 each time, not 1:
		step = (mode == SDL_PIXELFORMAT_INDEX8) ? CLUT_SIZE_8BPP / CLUT_SIZE_8BPP : CLUT_SIZE_8BPP / CLUT_SIZE_4BPP;
		if (reversedFlag != 0) {	//reversed grayscale
			for (i = 0; i < length; i++) {
				colors[i].r = colors[i].g = colors[i].b = 0xFF - i * step;
			}
		} else {	//normal grayscale
			for (i = 0; i < length; i++) {
				colors[i].r = colors[i].g = colors[i].b = i * step;
			}
		}
	} else {
		/*
		 *That's clut mode - read appropriate size clut from vram buffer
		 *and copy to pal
		 *15bpp surface as array of abgr1555 colors
		 */
		srcColor = (u16 *) sur->pixels + clutLine->startCoord.y * VRAM_WIDTH + clutLine->startCoord.x;
		//SDL_SetPaletteColors accepts 32 bit colors. Convert from u16 color and increase intensivity
		for (i = 0; i < length; i++) {
			colors[i].r = (*srcColor & 0x1F) << 3;
			colors[i].g = ((*srcColor & 0x3E0) >> 5) << 3;
			colors[i].b = ((*srcColor++ & 0x7C00) >> 10) << 3;
		}
	}
	SDL_SetPaletteColors(surOut->format->palette, colors, 0, length);

}

/*
*search for vram.bin, then for command-line filename 
*and if not found, wait for drag-n-drop file
*/

int getFileName(char *fileName, SDL_Window * window, int argc, char *argv[])
{
	SDL_Event event;
	SDL_Surface *winSur;
	int running = 1;

	if ((argc == 2) && (access(argv[1], F_OK) != -1)) {
		strcpy(fileName, argv[1]);
		return 1;
	}

	if (access(DEFAULT_FILENAME, F_OK) != -1) {
		strcpy(fileName, DEFAULT_FILENAME);
		return 1;
	}

	winSur = SDL_GetWindowSurface(window);	//so window's black content will be drawn
	SDL_SetWindowTitle(window, "Drag and drop input file");
	while (running) {
		SDL_WaitEvent(&event);
		if (event.type == SDL_DROPFILE) {
			strcpy(fileName, event.drop.file);
			free(event.drop.file);
			SDL_SetWindowTitle(window, fileName);
			break;
		} else if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				running = 0;
			}
		}
		SDL_UpdateWindowSurface(window);
	}
	SDL_FreeSurface(winSur);
	return running;
}

void showMouseCoords(int x, int y, SDL_Rect * rect, SDL_Window * win)
{
	char strBuf[MAX_STR_LEN] = "";

	switch (mode) {		//calculate global mouse coords based on relative coords and rect position
	case SDL_PIXELFORMAT_INDEX4MSB:
		x = rect->x + (x / (sizeof(u16) * 2));	//half a byte per pixel
		y += rect->y;
		break;
	case SDL_PIXELFORMAT_INDEX8:
		x = rect->x + (x / sizeof(u16));
		y += rect->y;
		break;
	case SDL_PIXELFORMAT_BGR888:
		x = (x * 3) / sizeof(u16);	//3 bytes per pixel
		break;
	}
	sprintf(strBuf, "X:%03d, Y:%03d, Offset:0x%05X", x, y, (x + y * VRAM_WIDTH) * sizeof(u16));
	SDL_SetWindowTitle(win, strBuf);
}

int main(int argc, char *argv[])
{
	SDL_Surface *sur15, *sur24, *sur8, *sur4, *winSur, *winSur2;
	SDL_Window *window, *window2;
	SDL_Event event;
	int running = 1, offset = 0;
	int x, y;
	char fileName[MAX_STR_LEN], hdrStr[5];
	FILE *fIn;
	SDL_Rect rect = { RECT_X_INIT, RECT_Y_INIT, RECT_W_INIT, RECT_H_INIT };
	line clutLine = { {RECT_X_INIT, RECT_Y_INIT}, 0 };
	u32 *pInBuffer24;
	u16 *pInBuffer;
	u32 *pInBufferIterator;

	reversedFlag = 0;
	clutFlag = 0;
	clutSpeed = 0x10;
	mode = SDL_PIXELFORMAT_ABGR1555;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		//printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow("VRAM overview", SDL_WINDOWPOS_UNDEFINED, 32, VRAM_WIDTH, VRAM_HEIGHT, SDL_WINDOW_SHOWN);
	winSur = SDL_GetWindowSurface(window);

	if (getFileName(fileName, window, argc, argv) == 0) {
		SDL_DestroyWindow(window);
		return -1;
	}
	//mode view window:     
	SDL_GetWindowPosition(window, &x, &y);
	window2 = SDL_CreateWindow("Mode viewer", SDL_WINDOWPOS_UNDEFINED, y + VRAM_HEIGHT, VRAM_WIDTH, VRAM_HEIGHT, SDL_WINDOW_SHOWN);
	winSur2 = SDL_GetWindowSurface(window2);

	pInBuffer = (u16 *) malloc(VRAM_WIDTH * VRAM_HEIGHT * sizeof(u16));
	pInBuffer24 = malloc(VRAM_WIDTH_24BPP * VRAM_HEIGHT * sizeof(u32));
	sur8 = SDL_CreateRGBSurface(0, rect.w * 2, rect.h, 8, 0, 0, 0, 0);
	/*
	 *SDL cannot blit 4bpp on truecolor surface
	 *so 4bpp mode will be converted to 8bpp surface by     
	 *drawInbound and then should be treated as an 8bit surface
	 */
	sur4 = SDL_CreateRGBSurface(0, rect.w * 4, rect.h, 8, 0, 0, 0, 0);

readFile:
	if ((fIn = fopen(fileName, "rb")) == NULL) {
		SDL_Quit();
		free(pInBuffer);
		free(pInBuffer24);
		SDL_DestroyWindow(window);
		SDL_DestroyWindow(window2);
		SDL_FreeSurface(sur4);
		SDL_FreeSurface(sur8);
		return 1;
	}
	fread(hdrStr, sizeof(char), 5, fIn);
	if (strncmp(hdrStr, "ePSXe", 5) == 0)
		offset = EPSXE_VRAM_START;
	if (strncmp(hdrStr, "NO$PS", 5) == 0)
		offset = NO$PS_VRAM_START;
	fseek(fIn, offset, SEEK_SET);
	fread(pInBuffer, sizeof(u16), VRAM_WIDTH * VRAM_HEIGHT, fIn);
	fclose(fIn);
	//overview window       
	sur15 = SDL_CreateRGBSurfaceFrom((u8 *) pInBuffer, VRAM_WIDTH, VRAM_HEIGHT, 16, VRAM_WIDTH * sizeof(u16), 0x1F, 0x3E0, 0x7c00, 0x0);
	SDL_BlitSurface(sur15, 0, winSur, 0);
	drawRect(sur15, winSur, &rect, &clutLine);
	//SDL_UpdateWindowSurface(window);

	//PSX reads 24 bpp info at the same pitch as 15 bpp, so last 2 bytes of each scanline cannot be rendered in 24 bpp mode.
	//We'll prepare buffer for 24bpp mode once:

	pInBufferIterator = pInBuffer24;
	for (y = 0, offset = 0; y < VRAM_HEIGHT; y++) {
		for (x = 0; x < VRAM_WIDTH_24BPP; x++) {

			*(pInBufferIterator++) = (*(u32 *) ((u8 *) pInBuffer + offset));
			offset += 3;
		}
		offset = y * VRAM_WIDTH * sizeof(u16);
	}
	sur24 = SDL_CreateRGBSurfaceFrom((u8 *) pInBuffer24, VRAM_WIDTH_24BPP, VRAM_HEIGHT, 32, VRAM_WIDTH_24BPP * 4, 0xFF, 0xFF00, 0xFF0000, 0x0);

	switch (mode) {
	case SDL_PIXELFORMAT_INDEX4MSB:
		updatePal(sur15, &clutLine, sur4);
		drawInbound(sur4, window2, winSur2, pInBuffer, &rect);
		break;
	case SDL_PIXELFORMAT_INDEX8:
		updatePal(sur15, &clutLine, sur8);
		drawInbound(sur8, window2, winSur2, pInBuffer, &rect);
		break;
	case SDL_PIXELFORMAT_ABGR1555:
		SDL_BlitSurface(sur15, 0, winSur2, 0);
		break;
	case SDL_PIXELFORMAT_BGR888:
		SDL_BlitSurface(sur24, 0, winSur2, 0);
		break;
	}

	//Keep looping until the user closes the SDL window
	while (running) {
		SDL_UpdateWindowSurface(window);
		SDL_UpdateWindowSurface(window2);
		SDL_WaitEvent(&event);

		switch (event.type) {

		case SDL_KEYDOWN:
			/*
			 *esc to exit
			 *~ switches reverse mode
			 *wsad moves rect upDownLeftRight moves clutLine
			 *ctrl switches rect slow move mode. Just do not step 1px at width/height change
			 *shift changes width/height of rect
			 *enter reloads source dump
			 */
			if ((event.key.keysym.mod & KMOD_CTRL) != 0 && ((event.key.keysym.mod & KMOD_SHIFT) == 0)) {
				rectSpeed = 1;
			} else
				rectSpeed = 8;

			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				running = 0;
				break;

			case SDLK_d:	//move rect                         
				if (event.key.keysym.mod & KMOD_SHIFT) {
					rect.w += rectSpeed;
				} else {
					rect.x += rectSpeed;
				}
				break;

			case SDLK_a:
				if (event.key.keysym.mod & KMOD_SHIFT) {
					rect.w -= rectSpeed;
				} else {
					rect.x -= rectSpeed;
				}
				break;

			case SDLK_w:
				if (event.key.keysym.mod & KMOD_SHIFT) {
					rect.h -= rectSpeed;
				} else {
					rect.y -= rectSpeed;
				}
				break;

			case SDLK_s:
				if (event.key.keysym.mod & KMOD_SHIFT) {
					rect.h += rectSpeed;
				} else {
					rect.y += rectSpeed;
				}
				break;
			case SDLK_BACKQUOTE:	//reverse colors
				if (mode == SDL_PIXELFORMAT_INDEX4MSB) {
					reversedFlag = !reversedFlag;
					updatePal(sur15, &clutLine, sur4);
				} else if (mode == SDL_PIXELFORMAT_INDEX8) {
					reversedFlag = !reversedFlag;
					updatePal(sur15, &clutLine, sur8);
				}
				updateTitle(window, window2, &rect, &clutLine);
				break;

			case SDLK_UP:	//move clutLine
				clutLine.startCoord.y -= rectSpeed;
				break;
			case SDLK_DOWN:
				clutLine.startCoord.y += rectSpeed;
				break;
			case SDLK_LEFT:
				clutLine.startCoord.x -= clutSpeed;
				//align position GPU can address only full clut coords, which lessens possible clut positions
				break;
			case SDLK_RIGHT:
				clutLine.startCoord.x += clutSpeed;
				break;
			}

			drawRect(sur15, winSur, &rect, &clutLine);

			if (mode == SDL_PIXELFORMAT_INDEX8) {
				if (event.key.keysym.mod & KMOD_SHIFT) {
					SDL_FreeSurface(sur8);
					sur8 = SDL_CreateRGBSurface(0, rect.w * 2, rect.h, 8, 0, 0, 0, 0);	//recreate surface with new size
				}
				updatePal(sur15, &clutLine, sur8);
				drawInbound(sur8, window2, winSur2, pInBuffer, &rect);
			} else if (mode == SDL_PIXELFORMAT_INDEX4MSB) {

				if (event.key.keysym.mod & KMOD_SHIFT) {
					SDL_FreeSurface(sur4);
					sur4 = SDL_CreateRGBSurface(0, rect.w * 4, rect.h, 8, 0, 0, 0, 0);
				}
				updatePal(sur15, &clutLine, sur4);
				drawInbound(sur4, window2, winSur2, pInBuffer, &rect);
			}
			break;

		case SDL_KEYUP:
			{
				switch (event.key.keysym.sym) {
				case SDLK_0:
					clutFlag = 0;
					if (mode != SDL_PIXELFORMAT_BGR888) {
						mode = SDL_PIXELFORMAT_BGR888;

						SDL_SetWindowSize(window2, VRAM_WIDTH_24BPP, VRAM_HEIGHT);
						winSur2 = SDL_GetWindowSurface(window2);
						SDL_BlitSurface(sur24, 0, winSur2, 0);
						mode = SDL_PIXELFORMAT_BGR888;
					}
					break;

				case SDLK_9:
					clutFlag = 0;
					if (mode != SDL_PIXELFORMAT_ABGR1555) {
						mode = SDL_PIXELFORMAT_ABGR1555;

						SDL_SetWindowSize(window2, VRAM_WIDTH, VRAM_HEIGHT);
						winSur2 = SDL_GetWindowSurface(window2);
						SDL_BlitSurface(sur15, 0, winSur2, 0);
						mode = SDL_PIXELFORMAT_ABGR1555;
					}
					break;

				case SDLK_2:
					mode = SDL_PIXELFORMAT_INDEX8;
					clutFlag = (event.key.keysym.mod & KMOD_SHIFT) ? 1 : 0;	//shift key switches clut select mode
					SDL_FreeSurface(sur8);
					sur8 = SDL_CreateRGBSurface(0, rect.w * 2, rect.h, 8, 0, 0, 0, 0);	//recreate surface with new size                                        
					updatePal(sur15, &clutLine, sur8);
					drawRect(sur15, winSur, &rect, &clutLine);
					drawInbound(sur8, window2, winSur2, pInBuffer, &rect);

					break;

				case SDLK_1:
					mode = SDL_PIXELFORMAT_INDEX4MSB;
					clutFlag = (event.key.keysym.mod & KMOD_SHIFT) ? 1 : 0;
					SDL_FreeSurface(sur4);
					sur4 = SDL_CreateRGBSurface(0, rect.w * 4, rect.h, 8, 0, 0, 0, 0);

					updatePal(sur15, &clutLine, sur4);
					drawRect(sur15, winSur, &rect, &clutLine);
					drawInbound(sur4, window2, winSur2, pInBuffer, &rect);
					break;
				case SDLK_RETURN:
					SDL_FreeSurface(sur15);
					SDL_FreeSurface(sur24);	//these are created from reread buffers
					goto readFile;	//sorry, but that's the most straightforward way
					break;
				}
				updateTitle(window, window2, &rect, &clutLine);
				break;

			}

		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				running = 0;
			}
			break;

		case SDL_MOUSEMOTION:
			if ((event.motion.windowID == 2) && (SDL_GetModState() & KMOD_CTRL)) {	//show coords at any mouse movement and ctrl is pressed
				showMouseCoords(event.motion.x, event.motion.y, &rect, window2);
			}
			break;

		}
	}
	SDL_Quit();
	free(pInBuffer);
	free(pInBuffer24);
	SDL_DestroyWindow(window);
	SDL_DestroyWindow(window2);
	SDL_FreeSurface(sur4);
	SDL_FreeSurface(sur8);
	SDL_FreeSurface(sur15);
	SDL_FreeSurface(sur24);

	return 0;
}
