#include "spriteSDL.h"
#include <math.h>
#include <stdio.h>
#include <string.h>




int main() {

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect window_dimensions = {0};
    InitialisationSDL(&window, &renderer, &window_dimensions);
    // Chargement des couches de fond
    SDL_Texture** layers = chargerCouche(window, renderer);  
    // Chargement du sprite du dragon
    SDL_Texture* dragonTex = chargerDragon(window, renderer);




    int dragonFrame = 0, frameCount = 0;
    int dragonPos[2] = {window_dimensions.w / 4, window_dimensions.h * 2 / 8};
    int x1[COUCHES_NB], x2[COUCHES_NB];
    float speeds[COUCHES_NB] = {5, 4, 3, 2, 1};  // vitesse de mouvement des couches De l'avant vers l’arriere
    
    // Initialisation des positions des couches
    for (int i = 0; i < COUCHES_NB; i++) {
        x1[i] = 0;
        x2[i] = window_dimensions.w;
    }

    //condition d'arret
    int running = 1;


    SDL_Event event;
    
    // Boucle principale
    while (running) {

        // Gestion des événements clavier
        const Uint8* state = SDL_GetKeyboardState(NULL);

        // Récupération des événements (fermeture, etc.)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = 0;
        }

        // Lecture des touches pressées en continu
        if (state[SDL_SCANCODE_UP])    dragonPos[1] -= 10;
        if (state[SDL_SCANCODE_DOWN])  dragonPos[1] += 10;
        if (state[SDL_SCANCODE_LEFT])  dragonPos[0] -= 10;
        if (state[SDL_SCANCODE_RIGHT]) dragonPos[0] += 10;

        SDL_RenderClear(renderer);

        // Affichage des couches
        for (int i = COUCHES_NB - 1; i >= 0; i--) {
            ShowLayer(layers[i], window_dimensions, renderer, x1[i], 0);
            ShowLayer(layers[i], window_dimensions, renderer, x2[i], 0);

            x1[i] -= speeds[i];
            x2[i] -= speeds[i];

            if (x1[i] < -window_dimensions.w) x1[i] += 2 * window_dimensions.w;
            if (x2[i] < -window_dimensions.w) x2[i] += 2 * window_dimensions.w;
        }

        // Affichage du dragon
        CreateDragon(dragonTex, window, renderer, dragonPos[0], dragonPos[1], dragonFrame % 3);

        // Changement de sprite toutes les 10 frames
        if (frameCount % 10 == 0) dragonFrame++;

        SDL_RenderPresent(renderer);
        SDL_Delay(20);
        frameCount++;
    }

    destroyAndQuit(layers, &dragonTex, &window, &renderer); 
    free(layers);
    return 0;
}
