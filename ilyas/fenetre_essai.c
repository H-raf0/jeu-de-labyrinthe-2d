#include <SDL2/SDL.h>
#include <stdio.h>
void Init_SDL()
{
    /* Initialisation de la SDL  + gestion de l'échec possible */
    if(SDL_Init(SDL_INIT_VIDEO)!=0)
    {
        // l'initialisation de la SDL a échoué
        SDL_Log("Error : SDL initialisation - %s\n",SDL_GetError());
        exit(EXIT_FAILURE);
    }
}
SDL_Window * Create_Window (int posX, int posY, int length, int height, SDL_Renderer ** renderer)
{
    SDL_Window * window;
    window = SDL_CreateWindow("Fenetre",posX, posY, length, height,SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        SDL_Log("Error : SDL window 1 creation - %s\n", 
                    SDL_GetError());                 // échec de la création de la fenêtre
        SDL_Quit();                              // On referme la SDL       
        exit(EXIT_FAILURE);
    }
    *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (*renderer == NULL) {
        SDL_Log("Error : SDL renderer creation - %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return window;


}
int main()
{
    return 0;
}