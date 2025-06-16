#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#define PI 3.14


void Init_SDL(){
    /* Initialisation de la SDL  + gestion de l'échec possible */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Error : SDL initialisation - %s\n", 
                SDL_GetError());                // l'initialisation de la SDL a échoué 
    exit(EXIT_FAILURE);
    }
}

SDL_Window* CreateWindow(int posX, int posY, int length, int height, SDL_Renderer** renderer) {
    SDL_Window *window = NULL;                     // Future fenêtre de gauche

    /* Création de la fenêtre */
    window = SDL_CreateWindow(
    "Fenêtre",                    // codage en utf8, donc accents possibles
    posX, posY,                                  // coin haut gauche en haut gauche de l'écran
    length, height,                              // largeur, hauteur
    SDL_WINDOW_SHOWN);                 // redimensionnable

    if (window == NULL) {
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

    return window;
    
}


void CreateWaveWindows(SDL_Window **windowsArray, SDL_Renderer **renderersArray, int h, int k, int r){ // h : Centre x  k : Centre y  r : diametre, Definir les cordonnes du centre et du diametre du cercle

    int decal = 2; // decalage utiliser pour sauter 'decal' etapes de fenetres
    static int waveNb = 0;
    int offset = 8 * waveNb;
    double x, y, theta;

    for(int i=0; i<5; i++){

        // Calculate angle for each point (incrementing by 45 degrees)
        theta = i * 45.0 * PI / 180.0;

        // Calculate x and y coordinates using trigonometry
        x = h + r * cos(theta);
        y = k + r * sin(theta);

        windowsArray[offset + i] = CreateWindow(x, y, 100, 100, &renderersArray[offset + i]);
    }
    h += r*2;
    for(int i=5; i<8; i++){

        // Calculate angle for each point (incrementing by 45 degrees)
        theta = (i + decal) * 45.0 * PI / 180.0;

        // Calculate x and y coordinates using trigonometry
        x = h + r * cos(theta);
        y = k + r * sin(theta);

        windowsArray[offset + i + decal] = CreateWindow(x, y, 100, 100, &renderersArray[offset + i + decal]);
        decal -= 2;
    }
    waveNb++;
}


void MoveWindows(SDL_Window **windowsArray, int count) {
    for (int i = 0; i < count; i++) {
        int x, y;
        SDL_GetWindowPosition(windowsArray[i], &x, &y);
        SDL_SetWindowPosition(windowsArray[i], x + 20, y);
    }
    SDL_Delay(100);
}

void DestroyWindow(SDL_Window *window, SDL_Renderer *renderer){

    /* on referme tout ce qu'on a ouvert en ordre inverse de la création */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


int main(){
    int nbWaves = 2;
    int windowsNb = 8*nbWaves;
    Init_SDL();
    SDL_Window **windowsArray = (SDL_Window **) malloc(sizeof(SDL_Window *) * windowsNb);
    SDL_Renderer **renderersArray = (SDL_Renderer **)malloc(sizeof(SDL_Renderer *) * windowsNb);
     

    for(int i=0; i<nbWaves; i++){
        
        CreateWaveWindows(windowsArray, renderersArray, 300+i*100*4, 600, 100);  
    }
    
    
    for (int j = 0; j < 40; j++) {
        MoveWindows(windowsArray, windowsNb);
    }
    

    for(int i=windowsNb-1; i>=0; i--){
        
        DestroyWindow(windowsArray[i], renderersArray[i]);
    }

    free(windowsArray);
    free(renderersArray);
    

    SDL_Quit();                                // quitter la SDL
    return 0;

}