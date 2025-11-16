#include "bSDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Fonction pour gérer proprement la fin du programme
void end_sdl(char ok, const char* msg, SDL_Window* window, SDL_Renderer* renderer) {
    if (!ok) {
        SDL_Log("%s : %s\n", msg, SDL_GetError());
    }

    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    if (window != NULL) SDL_DestroyWindow(window);

    SDL_Quit();
    if (!ok) exit(EXIT_FAILURE);
}

// Charge une texture à partir d'un fichier image
SDL_Texture* load_texture_from_image(char* file_image_name, SDL_Window* window, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(file_image_name);
    if (surface == NULL) end_sdl(0, "Erreur chargement image", window, renderer);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == NULL) end_sdl(0, "Erreur création texture", window, renderer);

    return texture;
}

// Affiche une couche de fond avec décalage horizontal
void ShowLayer(SDL_Texture* texture, SDL_Rect window_dim, SDL_Renderer* renderer, int x, int y) {
    SDL_Rect src = {0}, dst = {0};
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    dst = window_dim;
    dst.x = x;
    dst.y = y;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

// Affiche une vignette du sprite du dragon
void CreateDragon(SDL_Texture* texture, SDL_Window* window, SDL_Renderer* renderer, int posX, int posY, int frameIndex) {
    SDL_Rect src = {0}, dst = {0}, win_dim = {0};

    SDL_GetWindowSize(window, &win_dim.w, &win_dim.h);
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    int columns = 3, rows = 4;
    float zoom = 1.5f;
    int frameW = src.w / columns;
    int frameH = src.h / rows;

    src.x = frameIndex * frameW; // colonne (frameIndex+1)
    src.y = 1 * frameH;  // Ligne 2
    src.w = frameW;
    src.h = frameH;

    dst.w = frameW * zoom;
    dst.h = frameH * zoom;
    dst.x = posX;
    dst.y = posY;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

void InitialisationSDL(SDL_Window** window, SDL_Renderer** renderer, SDL_Rect* window_dimensions){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur initialisation SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    *window = SDL_CreateWindow("fenetre", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) end_sdl(0, "Erreur création fenêtre", NULL, NULL);

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!(*renderer)) end_sdl(0, "Erreur création renderer", *window, NULL);

    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);  // fond noir

    // recuperer les dimension de l'ecran
    SDL_Rect win_dim = {0};
    SDL_GetWindowSize(*window, &win_dim.w, &win_dim.h);
    *window_dimensions = win_dim;
}

SDL_Texture** chargerCouche(SDL_Window* window, SDL_Renderer* renderer){
    SDL_Texture** couches = malloc(sizeof(SDL_Texture*)*COUCHES_NB);
    for (int i = 0; i < COUCHES_NB; i++) {
        char filename[100];
        sprintf(filename, "./img/winter/%d.png", i + 1);
        couches[i] = load_texture_from_image(filename, window, renderer);
    }
    return couches;
}

SDL_Texture* chargerDragon(SDL_Window* window, SDL_Renderer* renderer){
    return load_texture_from_image("./img/dragon/drg.png", window, renderer);
}

void destroyLayersAndDragon(SDL_Texture** layers, SDL_Texture** dragonTex){

    // Libération des ressources
    for (int i = 0; i < COUCHES_NB; i++) {
        SDL_DestroyTexture(layers[i]);
    }
    SDL_DestroyTexture(*dragonTex);
}

void destroyAndQuit(SDL_Window** window, SDL_Renderer** renderer){
    SDL_DestroyRenderer(*renderer);
    SDL_DestroyWindow(*window);

    IMG_Quit();
    SDL_Quit();

}