#include <cassert>
#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>

#include "errors.hpp"
#include "resource.hpp"
#include "ziploader.hpp"

using namespace std;

struct gstate_t {
    int width;
    int height;
    SDL_Window *win;
    SDL_Renderer *ren;
};

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

static SDL_Texture *createTextureMem(gstate_t *gstate,
                                     const vector<unsigned char> &data,
                                     size_t size)
{
    SDL_RWops *ro_ops = SDL_RWFromConstMem(&(data[0]), size);
    if (ro_ops == nullptr) {
        logSDLError("SDL_RWops");
        return nullptr;
    }
    SDL_Texture *tex = IMG_LoadTexture_RW(gstate->ren, ro_ops, 0);
    SDL_FreeRW(ro_ops);
    if (tex == nullptr) {
        logSDLError("IMG_LoadTexture_RW");
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

    /* Load Background from ZIP */
    ZipFile zip;

    string resPath = getResourcePath();
    if (zip.Init(resPath + "ziptest.zip")) {
        log("ZipFile parsed correctly");
    } else {
        SDL_DestroyTexture(image);
        return -1;
    }
    vector<unsigned char> data;
    size_t bytes = 0;

    string filename("resources/Test3/image.png");
    if (!zip.Find(filename, data, &bytes))
    {
        logError("XXX", filename + " is missing !");
        SDL_DestroyTexture(image);
        return -1;
    }

    SDL_Texture *image2 = createTextureMem(gstate, data, bytes);
    if (image2 == nullptr) {
        logError("XXX", "Can't create from memory");
        SDL_DestroyTexture(image);
        return -1;
    }
    data.clear();

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
        renderTexture(gstate, image, 0, 0);
        renderTexture(gstate, image2, 200, 200);

        /* End of main drawing code */

        SDL_RenderPresent(gstate->ren);
    }

    SDL_DestroyTexture(image);
    SDL_DestroyTexture(image2);

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

    int ret;
    try {
        ret = run(&gstate);

        if (gstate.ren != nullptr)
            SDL_DestroyRenderer(gstate.ren);
        if (gstate.win != nullptr)
            SDL_DestroyWindow(gstate.win);
    } catch (bad_alloc &e) {
        cerr << "Some 'new' allocation failed: " << e.what() << endl;
        return -2;
    }
    IMG_Quit();
    SDL_Quit();

    return ret;
}
