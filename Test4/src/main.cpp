#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include <random>

#include "errors.hpp"
#include "resource.hpp"

using namespace std;

struct gstate_t {
    SDL_Window *win;
    SDL_Renderer *ren;
};

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   800

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

__attribute__((unused))
static void print_table(const int *a)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << a[4 * i + j] << " ";
        }
        cout << endl;
    }
}

// returns the position of the hole
static int generate_perm(int *a, int n, int hole_pos)
{

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> udist(0, n - 1);

    for (int i = n - 1; i > 0; i--) {
        int j = udist(rng);
        int temp = a[i];
        a[i] = a[j];
        a[j] = temp;

        if (hole_pos == i)
            hole_pos = j;
        else if (hole_pos == j)
            hole_pos = i;
    }
    return hole_pos;
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

/* 4 neighbourhood */
static int adj[16][4] = {
    { 1, 4, -1, -1 },
    { 0, 5, 2, -1 },
    { 1, 6, 3, -1 },
    { 2, 7, -1, -1 },

    { 0, 5, 8, -1 },
    { 1, 4, 6, 9 },
    { 2, 5, 7, 10 },
    { 3, 6, 11, -1 },

    { 4, 9, 12, -1 },
    { 5, 8, 10, 13 },
    { 6, 9, 11, 14 },
    { 7, 10, 15, -1 },

    { 8, 13, -1, -1 },
    { 9, 12, 14, -1 },
    { 10, 13, 15, -1 },
    { 11, 14, -1, -1 }
};

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
    static int perm[16] = { 0 };
    int timeout = 100;


    for (int i = 0; i < 16; i++)
        perm[i] = i;

    int hole_pos = 15;

    while (timeout > 0) {
        hole_pos = generate_perm(perm, 16, hole_pos);
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
    /* cout << "Hole in position " << hole_pos << endl; */
    /* print_table(perm); */

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
        }
    }

    while (true) {
        /* Event management */
        int x, y;
        SDL_Event event;
        bool done = false;
        bool clicked = false;
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
                case SDL_MOUSEBUTTONDOWN:
                    x = event.button.x - 50;
                    y = event.button.y - 50;
                    x /= 100;
                    y /= 100;
                    if ((x % 2) == 0 && (y % 2) == 0) {
                        x /= 2;
                        y /= 2;
                        clicked = true;
                    }
                    break;
            }
            if (done)
                break;
        }
        if (done)
            break;

        bool solved = false;
        if (clicked) {
            int pos = x + 4 * y;
            int *neighb = adj[pos];
            for (int i = 0; i < 4; i++) {
                if (neighb[i] < 0)
                    break;
                if (neighb[i] == hole_pos) {
                    perm[hole_pos] = perm[pos];
                    perm[pos] = 15;
                    hole_pos = pos;

                    solved = true;
                    /* Check for success */
                    for (int z = 0; z < 16; z++) {
                        if (perm[z] != z) {
                            solved = false;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        SDL_RenderClear(gstate->ren);
        for (int j = 0; j < 4; j++) {
            for (int i = 0; i < 4; i++) {
                int ij = i + 4 * j;
                SDL_Rect box = {
                    .x = 50 + 200 * i,
                    .y = 50 + 200 * j,
                    .w = 100,
                    .h = 100
                };
                int nr = perm[ij];
                Uint8 r, g, b, a;
                SDL_GetRenderDrawColor(gstate->ren, &r, &g, &b, &a);
                if (ij == hole_pos) {
                    assert(perm[ij] == 15);
                    SDL_SetRenderDrawColor(gstate->ren, 255, 0, 0, 255);
                    SDL_RenderFillRect(gstate->ren, &box);
                }
                renderTexture(gstate, image[nr], box.x, box.y,
                              100, 100);
                SDL_SetRenderDrawColor(gstate->ren, 0, 255, 0, 255);
                SDL_RenderDrawRect(gstate->ren, &box);
                SDL_SetRenderDrawColor(gstate->ren, r, g, b, a);
            }
        }
        SDL_RenderPresent(gstate->ren);
        if (solved) {
            cout << "Solved !" << endl;
            SDL_Delay(2000);
            break;
        }
    }

    for (int z = 0; z < 16; z++)
        SDL_DestroyTexture(image[z]);

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
