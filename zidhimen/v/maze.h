#ifndef MAZE_H
#define MAZE_H

#include <stdbool.h>

// Dimensions du labyrinthe
#define MAZE_WIDTH 40
#define MAZE_HEIGHT 30

// Structure pour une cellule (pour la conversion en graphe)
typedef struct {
    int x;
    int y;
} Point;

// Structure du labyrinthe
// Les murs sont stockés pour chaque cellule.
// walls[y][x][0] = mur Nord, [1] = Est, [2] = Sud, [3] = Ouest
typedef struct {
    int width;
    int height;
    bool walls[MAZE_HEIGHT][MAZE_WIDTH][4]; // N, E, S, W
} Maze;

// Fonctions de gestion du labyrinthe
Maze* create_maze(int width, int height);
void generate_maze_kruskal(Maze* maze, float p); // p est la proba de créer un cycle
void destroy_maze(Maze* maze);

#endif