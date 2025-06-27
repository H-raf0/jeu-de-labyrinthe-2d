#ifndef ENDSCREEN_H
#define ENDSCREEN_H

#include <SDL2/SDL.h>

// Énumération pour savoir comment la partie s'est terminée
typedef enum {
    GAME_WON,
    GAME_LOST,
    GAME_QUIT_MANUALLY
} GameResult;

// Affiche un écran de fin avec une image en fondu
// renderer: le rendu de la fenêtre principale
// image_path: le chemin vers l'image à afficher (ex: "gameover.png")
void show_end_screen(SDL_Renderer* renderer, const char* image_path);

#endif // ENDSCREEN_H