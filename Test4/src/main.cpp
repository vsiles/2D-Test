#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

#include "errors.hpp"
#include "resource.hpp"

using namespace std;

struct gstate_t {
    SDL_Window *win;
    SDL_Renderer *ren;
};

#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768

static int createWindow(gstate_t *gstate, const char* name,
                        Uint32 renderer_flags,
                        size_t posx = 100, size_t posy =  100,
                        size_t width = SCREEN_WIDTH, size_t height = SCREEN_HEIGHT,
                        Uint32 window_flags = SDL_WINDOW_SHOWN)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logSDLError("SDL_Init");
        return -1;
    }

    gstate->win = SDL_CreateWindow(name, posx, posy, width, height,
                                   window_flags);
    if (gstate->win == nullptr) {
        logSDLError("SDL_CreateWindow");
        return -1;
    }

    gstate->ren = SDL_CreateRenderer(gstate->win, -1, renderer_flags);
    if (gstate->ren == nullptr) {
        logSDLError("SDL_CreateRenderer");
        return -1;
    }

    if (TTF_Init() != 0){
        logTTFError("TTF_Init");
	return -1;
    }

    return 0;
}

enum texture_kind {
    TEXTURE_BMP = 0,
};

static SDL_Texture *createTexture(gstate_t *gstate, const char *path,
                                  texture_kind kind = TEXTURE_BMP)
{
    SDL_Surface *surf = nullptr;

    switch (kind) {
        case TEXTURE_BMP:
            surf = SDL_LoadBMP(path);
            break;
        default:
            logError("XXX", "Unsupported texture format");
            return nullptr;
    }

    if (surf == nullptr) {
        string msg = "createTexture loading - ";
        msg = msg + path;
        logSDLError(msg.c_str());
        return nullptr;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(gstate->ren, surf);
    SDL_FreeSurface(surf);

    if (tex == nullptr) {
        string msg = "createTexture to surface - ";
        msg = msg + path;
        logSDLError(msg.c_str());
        return nullptr;
    }

    return tex;
}

static void renderTexture(gstate_t *gstate, SDL_Texture *tex, int x, int y)
{
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;

    /* TODO Optimize this */
    SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(gstate->ren, tex, NULL, &dst);
}

enum TextStyle {
    TS_SOLID = 0,
    TS_SHADED,
    TS_BLENDED
};

SDL_Texture* renderText(gstate_t *gstate, const string &message,
                        const string &fontFile, SDL_Color color, int fontSize,
                        TextStyle style = TS_SOLID)
{

    TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if (font == nullptr){
        logSDLError("TTF_OpenFont");
        return nullptr;
    }

    SDL_Surface *surf;
    SDL_Color bg = { 0, 0, 0, 255 };

    switch (style) {
        case TS_SOLID:
            surf = TTF_RenderText_Solid(font, message.c_str(), color);
            break;
        case TS_SHADED:
            surf = TTF_RenderText_Shaded(font, message.c_str(), color, bg);
            break;
        case TS_BLENDED:
            surf = TTF_RenderText_Blended(font, message.c_str(), color);
            break;
    }

    if (surf == nullptr){
        TTF_CloseFont(font);
        logSDLError("TTF_RenderText");
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(gstate->ren, surf);
    if (texture == nullptr){
        logSDLError("CreateTexture");
    }

    SDL_FreeSurface(surf);
    TTF_CloseFont(font);
    return texture;
}

static int run(gstate_t *gstate)
{
    int ret;

    /* TODO: make pos & size configurable */
    ret = createWindow(gstate, "Hello World !",
                       SDL_RENDERER_ACCELERATED |
                       SDL_RENDERER_PRESENTVSYNC);
    if (ret != 0)
        return ret;

    const string resPath = getResourcePath("fonts");
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Texture *image[3];

    image[0] = renderText(gstate, "TTF fonts are cool !",
                          resPath + "legos.ttf", color, 50,
                          TS_SOLID);
    image[1] = renderText(gstate, "TTF fonts are cool !",
                          resPath + "legos.ttf", color, 50,
                          TS_SHADED);
    image[2] = renderText(gstate, "TTF fonts are cool !",
                          resPath + "legos.ttf", color, 50,
                          TS_BLENDED);

    if (image[0] == nullptr) {
        return -1;
    }
    if (image[1] == nullptr) {
        return -1;
    }
    if (image[2] == nullptr) {
        return -1;
    }

    int width, height;
    SDL_QueryTexture(image[0], NULL, NULL, &width, &height);
    int x = (SCREEN_WIDTH - width) / 2;
    int y = (SCREEN_HEIGHT - height) / 2;

    while (true) {
        /* Event management */
        SDL_Event event;
        bool done = false;
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                case SDL_KEYDOWN:
                case SDL_MOUSEBUTTONDOWN:
                    done = true;
                    break;
            }
            if (done)
                break;
        }
        if (done)
            break;

        SDL_RenderClear(gstate->ren);
        renderTexture(gstate, image[0], x, y - 120);
        renderTexture(gstate, image[1], x, y - 60);
        renderTexture(gstate, image[2], x, y);
        SDL_RenderPresent(gstate->ren);
    }

    SDL_DestroyTexture(image[0]);
    SDL_DestroyTexture(image[1]);
    SDL_DestroyTexture(image[2]);

    return 0;
}

static gstate_t gstate;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    gstate.win = nullptr;
    gstate.ren = nullptr;

    int ret = run(&gstate);

    if (gstate.ren != nullptr)
        SDL_DestroyRenderer(gstate.ren);
    if (gstate.win != nullptr)
        SDL_DestroyWindow(gstate.win);

    TTF_Quit();
    SDL_Quit();

    return ret;
}
