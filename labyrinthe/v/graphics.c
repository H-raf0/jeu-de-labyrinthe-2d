#include "graphics.h"
#include <stdio.h>

Graphics* init_graphics() {
    Graphics* g = malloc(sizeof(Graphics));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        free(g);
        return NULL;
    }
    
    g->window = SDL_CreateWindow("Labyrinthe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 MAZE_WIDTH * TILE_SIZE, MAZE_HEIGHT * TILE_SIZE, SDL_WINDOW_SHOWN);
    if (!g->window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        free(g);
        return NULL;
    }
    
    g->renderer = SDL_CreateRenderer(g->window, -1, SDL_RENDERER_ACCELERATED);
    IMG_Init(IMG_INIT_PNG);

    return g;
}

void load_tileset(Graphics* g, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return;
    }
    g->tileset = SDL_CreateTextureFromSurface(g->renderer, surface);
    SDL_FreeSurface(surface);
    
    // On définit manuellement les zones (SDL_Rect) pour les vignettes
    // En regardant votre image :
    // - Un bout de sol : (x=0, y=16, w=16, h=16) - en haut à gauche
    // - Un bout de mur : (x=64, y=0, w=16, h=16) - une brique simple
    g->floor_tile = (SDL_Rect){0, 16, 16, 16};
    g->wall_tile = (SDL_Rect){64, 0, 16, 16};
}

void close_graphics(Graphics* g) {
    SDL_DestroyTexture(g->tileset);
    SDL_DestroyRenderer(g->renderer);
    SDL_DestroyWindow(g->window);
    IMG_Quit();
    SDL_Quit();
    free(g);
}

// Option 1 : Dessin avec des lignes
void draw_maze_lines(Graphics* g, Maze* maze) {
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255); // Murs blancs
    SDL_RenderClear(g->renderer);
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255); // Fond noir

    for (int y = 0; y < maze->height; y++) {
        for (int x = 0; x < maze->width; x++) {
            if (maze->walls[y][x][0]) { // Nord
                SDL_RenderDrawLine(g->renderer, x * TILE_SIZE, y * TILE_SIZE, (x + 1) * TILE_SIZE, y * TILE_SIZE);
            }
            if (maze->walls[y][x][1]) { // Est
                SDL_RenderDrawLine(g->renderer, (x + 1) * TILE_SIZE, y * TILE_SIZE, (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);
            }
            // Sud et Ouest sont dessinés par les cellules voisines
        }
    }
}

// Option 2: Dessin avec la planche de vignettes
// Cette version dessine un labyrinthe plus "épais"
void draw_maze_tiles(Graphics* g, Maze* maze) {
    SDL_Rect dest_rect = {0, 0, TILE_SIZE, TILE_SIZE};
    
    // On redimensionne la fenêtre pour un affichage plus joli
    SDL_SetWindowSize(g->window, (maze->width * 2 + 1) * TILE_SIZE, (maze->height * 2 + 1) * TILE_SIZE);
    
    for (int y = 0; y < maze->height * 2 + 1; y++) {
        for (int x = 0; x < maze->width * 2 + 1; x++) {
            dest_rect.x = x * TILE_SIZE;
            dest_rect.y = y * TILE_SIZE;
            
            bool is_wall = true;
            if (y % 2 != 0 && x % 2 != 0) { // C'est une cellule
                is_wall = false;
            } else if (y % 2 != 0) { // Entre deux cellules, verticalement
                int maze_x = (x / 2) - 1;
                int maze_y = (y - 1) / 2;
                if (x > 0 && x < maze->width * 2 && !maze->walls[maze_y][maze_x][1]) // Mur Est de la cellule à gauche
                    is_wall = false;
            } else if (x % 2 != 0) { // Entre deux cellules, horizontalement
                int maze_x = (x - 1) / 2;
                int maze_y = (y / 2) - 1;
                 if (y > 0 && y < maze->height*2 && !maze->walls[maze_y][maze_x][2]) // Mur Sud de la cellule au dessus
                    is_wall = false;
            }

            if(is_wall) {
                SDL_RenderCopy(g->renderer, g->tileset, &g->wall_tile, &dest_rect);
            } else {
                 SDL_RenderCopy(g->renderer, g->tileset, &g->floor_tile, &dest_rect);
            }
        }
    }
}

void draw_path_info(Graphics* g, PathResult* result, Point start, Point end) {
    if (!result) return;
    SDL_Rect cell_rect = {0, 0, TILE_SIZE, TILE_SIZE};

    // 1. Noeuds visités par l'algorithme (en bleu semi-transparent)
    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 255, 100);
    for (int i = 0; i < result->visited_count; i++) {
        cell_rect.x = result->visited[i].x * TILE_SIZE;
        cell_rect.y = result->visited[i].y * TILE_SIZE;
        SDL_RenderFillRect(g->renderer, &cell_rect);
    }
    
    // 2. Plus court chemin (en jaune)
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 0, 150);
    for (int i = 0; i < result->path_len; i++) {
        cell_rect.x = result->path[i].x * TILE_SIZE;
        cell_rect.y = result->path[i].y * TILE_SIZE;
        SDL_RenderFillRect(g->renderer, &cell_rect);
    }

    // 3. Départ (vert) et Arrivée (rouge)
    SDL_SetRenderDrawColor(g->renderer, 0, 255, 0, 255);
    cell_rect.x = start.x * TILE_SIZE;
    cell_rect.y = start.y * TILE_SIZE;
    SDL_RenderFillRect(g->renderer, &cell_rect);
    
    SDL_SetRenderDrawColor(g->renderer, 255, 0, 0, 255);
    cell_rect.x = end.x * TILE_SIZE;
    cell_rect.y = end.y * TILE_SIZE;
    SDL_RenderFillRect(g->renderer, &cell_rect);
}

void draw_distance_gradient(Graphics* g, PathResult* result) {
     if (!result || !result->distances) return;
    SDL_Rect cell_rect = {0, 0, TILE_SIZE, TILE_SIZE};
    int max_dist = 0;
    for (int i = 0; i < MAZE_WIDTH * MAZE_HEIGHT; ++i) {
        if (result->distances[i] > max_dist) max_dist = result->distances[i];
    }

    if (max_dist == 0) return;

    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            int dist = result->distances[y * MAZE_WIDTH + x];
            if (dist != -1) {
                // Dégradé de bleu (proche) à rouge (loin)
                Uint8 r = (Uint8)(255 * dist / max_dist);
                Uint8 b = (Uint8)(255 * (max_dist - dist) / max_dist);
                SDL_SetRenderDrawColor(g->renderer, r, 0, b, 128);
                cell_rect.x = x * TILE_SIZE;
                cell_rect.y = y * TILE_SIZE;
                SDL_RenderFillRect(g->renderer, &cell_rect);
            }
        }
    }
}