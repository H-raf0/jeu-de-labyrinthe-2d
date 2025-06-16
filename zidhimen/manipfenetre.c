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
void MoveWindows(SDL_Window *windowsArray[],int count){
    for (int i = 0; i < count; i++) {
        int x, y;
        SDL_GetWindowPosition(windowsArray[i], &x, &y);
        SDL_SetWindowPosition(windowsArray[i], x + 60, y);
    }
    

}

void horizontal(SDL_Window **windowsArray, SDL_Renderer **renderersArray){
    int y=350;//position initial
    int x=0;//position de debut de x 
    for(int i=0; i<90; i++){
        x+=20;
        windowsArray[i] = CREATEWindowfenetre(x, y, 100, 100, &renderersArray[i]);
        
        
    }
    
}







int main(){
    int nbWaves = 1;
    int windowsNb = 8*nbWaves;
    Init_SDL();
    SDL_Window **windowsArray = (SDL_Window **) malloc(sizeof(SDL_Window *) * windowsNb);
    SDL_Renderer **renderersArray = (SDL_Renderer **)malloc(sizeof(SDL_Renderer *) * windowsNb);
    horizontal(windowsArray, renderersArray); 
    for(int i=windowsNb-1; i>=0; i--){    
        fermeture(windowsArray[i], renderersArray[i]);
    }
    free(windowsArray);
    free(renderersArray);
    

    SDL_Quit();                                // quitter la SDL
    return 0;


}
