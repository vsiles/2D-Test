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

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

#if 0
	SDL_INIT_TIMER		timer subsystem
	SDL_INIT_AUDIO		audio subsystem
	SDL_INIT_VIDEO		video subsystem; automatically initializes the events subsystem
	SDL_INIT_JOYSTICK	joystick subsystem; automatically initializes the events subsystem
	SDL_INIT_HAPTIC		haptic(force feedback) subsystem
	SDL_INIT_GAMECONTROLLER	controller subsystem; automatically initializes the joystick subsystem
	SDL_INIT_EVENTS		events subsystem
	SDL_INIT_EVERYTHING 		all of the above subsystems
#endif
	SDL_Init(SDL_INIT_EVERYTHING);

	/* Create window & renderer */
	SDL_Window *sdlWindow;
	SDL_Renderer *sdlRenderer;
	int ret = SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_RESIZABLE, &sdlWindow, &sdlRenderer);

	if (ret) {
		cerr << "SDL_CreateWindowAnRendere error: " << SDL_GetError() << endl;
		SDL_Quit();
		return -1;
	}

	/* Configure renderer ... */
	SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderPresent(sdlRenderer);

	/* ... and scale the window to 640 x 480 */
//	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
//	SDL_RenderSetLogicalSize(sdlRenderer, 640, 480);

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

	cout << "w = " << w << endl;
	cout << "h = " << h << endl;

	rect.w = w;
	rect.h = h;

	ret = SDL_RenderCopy(sdlRenderer, texture, NULL, &rect);
	if (ret) {
		SDL_DestroyTexture(texture);
		myerror("SDL_RenderCopy error: ", sdlWindow, sdlRenderer);
	}

	SDL_RenderPresent(sdlRenderer);

    /* Pause to see the result */
    SDL_Delay(2000);

    /* Exit */
	myerror("Success !", sdlWindow, sdlRenderer);

    return 0;
}
