#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

void Init_SDL (){
        /* Initialisation de la SDL  + gestion de l'échec possible */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Error : SDL initialisation - %s\n", 
                SDL_GetError());                // l'initialisation de la SDL a échoué 
    exit(EXIT_FAILURE);
    }
}
SDL_Window* CREATEWindowfenetre(int posX,int posY, int length,int height,SDL_Renderer** renderer){
    SDL_Window *window;
    window=SDL_CreateWindow(
        "SDL2 Programme 0.1",
        posX,posY,
        length,height,
        SDL_WINDOW_SHOWN);
}
int main(){
    return 0;
}
