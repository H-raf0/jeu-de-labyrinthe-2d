#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080


void end_sdl(char ok,                                               // fin normale : ok = 0 ; anormale ok = 1
    char const* msg,                                       // message à afficher
    SDL_Window* window,                                    // fenêtre à fermer
    SDL_Renderer* renderer) {                              // renderer à fermer
    char msg_formated[255];                                            
    int l;                                                     
                                            
    if (!ok) {                                                        // Affichage de ce qui ne va pas
        strncpy(msg_formated, msg, 250);                                         
        l = strlen(msg_formated);                                            
        strcpy(msg_formated + l, " : %s\n");                                     
                                            
        SDL_Log(msg_formated, SDL_GetError());                                   
    }                                                          
                                            
    if (renderer != NULL) {                                           // Destruction si nécessaire du renderer
        SDL_DestroyRenderer(renderer);                                  // Attention : on suppose que les NULL sont maintenus !!
        renderer = NULL;
    }
    if (window != NULL)   {                                           // Destruction si nécessaire de la fenêtre
        SDL_DestroyWindow(window);                                      // Attention : on suppose que les NULL sont maintenus !!
        window= NULL;
    }
                                            
    SDL_Quit();                                                    
                                            
    if (!ok) {                                       // On quitte si cela ne va pas            
        exit(EXIT_FAILURE);                                     
    }                                                          
}  





int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Error : SDL initialisation - %s\n", SDL_GetError());              
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = SDL_CreateWindow("Sprites",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        SDL_Log("Error : Window creation - %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_Log("Error : Renderer creation - %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //SDL_DestroyTexture(SDL_Texture);
    SDL_Quit();

    return 0;
}