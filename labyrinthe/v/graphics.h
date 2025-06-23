#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "maze.h"
#include "pathfinding.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define TILE_SIZE 16 // Taille d'une vignette en pixels

// Structure pour gérer les ressources graphiques
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* tileset;
    SDL_Rect floor_tile;
    SDL_Rect wall_tile;
} Graphics;

// Fonctions graphiques
Graphics* init_graphics();
void close_graphics(Graphics* g);
void load_tileset(Graphics* g, const char* path);

void draw_maze_lines(Graphics* g, Maze* maze);
void draw_maze_tiles(Graphics* g, Maze* maze);
void draw_path_info(Graphics* g, PathResult* result, Point start, Point end);
void draw_distance_gradient(Graphics* g, PathResult* result);
void animate_character(Graphics* g, PathResult* result, Point start);

#endif