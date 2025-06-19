#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>  // Nécessite SDL2 installée pour compilation et linkage

#define CELL_SIZE 25

typedef struct {
    int *parent;
    int *rang;
    int size;
} partition;

typedef struct {
    int u, v;
} arete;

void init_partition(partition* p, int total) {
    p->size = total;
    p->parent = malloc(sizeof(int) * total);
    p->rang = malloc(sizeof(int) * total);
    if (!p->parent || !p->rang) {
        fprintf(stderr, "Allocation mémoire impossible pour partition\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < total; i++) {
        p->parent[i] = i;
        p->rang[i] = 0;
    }
}

void free_partition(partition* p) {
    free(p->parent);
    free(p->rang);
}

int trouver_classe(partition* p, int i) {
    if (p->parent[i] != i)
        p->parent[i] = trouver_classe(p, p->parent[i]);
    return p->parent[i];
}

int fusion(partition* p, int i, int j) {
    int ri = trouver_classe(p, i);
    int rj = trouver_classe(p, j);
    if (ri != rj) {
        if (p->rang[ri] < p->rang[rj]) {
            p->parent[ri] = rj;
        } else if (p->rang[ri] > p->rang[rj]) {
            p->parent[rj] = ri;
        } else {
            p->parent[rj] = ri;
            p->rang[ri]++;
        }
        return 1;
    }
    return 0;
}

void fisher_yates(arete G[], int n) {
    srand((unsigned)time(NULL));
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        arete tmp = G[i];
        G[i] = G[j];
        G[j] = tmp;
    }
}

int generation_grille_vide(arete **G_ptr, int rows, int cols) {
    //int total_cells = rows * cols;
    int max_edges = 2 * rows * cols - rows - cols;
    arete *G = malloc(sizeof(arete) * max_edges);
    if (!G) {
        fprintf(stderr, "Allocation mémoire impossible pour arêtes\n");
        exit(EXIT_FAILURE);
    }
    int edge_count = 0;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int i = y * cols + x;
            if (y + 1 < rows) G[edge_count++] = (arete){i, (y + 1) * cols + x}; // bas
            if (x + 1 < cols) G[edge_count++] = (arete){i, y * cols + (x + 1)};       // droite
        }
    }
    *G_ptr = G;
    return edge_count;
}

void construire_arbre_couvrant(arete G[], int edge_count, arete *arbre, int* tree_count, int total_cells) {
    partition p;
    init_partition(&p, total_cells);
    *tree_count = 0;
    for (int i = 0; i < edge_count; i++) {
        if (fusion(&p, G[i].u, G[i].v)) {
            arbre[*tree_count] = G[i];
            (*tree_count)++;
            if (*tree_count >= total_cells - 1) break;
        }
    }
    free_partition(&p);
}

void generer_dot(const char* nom, arete edges[], int count) {
    FILE* f = fopen(nom, "w");
    if (!f) {
        perror("Erreur ouverture fichier DOT");
        return;
    }
    fprintf(f, "graph G {\n  node [shape=circle];\n");
    for (int i = 0; i < count; i++) {
        fprintf(f, "  %d -- %d;\n", edges[i].u, edges[i].v);
    }
    fprintf(f, "}\n");
    fclose(f);
}

void index_to_coord(int index, int cols, int* x, int* y) {
    *y = index / cols;
    *x = index % cols;
}

void supprimer_mur(int *murs, int cols, int u, int v) {
    int x1, y1, x2, y2;
    index_to_coord(u, cols, &x1, &y1);
    index_to_coord(v, cols, &x2, &y2);
    int dx = x2 - x1;
    int dy = y2 - y1;
    int idx1 = y1 * cols + x1;
    int idx2 = y2 * cols + x2;
    if (dx == 1) {
        murs[idx1] &= ~2;
        murs[idx2] &= ~8;
    } else if (dx == -1) {
        murs[idx1] &= ~8;
        murs[idx2] &= ~2;
    } else if (dy == 1) {
        murs[idx1] &= ~4;
        murs[idx2] &= ~1;
    } else if (dy == -1) {
        murs[idx1] &= ~1;
        murs[idx2] &= ~4;
    }
}

void dessiner_murs(SDL_Renderer* renderer, int x, int y, int *murs, int cols) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int val = murs[y * cols + x];
    if (val & 1) SDL_RenderDrawLine(renderer, px, py, px + CELL_SIZE, py);
    if (val & 2) SDL_RenderDrawLine(renderer, px + CELL_SIZE, py, px + CELL_SIZE, py + CELL_SIZE);
    if (val & 4) SDL_RenderDrawLine(renderer, px, py + CELL_SIZE, px + CELL_SIZE, py + CELL_SIZE);
    if (val & 8) SDL_RenderDrawLine(renderer, px, py, px, py + CELL_SIZE);
}

