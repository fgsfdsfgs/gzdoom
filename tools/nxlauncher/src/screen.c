#include "config.h"
#include "screen.h"
#include "input.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <SDL2/SDL.h>

#define R(c) (c & 0xff)
#define G(c) ((c >> 8) & 0xff)
#define B(c) ((c >> 16) & 0xff)
#define A(c) ((c >> 24) & 0xff)

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;

struct fnt
{
    int ch_w;
    int ch_h;
    int start;
    SDL_Surface *surf;
    SDL_Texture *tex;
};

static struct fnt fnt_main = { 16, 16, 0 };
static struct fnt fnt_buttons = { 16, 16, 48 };

static int drawing = 0;

int R_Init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        R_Free();
        return -1;
    }

    win = SDL_CreateWindow("", 0, 0, 1280, 720, 0);
    if (!win)
    {
        R_Free();
        return -2;
    }

    ren = SDL_CreateRenderer(win, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        R_Free();
        return -3;
    }

    fnt_main.surf = SDL_LoadBMP("romfs:/fnt_main.bmp");
    fnt_buttons.surf = SDL_LoadBMP("romfs:/fnt_buttons.bmp");
    if (!fnt_main.surf || !fnt_buttons.surf)
    {
        R_Free();
        return -4;
    }

    SDL_SetColorKey(fnt_main.surf, SDL_TRUE, RGBA8(0, 0, 0, 255));
    SDL_SetColorKey(fnt_buttons.surf, SDL_TRUE, RGBA8(0, 0, 0, 255));

    fnt_main.tex = SDL_CreateTextureFromSurface(ren, fnt_main.surf);
    fnt_buttons.tex = SDL_CreateTextureFromSurface(ren, fnt_buttons.surf);
    if (!fnt_main.tex || !fnt_buttons.tex)
    {
        R_Free();
        return -5;
    }

    return 0;
}

void R_Free(void)
{
    if (fnt_main.tex) SDL_DestroyTexture(fnt_main.tex);
    if (fnt_buttons.tex) SDL_DestroyTexture(fnt_buttons.tex);
    if (fnt_main.surf) SDL_FreeSurface(fnt_main.surf);
    if (fnt_buttons.surf) SDL_FreeSurface(fnt_buttons.surf);
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
}

void R_BeginDrawing(void)
{
    if (drawing) R_EndDrawing();
    drawing = 1;
}

void R_EndDrawing(void)
{
    if (drawing)
    {
        SDL_RenderPresent(ren);
        drawing = 0;
    }
}

void R_Clear(unsigned c)
{
    SDL_SetRenderDrawColor(ren, R(c), G(c), B(c), A(c));
    SDL_RenderClear(ren);
}

static inline void DrawChar(struct fnt *f, int x, int y, float s, char ch)
{
    static SDL_Rect src, dst;
    ch -= f->start;
    src.x = (ch % 16) * f->ch_w;
    src.y = (ch / 16) * f->ch_h;
    src.w = f->ch_w;
    src.h = f->ch_h;
    dst.x = x;
    dst.y = y;
    dst.w = s * src.w;
    dst.h = s * src.h;
    SDL_RenderCopy(ren, f->tex, &src, &dst);
}

static void DrawText(struct fnt *f, int x, int y, float s, unsigned c, char *buf)
{
    char *p = buf;
    int cx = x, cy = y;
    int cw = f->ch_w * s, ch = f->ch_h * s;
    SDL_SetRenderDrawColor(ren, R(c), G(c), B(c), A(c));
    SDL_SetTextureColorMod(f->tex, R(c), G(c), B(c));
    SDL_SetTextureAlphaMod(f->tex, A(c));
    while (p && *p)
    {
        if (*p == '\n')
        {
            cy += ch;
            cx = x;
            p++;
            continue;
        }
        DrawChar(f, cx, cy, s, *p);
        cx += cw;
        p++;
    }
}

static inline void TextSize(struct fnt *f, float s, char *buf, int *out_w, int *out_h)
{
    char *p = buf;
    float cw = (float)(f->ch_w) * s, ch = (float)(f->ch_h) * s;
    float cx = 0.f, cy = ch;
    float mx = 0.f;
    while (p && *p)
    {
        if (*p == '\n')
        {
            cy += ch;
            if (cx > mx) mx = cx;
            cx = 0;
            p++;
            continue;
        }
        cx += cw;
        p++;
    }
    if (cx > mx) mx = cx;
    *out_w = (int)(mx + 0.5f);
    *out_h = (int)(cy + 0.5f);
}

void R_PrintScaled(int flags, int x, int y, float s, unsigned c, const char *fmt, ...)
{
    static char buf[4096];
    int tw = 0, th = 0;

    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(buf, sizeof(buf), fmt, argptr);
    va_end(argptr);

    // don't care about performance, it's a fucking menu
    TextSize(&fnt_main, s, buf, &tw, &th);

    if (flags & P_XCENTER)
        x -= tw / 2;
    else if (flags & P_ARIGHT)
        x -= tw;

    if (flags & P_YCENTER)
        y -= th / 2;
    else if (flags & P_ABOTTOM)
        y -= th;

    DrawText(&fnt_main, x, y, s, c, buf);
}

void R_DrawButton(int x, int y, unsigned c, int button)
{
    SDL_SetRenderDrawColor(ren, R(c), G(c), B(c), A(c));
    SDL_SetTextureColorMod(fnt_buttons.tex, R(c), G(c), B(c));
    SDL_SetTextureAlphaMod(fnt_buttons.tex, A(c));
    DrawChar(&fnt_buttons, x, y, 1.f, button);
}

void R_DrawRect(int x, int y, int w, int h, unsigned c)
{
    SDL_Rect rect = { x, y, w, h };
    SDL_SetRenderDrawColor(ren, R(c), G(c), B(c), A(c));
    SDL_RenderFillRect(ren, &rect);
}

void R_DrawFrame(int x1, int y1, int x2, int y2, int w, unsigned c)
{
    SDL_Rect rect = { x1, y1, x2-x1, y2-y1 };
    SDL_SetRenderDrawColor(ren, R(c), G(c), B(c), A(c));
    SDL_RenderDrawRect(ren, &rect);
}

void R_DrawLine(int x1, int y1, int x2, int y2, unsigned c)
{
    SDL_SetRenderDrawColor(ren, R(c), G(c), B(c), A(c));
    SDL_RenderDrawLine(ren, x1, y1, x2, y2);
}
