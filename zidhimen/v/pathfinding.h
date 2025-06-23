#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "maze.h"
#include <stdbool.h>

// Structure pour stocker les résultats d'un algorithme de recherche
typedef struct {
    Point* path;
    int path_len;
    Point* visited;
    int visited_count;
    int* distances; // Distance de chaque cellule à la source/destination
} PathResult;

// Fonctions pour les structures de données
// (Implémentations internes dans pathfinding.c)

// Algorithmes de recherche
PathResult* bfs_path(Maze* maze, Point start, Point end);
PathResult* dijkstra_path(Maze* maze, Point start, Point end);
PathResult* a_star_path(Maze* maze, Point start, Point end, int heuristic_choice);

// Heuristiques pour A*
#define MANHATTAN 1
#define EUCLIDEAN 2
#define CHEBYSHEV 3

void destroy_path_result(PathResult* result);

#endif