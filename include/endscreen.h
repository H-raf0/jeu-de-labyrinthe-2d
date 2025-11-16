#ifndef ENDSCREEN_H
#define ENDSCREEN_H

#include <SDL2/SDL.h>
#include "effetSDL.h"

// Énumération pour savoir comment la partie s'est terminée
typedef enum {
    GAME_WON,
    GAME_LOST,
    GAME_QUIT_MANUALLY
} GameResult;

void show_end_screen(SDL_Renderer* renderer, const char* image_path);

#endif // ENDSCREEN_H