#pragma once

// screen geometry

#define SCR_W 1280
#define SCR_H 720
#define SCR_CX 640
#define SCR_CY 360

// R_Print flags

#define P_XCENTER 0x01
#define P_YCENTER 0x02
#define P_ARIGHT  0x04
#define P_ABOTTOM 0x08
#define P_BOLD    0x10

// colors

#define RGBA8(r,g,b,a) (((r)&0xff)|(((g)&0xff)<<8)|(((b)&0xff)<<16)|(((a)&0xff)<<24))

static const unsigned C_RED = RGBA8(255, 0, 0, 255);
static const unsigned C_WHITE = RGBA8(255, 255, 255, 255);
static const unsigned C_LTGREY = RGBA8(192, 192, 192, 255);
static const unsigned C_GREY = RGBA8(128, 128, 128, 255);
static const unsigned C_DKGREY = RGBA8(80, 80, 80, 255);
static const unsigned C_BLACK = RGBA8(0, 0, 0, 255);
static const unsigned C_GREEN = RGBA8(0, 255, 0, 255);
static const unsigned C_YELLOW = RGBA8(255, 255, 0, 255);
static const unsigned C_ORANGE = RGBA8(255, 180, 0, 255);
static const unsigned C_BROWN = RGBA8(80, 64, 25, 225);

// 

int R_Init(void);
void R_Free(void);

void R_BeginDrawing(void);
void R_EndDrawing(void);

void R_Clear(unsigned c);

void R_PrintScaled(int flags, int x, int y, float s, unsigned c, const char *fmt, ...);
#define R_Print(f, x, y, c, ...) R_PrintScaled((f), (x), (y), 1.f, (c), __VA_ARGS__)

void R_DrawButton(int x, int y, unsigned c, int button);
void R_DrawRect(int x, int y, int w, int h, unsigned c);
void R_DrawFrame(int x1, int y1, int x2, int y2, int w, unsigned c);
void R_DrawLine(int x1, int y1, int x2, int y2, unsigned c);
