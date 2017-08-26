#include <iostream>
#include <SDL2/SDL.h>

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

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

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
        int ret = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
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
        SDL_Surface *hello = SDL_LoadBMP("res/img/purple.bmp");
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
        delete [] level;
        myerror("Success !", sdlWindow, sdlRenderer);

        return 0;
}
