#ifndef DRAWING_H
#define DRAWING_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


#define NUM_LAYERS 2

// Énumération pour la difficulté
typedef enum {
    EASY,
    MEDIUM,
    HARD,
    DIFFICULTY_COUNT
} DifficultyLevel;

// Structure pour les données du fond en parallaxe
typedef struct {
    SDL_Texture* layers[NUM_LAYERS];
    float speeds[NUM_LAYERS];
    float x_pos[NUM_LAYERS][2]; // [couche][0 pour image1, 1 pour image2]
} ParallaxBackground;




void init_parallax(SDL_Renderer* renderer, ParallaxBackground* bg, int window_w);

void draw_parallax(SDL_Renderer* renderer, ParallaxBackground* bg, int window_w, int window_h);


void draw_decorated_button(SDL_Renderer* renderer, SDL_Rect* rect, const char* icon_type, DifficultyLevel difficulty, bool is_hovered);
void draw_sound_icon(SDL_Renderer* renderer, SDL_Rect* rect, bool sound_on, bool is_hovered);
void cleanup_drawing(ParallaxBackground* bg);

#endif