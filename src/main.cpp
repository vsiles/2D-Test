#include <SDL/SDL.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SDL_Surface* hello = NULL;
    SDL_Surface* screen = NULL;

    //Start SDL 
    SDL_Init(SDL_INIT_EVERYTHING);

    //Set up screen
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    
    //Load image
    hello = SDL_LoadBMP("res/img/purple.bmp");

    //Apply image to screen
    SDL_BlitSurface(hello, NULL, screen, NULL);
    
    //Update Screen
    SDL_Flip(screen);
    
    //Pause
    SDL_Delay(2000);

    //Quit SDL
    SDL_Quit();

    return 0;
}
