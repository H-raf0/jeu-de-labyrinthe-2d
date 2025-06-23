#include "maze.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

// --- Implémentation de Disjoint Set Union (DSU) ---
typedef struct {
    int* parent;
    int* rank;
    int n;
} DSU;

DSU* dsu_create(int n) {
    DSU* dsu = malloc(sizeof(DSU));
    dsu->n = n;
    dsu->parent = malloc(n * sizeof(int));
    dsu->rank = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        dsu->parent[i] = i;
        dsu->rank[i] = 0;
    }
    return dsu;
}

int dsu_find(DSU* dsu, int i) {
    if (dsu->parent[i] != i) {
        dsu->parent[i] = dsu_find(dsu, dsu->parent[i]); // Path compression
    }
    return dsu->parent[i];
}

void dsu_union(DSU* dsu, int i, int j) {
    int root_i = dsu_find(dsu, i);
    int root_j = dsu_find(dsu, j);
    if (root_i != root_j) {
        if (dsu->rank[root_i] < dsu->rank[root_j]) {
            dsu->parent[root_i] = root_j;
        } else if (dsu->rank[root_i] > dsu->rank[root_j]) {
            dsu->parent[root_j] = root_i;
        } else {
            dsu->parent[root_j] = root_i;
            dsu->rank[root_i]++;
        }
    }
}

void dsu_destroy(DSU* dsu) {
    free(dsu->parent);
    free(dsu->rank);
    free(dsu);
}

// --- Logique du Labyrinthe ---

Maze* create_maze(int width, int height) {
    Maze* maze = malloc(sizeof(Maze));
    maze->width = width;
    maze->height = height;
    // Initialiser tous les murs à 'true' (présents)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int i = 0; i < 4; ++i) {
                maze->walls[y][x][i] = true;
            }
        }
    }
    return maze;
}

void destroy_maze(Maze* maze) {
    free(maze);
}

// Fisher-Yates shuffle
void shuffle(void* array, size_t n, size_t size) {
    char* arr = array;
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            char* p1 = arr + i * size;
            char* p2 = arr + j * size;
            char tmp[size];
            memcpy(tmp, p1, size);
            memcpy(p1, p2, size);
            memcpy(p2, tmp, size);
        }
    }
}

typedef struct {
    int x1, y1, x2, y2;
    int dir1, dir2;
} Edge;

void generate_maze_kruskal(Maze* maze, float p) {
    srand(time(NULL));
    int w = maze->width;
    int h = maze->height;
    int num_cells = w * h;

    // 1. Créer la liste de toutes les arêtes (murs intérieurs)
    Edge* edges = malloc( (w * (h - 1) + h * (w - 1)) * sizeof(Edge));
    int edge_count = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (x < w - 1) { // Mur Est
                edges[edge_count++] = (Edge){x, y, x + 1, y, 1, 3}; // E, W
            }
            if (y < h - 1) { // Mur Sud
                edges[edge_count++] = (Edge){x, y, x, y + 1, 2, 0}; // S, N
            }
        }
    }

    // 2. Mélanger les arêtes (algorithme de Fisher-Yates)
    shuffle(edges, edge_count, sizeof(Edge));
    
    // 3. Appliquer l'algorithme de Kruskal
    DSU* dsu = dsu_create(num_cells);
    
    for (int i = 0; i < edge_count; i++) {
        Edge e = edges[i];
        int cell1_idx = e.y1 * w + e.x1;
        int cell2_idx = e.y2 * w + e.x2;

        if (dsu_find(dsu, cell1_idx) != dsu_find(dsu, cell2_idx)) {
            // Les cellules sont dans des composantes différentes : on casse le mur
            dsu_union(dsu, cell1_idx, cell2_idx);
            maze->walls[e.y1][e.x1][e.dir1] = false;
            maze->walls[e.y2][e.x2][e.dir2] = false;
        } else {
            // Section 6.2.2 : Extension pour labyrinthe non arborescent
            // Les cellules sont déjà connectées. On peut quand même casser le mur avec une probabilité p.
            if ((float)rand() / RAND_MAX < p) {
                maze->walls[e.y1][e.x1][e.dir1] = false;
                maze->walls[e.y2][e.x2][e.dir2] = false;
            }
        }
    }

    free(edges);
    dsu_destroy(dsu);
}