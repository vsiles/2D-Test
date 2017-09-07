#include <iostream>
#include <fstream>
#include <cstdint>
#include <SDL2/SDL.h>
#include <zlib.h>

#include "json.hpp"
using json = nlohmann::json;

using namespace std;

static void myerror(char const *msg, SDL_Window *sdlWindow,
                    SDL_Renderer *sdlRenderer)
{
    cerr << msg << SDL_GetError() << endl;
    SDL_DestroyWindow(sdlWindow);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_Quit();
}

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

size_t nr_cells = 0;

typedef struct {
    unsigned int x, y;
} vector_t;

typedef struct {
    vector_t pos, size;
} cell_t;

extern int b64decode(const string &data, unsigned char **bytes, size_t *length);

int main(int argc, char *argv[])
{
    int ret;

    (void)argc;
    (void)argv;

    /* Load level information */
    ifstream level_info("./resources/maps/map0.json");
    if (!level_info.good()) {
        cerr << "Can't open file './resources/maps/map0.json' for reading" << endl;
        return -1;
    }

    json j;
    level_info >> j;

    json layer0 = j["layers"][0];
    const size_t width = j["width"];
    const size_t height = j["height"];

    if (width != layer0["width"] || height != layer0["height"]) {
        cerr << "Tile layer dimensions doesn't match map dimensions" << endl;
        return -1;
    }


    size_t length;
    unsigned char *bytes = NULL;

    if (layer0["encoding"] == "base64") {
        cout << "base64 encoding spotted, decoding ..." << endl;
        const string data = layer0["data"];
        ret = b64decode(data, &bytes, &length);
        if (ret) {
            cerr << "Can't decode layer0's data" << endl;
            return -1;
        }
        cout << "Decoded data. Old size was " << data.length();
        cout << " new size is " << length << endl;
    } else {
        cerr << "Unsupported map format: " << layer0["encoding"] << endl;
        return -1;
    }

    // spec says that 'data' is a stream of bytes that must be parsed
    // as uint32_t LE, so read 4 at a time
    size_t data_length = 4 * width * height;
    unsigned char *data_bytes = new unsigned char[data_length];
    if (data_bytes == NULL) {
        cerr << "Can't allocate buffer for decompression" << endl;
        delete [] bytes;
        return -1;
    }

    if (layer0["compression"] == "zlib") {
        ret = uncompress(data_bytes, &data_length, bytes, length);
        switch (ret) {
            case Z_OK:
                cout << "Decompressing successful" << endl;
                ret = 0;
                break;
            case Z_MEM_ERROR:
                cerr << "Decompressing ran out of memory" << endl;
                ret = 1;
                break;
            case Z_BUF_ERROR:
                cerr << "Decompressing buffer not large enough" << endl;
                ret = 1;
                break;
            case Z_DATA_ERROR:
                cerr << "Decompressing: input data corruption" << endl;
                ret = 1;
                break;
            default:
                cerr << "Decompressing: unknown error " << ret << endl;
                ret = 1;
                break;
        }
        delete [] bytes;

        if (ret) {
            delete [] data_bytes;
            return -1;
        }
    }

    // tileset info
    json tileset0 = j["tilesets"][0];
    const uint32_t first_gid = tileset0["firstgid"];

    const size_t nr_tiles = width * height;
    uint32_t *tiles_id = new uint32_t[nr_tiles];
    if (tiles_id == NULL) {
        cerr << "Can't allocate buffer for tiles id" << endl;
        delete [] data_bytes;
        return -1;
    }

    for (size_t i = 0; i < nr_tiles; ++i) {
        uint32_t val = 0;
        val |= ((uint32_t)data_bytes[(4 * i) + 0]) << 0;
        val |= ((uint32_t)data_bytes[(4 * i) + 1]) << 8;
        val |= ((uint32_t)data_bytes[(4 * i) + 2]) << 16;
        val |= ((uint32_t)data_bytes[(4 * i) + 3]) << 24;
        tiles_id[i] = val;
        if (val != 0) {
            // TODO: build a "GID -> tileset x tileid" function
            cout << "info: cell " << i << " = " << val - first_gid << endl;
        }
    }
    delete [] data_bytes;

    cout << "Exiting..." << endl;
    delete [] tiles_id;
    return 0;
#if 0
    SDL_INIT_TIMER          timer subsystem
        SDL_INIT_AUDIO          audio subsystem
        SDL_INIT_VIDEO          video subsystem; automatically initializes the events subsystem
        SDL_INIT_JOYSTICK       joystick subsystem; automatically initializes the events subsystem
        SDL_INIT_HAPTIC         haptic(force feedback) subsystem
        SDL_INIT_GAMECONTROLLER controller subsystem; automatically initializes the joystick subsystem
        SDL_INIT_EVENTS         events subsystem
        SDL_INIT_EVERYTHING             all of the above subsystems
#endif
    SDL_Init(SDL_INIT_EVERYTHING);

    /* Create window & renderer */
    SDL_Window *sdlWindow;
    SDL_Renderer *sdlRenderer;

#if 0
    SDL_WINDOW_FULLSCREEN_DESKTOP
        SDL_WINDOW_FULLSCREEN
#endif
    ret = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
                                      SDL_WINDOW_RESIZABLE, &sdlWindow, &sdlRenderer);

    if (ret) {
        cerr << "SDL_CreateWindowAnRendere error: " << SDL_GetError() << endl;
        SDL_Quit();
        return -1;
    }

    /* Configure renderer ... */
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);

    /* ... and scale the window correctly if fullscreen */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
    SDL_RenderSetLogicalSize(sdlRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    //Load image
    SDL_Surface *hello = SDL_LoadBMP("resources/img/purple.bmp");
    if (hello == NULL) {
        myerror("SDL_LoadBMP error: ", sdlWindow, sdlRenderer);
        return -1;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(sdlRenderer, hello);
    SDL_FreeSurface(hello);

    if (texture == NULL) {
        myerror("SDL_CreateTextureFromSurface error: ", sdlWindow, sdlRenderer);
        return -1;
    }

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;

    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);

    rect.w = w;
    rect.h = h;

    ret = SDL_RenderCopy(sdlRenderer, texture, NULL, &rect);
    if (ret) {
        SDL_DestroyTexture(texture);
        myerror("SDL_RenderCopy error: ", sdlWindow, sdlRenderer);
        return -1;
    }

    /* Create level */
    size_t nr_cells_plat = SCREEN_WIDTH / 32;
    nr_cells = nr_cells_plat + 2;

    size_t nh = SCREEN_HEIGHT / 32 - 5;

    cell_t *level = new cell_t[nr_cells];

    if (level == NULL) {
        myerror("new cell_t[] failure", sdlWindow, sdlRenderer);
        return -1;
    }

    for (size_t i = 0; i < nr_cells_plat; ++i) {
        level[i].pos.x = i;
        level[i].pos.y = nh;
        level[i].size.x = 1;
        level[i].size.y = 1;
    }

    level[nr_cells_plat].pos.x = 10;
    level[nr_cells_plat].pos.y = nh - 2;
    level[nr_cells_plat].size.x = 1;
    level[nr_cells_plat].size.y = 1;

    level[nr_cells_plat + 1].pos.x = 0; //11;
    level[nr_cells_plat + 1].pos.y = 4; //nh - 2;
    level[nr_cells_plat + 1].size.x = 1;
    level[nr_cells_plat + 1].size.y = 1;
    for (size_t i = 0; i < nr_cells; ++i) {
        size_t ci = i * 255 / 32;
        SDL_SetRenderDrawColor(sdlRenderer,
                               ci, 255 - ci, (255 - ci) / 2, 255);
        SDL_Rect rect2;
        rect2.x = level[i].pos.x * 32;
        rect2.y = level[i].pos.y * 32;
        rect2.w = level[i].size.x * 32;
        rect2.h = level[i].size.y * 32;
        SDL_RenderFillRect(sdlRenderer, &rect2);
    }


    SDL_RenderPresent(sdlRenderer);
    /* Pause to see the result */
    SDL_Delay(2000);

    /* Exit */
    delete[] level;
    myerror("Success !", sdlWindow, sdlRenderer);

    return 0;
}
