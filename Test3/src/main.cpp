#include <cassert>
#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>

#include "errors.hpp"
#include "resource.hpp"

using namespace std;

struct gstate_t {
    int width;
    int height;
    SDL_Window *win;
    SDL_Renderer *ren;
};

#define TILE_SIZE   40

static int createWindow(gstate_t *gstate, const char* name,
                        Uint32 renderer_flags,
                        size_t posx = 100, size_t posy =  100,
                        Uint32 window_flags = SDL_WINDOW_SHOWN)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logSDLError("SDL_Init");
        return -1;
    }

    const int width = gstate->width;
    const int height = gstate->height;

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

    /* Loads BMP by default, adding PNG support */
    int mask = IMG_Init(IMG_INIT_PNG);
    if ((mask & IMG_INIT_PNG) == 0) {
        logIMGError("IMG_Init");
        return -1;
    }

    return 0;
}

static SDL_Texture *createTexture(gstate_t *gstate, const string &path)
{
    SDL_Texture *tex = IMG_LoadTexture(gstate->ren, path.c_str());
    if (tex == nullptr) {
        logSDLError("IMG_LoadTexture");
    }
    return tex;
}

static void renderTexture(gstate_t *gstate, SDL_Texture *tex, SDL_Rect *dst,
                          SDL_Rect *clip)
{
    SDL_RenderCopy(gstate->ren, tex, clip, dst);
}

static void renderTexture(gstate_t *gstate, SDL_Texture *tex, int x, int y,
                          SDL_Rect *clip = nullptr)
{
    SDL_Rect dst = {
        .x = x,
        .y = y,
        .w = 0,
        .h = 0
    };

    if (clip != nullptr) {
        dst.w = clip->w;
        dst.h = clip->h;
    } else {
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    }
    renderTexture(gstate, tex, &dst, clip);
}

static int run(gstate_t *gstate)
{
    int ret;

    /* TODO: make pos configurable */
    ret = createWindow(gstate, "Hello World !",
                       SDL_RENDERER_ACCELERATED |
                       SDL_RENDERER_PRESENTVSYNC);
    if (ret != 0)
        return ret;

    /* Load Background */
    string imagePath = getResourcePath("Test3");
    SDL_Texture *image = createTexture(gstate, imagePath + "image.png");
    if (image == nullptr) {
        return -1;
    }

    int image_w, image_h;
    SDL_QueryTexture(image, NULL, NULL, &image_w, &image_h);
    assert(image_w == 200);
    assert(image_h == 200);

    int tile_w = image_w / 2;
    int tile_h = image_h / 2;

    int pos_x = (gstate->width - tile_w) / 2;
    int pos_y = (gstate->height - tile_h) / 2;

    SDL_Rect clip[4];
    for (int i = 0; i < 4; i++) {
        clip[i].x = i / 2 * tile_w;
        clip[i].y = i % 2 * tile_h;
        clip[i].w = tile_w;
        clip[i].h = tile_h;
    }

    int current_clip = 0;
    SDL_Event event;

    while (true) {
        /* Event management */
        bool done = false;
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    done = true;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = true;
                    }
                    current_clip = (current_clip + 1) %4;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    current_clip = (current_clip + 1) %4;
                    break;
            }
            if (done)
                break;
        }
        if (done)
            break;

        /* Render stuff */
        SDL_RenderClear(gstate->ren);

        /* Main drawing code */
        renderTexture(gstate, image, pos_x, pos_y, &clip[current_clip]);

        /* End of main drawing code */

        SDL_RenderPresent(gstate->ren);
    }

    SDL_DestroyTexture(image);

    return 0;
}

static gstate_t gstate;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    gstate.win = nullptr;
    gstate.ren = nullptr;
    gstate.width = 640;
    gstate.height = 480;

    int ret = run(&gstate);

    if (gstate.ren != nullptr)
        SDL_DestroyRenderer(gstate.ren);
    if (gstate.win != nullptr)
        SDL_DestroyWindow(gstate.win);

    IMG_Quit();
    SDL_Quit();

    return ret;
}
