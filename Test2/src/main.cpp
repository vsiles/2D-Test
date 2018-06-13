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

static void renderTexture(gstate_t *gstate, SDL_Texture *tex, int x, int y,
                          int w = 0, int h = 0)
{
    SDL_Rect dst;
    int width = w, height = h;
    dst.x = x;
    dst.y = y;

    if ((w == 0) || (h == 0)) {
        /* TODO Optimize this */
        SDL_QueryTexture(tex, NULL, NULL, &width, &height);
        if (w != 0)
            width = w;
        if (h != 0)
            height = w;
    }
    dst.w = width;
    dst.h = height;

    SDL_RenderCopy(gstate->ren, tex, NULL, &dst);
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
    string imagePath = getResourcePath("Test2");
    SDL_Texture *background = createTexture(gstate, imagePath + "background.png");
    if (background == nullptr)
        return -1;
    SDL_Texture *image = createTexture(gstate, imagePath + "image.png");
    if (image == nullptr) {
        /* TODO: resource manager */
        SDL_DestroyTexture(background);
        return -1;
    }

    int nr_tiles_x = gstate->width / TILE_SIZE;
    int nr_tiles_y = gstate->height / TILE_SIZE;

    SDL_Event event;

    while (true) {
        /* Event management */
        bool done = false;
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                case SDL_KEYDOWN:
                case SDL_MOUSEBUTTONDOWN:
                    done = true;
                    break;
                default:
                    cout << "Event: " << event.type << endl;
            }
            if (done)
                break;
        }
        if (done)
            break;

        /* Render stuff */
        SDL_RenderClear(gstate->ren);

        /* Main drawing code */
        for (int i = 0; i < nr_tiles_x; i++) {
            for (int j = 0; j < nr_tiles_y; j++) {
                renderTexture(gstate, background, i * TILE_SIZE, j * TILE_SIZE,
                              TILE_SIZE, TILE_SIZE);
            }
        }

        /* TODO: optimize this */
        int iW, iH;
        SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
        int x = gstate->width / 2 - iW / 2;
        int y = gstate->height / 2 - iH / 2;
        renderTexture(gstate, image, x, y);
        /* End of main drawing code */

        SDL_RenderPresent(gstate->ren);
    }

    SDL_DestroyTexture(image);
    SDL_DestroyTexture(background);

    /* Exit */
    log("Success !");
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
