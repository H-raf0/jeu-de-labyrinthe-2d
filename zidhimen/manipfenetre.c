#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

// Fonction d'initialisation de la bibliothèque SDL
void Init_SDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur : Initialisation de SDL - %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

// Fonction pour créer une fenêtre SDL avec un rendu associé
SDL_Window* CREATEWindowfenetre(int posX, int posY, int length, int height, SDL_Renderer** renderer) {
    SDL_Window *window = SDL_CreateWindow(
        "SDL2 Programme 0.1",   // Titre de la fenêtre
        posX, posY,             // Position X et Y
        length, height,         // Taille de la fenêtre
        SDL_WINDOW_SHOWN        // Option pour afficher la fenêtre
    );
        
    if (window == NULL) {
        SDL_Log("Erreur : Création de la fenêtre - %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Création du renderer (moteur de rendu graphique) associé à la fenêtre
    *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        SDL_Log("Erreur : Création du renderer - %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Nettoyage initial de la fenêtre (fond noir)
    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);

    return window;
}

// Fonction de destruction d'une fenêtre et de son renderer
void fermeture(SDL_Window *window, SDL_Renderer *renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

// Fonction qui crée plusieurs fenêtres et les fait descendre à l'écran
void horizontal(SDL_Window **windowsArray, SDL_Renderer **renderersArray, int windowsNb) {
    int y = 350; // Position de départ sur l'axe Y
    int x = 0;   // Position de départ sur l'axe X

    // Création de toutes les fenêtres les unes à côté des autres
    for (int i = 0; i < windowsNb; i++) {
        x += 20; // Décalage horizontal pour chaque nouvelle fenêtre
        windowsArray[i] = CREATEWindowfenetre(x, y, 100 + i, 100 + i, &renderersArray[i]);
    }

    // Animation : déplacement progressif vers le bas de chaque fenêtre
    for (int i = 0; i < windowsNb; i++) {
        int wx, wy;
        SDL_GetWindowPosition(windowsArray[i], &wx, &wy);

        // Déplacement en 20 étapes de 5 pixels (5 * 20 = 100 pixels)
        for (int step = 0; step < 100; step += 5) {
            SDL_SetWindowPosition(windowsArray[i], wx, wy + step);
            SDL_Delay(10); // Pause pour voir le mouvement
        }
    }
}

int main() {
    int nbWaves = 1;               // Nombre de "vagues" de fenêtres
    int windowsNb = 60 * nbWaves;  // Nombre total de fenêtres

    Init_SDL(); // Initialisation de SDL

    // Allocation dynamique de tableaux de fenêtres et de renderers
    SDL_Window **windowsArray = (SDL_Window **)malloc(sizeof(SDL_Window *) * windowsNb);
    SDL_Renderer **renderersArray = (SDL_Renderer **)malloc(sizeof(SDL_Renderer *) * windowsNb);

    // Création et animation horizontale des fenêtres
    horizontal(windowsArray, renderersArray, windowsNb);

    // Pause avant la fermeture (50 ms)
    SDL_Delay(50);

    // Fermeture et libération des fenêtres et renderers dans l'ordre inverse
    for (int i = windowsNb - 1; i >= 0; i--) {
        fermeture(windowsArray[i], renderersArray[i]);
    }

    // Libération de la mémoire
    free(windowsArray);
    free(renderersArray);

    SDL_Quit(); // Fermeture de SDL
    return 0;
}