void afficher_labyrinthe_sdl(arete arbre[], int nb_aretes, int rows, int cols) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }
    int window_width = cols * CELL_SIZE;
    int window_height = rows * CELL_SIZE;
    SDL_Window* win = SDL_CreateWindow("Labyrinthe",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    int total = rows * cols;
    int *murs = malloc(sizeof(int) * total);
    if (!murs) {
        fprintf(stderr, "Allocation mémoire impossible pour murs\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return;
    }
    for (int i = 0; i < total; i++) murs[i] = 1|2|4|8;
    for (int i = 0; i < nb_aretes; i++) supprimer_mur(murs, cols, arbre[i].u, arbre[i].v);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < rows; y++) for (int x = 0; x < cols; x++) dessiner_murs(renderer, x, y, murs, cols);
    SDL_RenderPresent(renderer);
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
        }
        SDL_Delay(10);
    }
    free(murs);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void afficher_labyrinthe_unicode(int *murs, int rows, int cols) {
    printf("┌");
    for (int x = 0; x < cols; x++) {
        printf("──");
        printf(x < cols - 1 ? "┬" : "┐");
    }
    printf("\n");
    for (int y = 0; y < rows; y++) {
        printf("│");
        for (int x = 0; x < cols; x++) {
            printf("  ");
            int val = murs[y * cols + x];
            printf(val & 2 ? "│" : " ");
        }
        printf("\n");
        if (y < rows - 1) {
            printf("├");
            for (int x = 0; x < cols; x++) {
                int val = murs[y * cols + x];
                printf(val & 4 ? "──" : "  ");
                printf(x < cols - 1 ? "┼" : "┤");
            }
            printf("\n");
        }
    }
    printf("└");
    for (int x = 0; x < cols; x++) {
        printf("──");
        printf(x < cols - 1 ? "┴" : "┘");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <rows> <cols>\n", argv[0]);
        return 1;
    }
    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);
    if (rows <= 0 || cols <= 0) {
        fprintf(stderr, "Dimensions invalides: rows et cols doivent être > 0\n");
        return 1;
    }
    // Choisir mode d'affichage : true pour SDL, false pour console
    bool use_sdl = false; // <--- modifier ici selon besoin

    int total_cells = rows * cols;
    arete *graphe;
    int edge_count = generation_grille_vide(&graphe, rows, cols);
    fisher_yates(graphe, edge_count);
    arete *arbre = malloc(sizeof(arete) * total_cells);
    if (!arbre) {
        fprintf(stderr, "Allocation mémoire impossible pour arbre\n");
        free(graphe);
        return 1;
    }
    int tree_count;
    construire_arbre_couvrant(graphe, edge_count, arbre, &tree_count, total_cells);
    generer_dot("graphe.dot", graphe, edge_count);
    generer_dot("arbre.dot", arbre, tree_count);
    printf("Fichiers DOT générés : graphe.dot et arbre.dot\n");

    int *murs = malloc(sizeof(int) * total_cells);
    if (!murs) {
        fprintf(stderr, "Allocation mémoire impossible pour murs\n");
        free(graphe);
        free(arbre);
        return 1;
    }
    for (int i = 0; i < total_cells; i++) murs[i] = 1|2|4|8;
    for (int i = 0; i < tree_count; i++) {
        int u = arbre[i].u;
        int v = arbre[i].v;
        int x1 = u % cols, y1 = u / cols;
        int x2 = v % cols, y2 = v / cols;
        int dx = x2 - x1;
        int dy = y2 - y1;
        int idx1 = y1 * cols + x1;
        int idx2 = y2 * cols + x2;
        if (dx == 1) { murs[idx1] &= ~2; murs[idx2] &= ~8; }
        else if (dx == -1) { murs[idx1] &= ~8; murs[idx2] &= ~2; }
        else if (dy == 1) { murs[idx1] &= ~4; murs[idx2] &= ~1; }
        else if (dy == -1) { murs[idx1] &= ~1; murs[idx2] &= ~4; }
    }

    if (use_sdl) {
        afficher_labyrinthe_sdl(arbre, tree_count, rows, cols);
    } else {
        afficher_labyrinthe_unicode(murs, rows, cols);
    }

    free(graphe);
    free(arbre);
    free(murs);
    return 0;
}
