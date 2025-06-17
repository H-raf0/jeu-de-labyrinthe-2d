#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define LAYERS_NB 5

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
void ShowMovingLayer(SDL_Texture* texture, SDL_Rect window_dim, SDL_Renderer* renderer, int x) {
    SDL_Rect src = {0}, dst = {0};
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    dst = window_dim;
    dst.x = x;
    dst.y = (window_dim.h - dst.h) / 2;

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

    src.x = frameIndex * frameW;
    src.y = 1 * frameH;  // Ligne 2
    src.w = frameW;
    src.h = frameH;

    dst.w = frameW * zoom;
    dst.h = frameH * zoom;
    dst.x = posX;
    dst.y = posY;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur initialisation SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = SDL_CreateWindow("Parallaxe & Dragon", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) end_sdl(0, "Erreur création fenêtre", NULL, NULL);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) end_sdl(0, "Erreur création renderer", window, NULL);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // fond noir

    SDL_Rect window_dimensions = {0};
    SDL_GetWindowSize(window, &window_dimensions.w, &window_dimensions.h);

    // Chargement des couches de fond
    SDL_Texture* layers[LAYERS_NB];
    for (int i = 0; i < LAYERS_NB; i++) {
        char filename[100];
        sprintf(filename, "./img/winter/%d.png", i + 1);
        layers[i] = load_texture_from_image(filename, window, renderer);
    }

    // Chargement du sprite du dragon
    SDL_Texture* dragonTex = load_texture_from_image("./img/dragon/drg.png", window, renderer);

    int dragonFrame = 0, frameCount = 0;
    int x1[LAYERS_NB], x2[LAYERS_NB];
    float speeds[LAYERS_NB] = {5, 4, 3, 2, 1};  // De l’arrière vers l’avant

    // Initialisation des positions des couches
    for (int i = 0; i < LAYERS_NB; i++) {
        x1[i] = 0;
        x2[i] = window_dimensions.w;
    }

    // Boucle principale
    while (frameCount <= 300) {
        SDL_RenderClear(renderer);

        // Affichage des couches de fond (parallaxe)
        for (int i = LAYERS_NB - 1; i >= 0; i--) {
            ShowMovingLayer(layers[i], window_dimensions, renderer, x1[i]);
            ShowMovingLayer(layers[i], window_dimensions, renderer, x2[i]);

            x1[i] -= speeds[i];
            x2[i] -= speeds[i];

            if (x1[i] < -window_dimensions.w) x1[i] += 2 * window_dimensions.w;
            if (x2[i] < -window_dimensions.w) x2[i] += 2 * window_dimensions.w;
        }

        // Affichage du dragon
        CreateDragon(dragonTex, window, renderer, window_dimensions.w / 4, window_dimensions.h * 2 / 8, dragonFrame % 3);

        // Changement de sprite toutes les 10 frames
        if (frameCount % 10 == 0) dragonFrame++;

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // Environ 60 FPS
        frameCount++;
    }

    // Libération des ressources
    for (int i = 0; i < LAYERS_NB; i++) {
        SDL_DestroyTexture(layers[i]);
    }
    SDL_DestroyTexture(dragonTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
