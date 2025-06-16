#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

void Init_SDL (){
        /* Initialisation de la SDL  + gestion de l'échec possible */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Error : SDL initialisation - %s\n",SDL_GetError());                // l'initialisation de la SDL a échoué 
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
    if (window == 0){
        SDL_Log("Error : SDL window 1 creation - %s\n",SDL_GetError());// échec de la création de la fenêtre
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED ); /*  SDL_RENDERER_SOFTWARE */
    if (renderer == 0) {
        SDL_Log("Error : SDL window 1 creation - %s\n",SDL_GetError());// échec de la création de la fenêtre
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);
    return window;
}








void fermeture(SDL_Window *window, SDL_Renderer *renderer){
    /* on referme tout ce qu'on a ouvert en ordre inverse de la création */

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

int main(){

    return 0;
}
