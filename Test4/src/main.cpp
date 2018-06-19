#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdlib>

#include "errors.hpp"
#include "resource.hpp"

using namespace std;

struct gstate_t {
    SDL_Window *win;
    SDL_Renderer *ren;
};

#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768

static int signature(const int *a, int n)
{
    int ret = 1;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (a[j] < a[i])
                ret = -ret;
        }
    }
    return ret;
}

static int signature_hole(const int *a, int n)
{
    int pos = 0;
    while (pos < n) {
        if (a[pos] == (n-1))
            break;
        pos++;
    }
    return ((pos % 2) == 0)?1:-1;
}

static void generate_perm(int *a, int n)
{
    for (int i = n - 1; i > 0; i--) {
        float f = (float)rand();
        f = f * (float)n / RAND_MAX;
        int j = (int)f;
        int temp = a[i];
        a[i] = a[j];
        a[j] = temp;
    }
}

static int createWindow(gstate_t *gstate, const char* name,
                        Uint32 renderer_flags,
                        size_t posx = 100, size_t posy =  100,
                        size_t width = SCREEN_WIDTH,
                        size_t height = SCREEN_HEIGHT,
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

#if 0
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
#endif

static void renderTexture(gstate_t *gstate, SDL_Texture *tex, int x, int y,
                          int w, int h)
{
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;

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
    SDL_Texture *image[16];
    SDL_Rect boxes[16];
    static int perm[16] = { 0 };
    int timeout = 100;

    while (timeout > 0) {
        for (int i = 0; i < 16; i++)
            perm[i] = i;

        generate_perm(perm, 16);
        int sig1 = signature(perm, 16);
        int sig2 = signature_hole(perm, 16);
        if (sig1 == sig2)
            break;
        timeout--;
    }

    if (timeout == 0) {
        cout << "Can't generate a permutation" << endl;
        return -1;
    }

    cout << "Generated in " << 100 - timeout + 1 << " attempts." << endl;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int nr = perm[x + 4 * y];
            string text = to_string(nr);
            if (nr == 15)
                text = string(" ");
            image[nr] = renderText(gstate, text, resPath + "Calibri.ttf",
                                   color, 100, TS_SOLID);
            if (image[nr] == nullptr) {
                for (int xxx = 0; xxx < nr; xxx++)
                    SDL_DestroyTexture(image[xxx]);
                return -1;
            }
            boxes[nr].x = 50 + 200 * x;
            boxes[nr].y = 50 + 200 * y;
            boxes[nr].w = 100;
            boxes[nr].h = 100;
        }
    }

    while (true) {
        /* Event management */
        SDL_Event event;
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

        SDL_RenderClear(gstate->ren);
        for (int i = 0; i < 16; i++) {
            int nr = perm[i];
            Uint8 r, g, b, a;
            SDL_GetRenderDrawColor(gstate->ren, &r, &g, &b, &a);
            if (nr == 15) {
                SDL_SetRenderDrawColor(gstate->ren, 255, 0, 0, 255);
                SDL_RenderFillRect(gstate->ren, &(boxes[nr]));
            }
            renderTexture(gstate, image[nr], boxes[nr].x, boxes[nr].y,
                          100, 100);
            SDL_SetRenderDrawColor(gstate->ren, 0, 255, 0, 255);
            SDL_RenderDrawRect(gstate->ren, &(boxes[nr]));
            SDL_SetRenderDrawColor(gstate->ren, r, g, b, a);
        }
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
