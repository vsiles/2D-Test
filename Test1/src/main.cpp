#include <iostream>
#include <string>
#include <SDL.h>

#include "errors.hpp"
#include "resource.hpp"

using namespace std;

struct gstate_t {
    SDL_Window *win;
    SDL_Renderer *ren;
};

static int window_creation(gstate_t *gstate, const char* name,
                           Uint32 renderer_flags,
                           size_t posx = 100, size_t posy =  100,
                           size_t width = 640, size_t height = 480,
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

    return 0;
}

static int run(gstate_t *gstate)
{
    int ret;

    /* TODO: make pos & size configurable */
    ret = window_creation(gstate, "Hello World !",
                          SDL_RENDERER_ACCELERATED |
                          SDL_RENDERER_PRESENTVSYNC);
    if (ret != 0)
        return ret;

    /* Load BMP */
    string imagePath = getResourcePath("img") + "hello.bmp";
    SDL_Surface *bmp = SDL_LoadBMP(imagePath.c_str());
    if (bmp == nullptr) {
        logSDLError("SDL_LoadBMP");
	return -1;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(gstate->ren, bmp);
    SDL_FreeSurface(bmp);

    if (tex == nullptr) {
        logSDLError("SDL_CreateTextureFromSurface");
        return -1;
    }

    /* Display BMP for 3 secs */
    for (int i = 0; i < 3; ++i) {
        SDL_RenderClear(gstate->ren);
        SDL_RenderCopy(gstate->ren, tex, NULL, NULL);
        SDL_RenderPresent(gstate->ren);
        SDL_Delay(1000);
    }

    SDL_DestroyTexture(tex);

    /* Exit */
    log("Succes !");
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

    SDL_Quit();

    return ret;
}
