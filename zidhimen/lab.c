#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LIGNES 10
#define COLONNES 10
#define CELL 40
#define LARGEUR (COLONNES * CELL)
#define HAUTEUR (LIGNES * CELL)

typedef struct {
    int u, v;
} Arete;

typedef struct {
    int *parent, *rang;
    int taille;
} Partition;

void init_partition(Partition* p, int taille) {
    p->taille = taille;
    p->parent = malloc(sizeof(int) * taille);
    p->rang = malloc(sizeof(int) * taille);
    for (int i = 0; i < taille; i++) {
        p->parent[i] = i;
        p->rang[i] = 0;
    }
}

void free_partition(Partition* p) {
    free(p->parent);
    free(p->rang);
}

int find(Partition* p, int i) {
    if (p->parent[i] != i)
        p->parent[i] = find(p, p->parent[i]);
    return p->parent[i];
}

int union_sets(Partition* p, int i, int j) {
    int ri = find(p, i), rj = find(p, j);
    if (ri == rj) return 0;
    if (p->rang[ri] < p->rang[rj])
        p->parent[ri] = rj;
    else {
        p->parent[rj] = ri;
        if (p->rang[ri] == p->rang[rj])
            p->rang[ri]++;
    }
    return 1;
}

void shuffle(Arete* G, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Arete tmp = G[i]; G[i] = G[j]; G[j] = tmp;
    }
}

int generer_aretes(Arete** G_ptr, int lignes, int colonnes) {
    int max = 2 * lignes * colonnes - lignes - colonnes;
    Arete* G = malloc(sizeof(Arete) * max);
    int k = 0;
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int i = y * colonnes + x;
            if (x + 1 < colonnes) G[k++] = (Arete){i, i + 1};
            if (y + 1 < lignes)   G[k++] = (Arete){i, i + colonnes};
        }
    }
    *G_ptr = G;
    return k;
}

void supprimer_mur(int* murs, int cols, int u, int v) {
    int x1 = u % cols, y1 = u / cols;
    int x2 = v % cols, y2 = v / cols;
    if (x2 == x1 + 1) { murs[u] &= ~2; murs[v] &= ~8; }     // droite
    else if (x2 == x1 - 1) { murs[u] &= ~8; murs[v] &= ~2; } // gauche
    else if (y2 == y1 + 1) { murs[u] &= ~4; murs[v] &= ~1; } // bas
    else if (y2 == y1 - 1) { murs[u] &= ~1; murs[v] &= ~4; } // haut
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Labyrinthe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    srand(time(NULL));

    int nb_cells = LIGNES * COLONNES;
    int* murs = malloc(sizeof(int) * nb_cells);
    for (int i = 0; i < nb_cells; i++) murs[i] = 15; // 1111 = tous les murs

    Arete* G;
    int nb_aretes = generer_aretes(&G, LIGNES, COLONNES);
    shuffle(G, nb_aretes);

    Partition p;
    init_partition(&p, nb_cells);

    for (int i = 0, count = 0; i < nb_aretes && count < nb_cells - 1; i++) {
        if (union_sets(&p, G[i].u, G[i].v)) {
            supprimer_mur(murs, COLONNES, G[i].u, G[i].v);
            count++;
        }
    }

    int quit = 0;
    SDL_Event e;
    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        for (int y = 0; y < LIGNES; y++) {
            for (int x = 0; x < COLONNES; x++) {
                int i = y * COLONNES + x;
                int mx = x * CELL;
                int my = y * CELL;
                if (murs[i] & 1) SDL_RenderDrawLine(renderer, mx, my, mx + CELL, my);             // haut
                if (murs[i] & 2) SDL_RenderDrawLine(renderer, mx + CELL, my, mx + CELL, my + CELL); // droite
                if (murs[i] & 4) SDL_RenderDrawLine(renderer, mx, my + CELL, mx + CELL, my + CELL); // bas
                if (murs[i] & 8) SDL_RenderDrawLine(renderer, mx, my, mx, my + CELL);             // gauche
            }
        }

        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
        }
    }

    free_partition(&p);
    free(murs);
    free(G);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
