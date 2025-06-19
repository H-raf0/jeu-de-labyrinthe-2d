/*
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <time.h>

#define LARGEUR 10
#define HAUTEUR 10
#define TAILLE_TUILE 32
#define MUR_HAUT    1
#define MUR_DROITE  2
#define MUR_BAS     4
#define MUR_GAUCHE  8

typedef struct {
    int a, b;
} Arete;

int trouver(int parent[], int i) {
    if (parent[i] != i)
        parent[i] = trouver(parent, parent[i]);
    return parent[i];
}

void unir(int parent[], int rang[], int x, int y) {
    int racine_x = trouver(parent, x);
    int racine_y = trouver(parent, y);

    if (racine_x != racine_y) {
        if (rang[racine_x] < rang[racine_y])
            parent[racine_x] = racine_y;
        else if (rang[racine_x] > rang[racine_y])
            parent[racine_y] = racine_x;
        else {
            parent[racine_y] = racine_x;
            rang[racine_x]++;
        }
    }
}

void melanger_aretes(Arete* aretes, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Arete tmp = aretes[i];
        aretes[i] = aretes[j];
        aretes[j] = tmp;
    }
}

void initialiser_grille(int grille[HAUTEUR][LARGEUR]) {
    for (int i = 0; i < HAUTEUR; i++) {
        for (int j = 0; j < LARGEUR; j++) {
            grille[i][j] = MUR_HAUT | MUR_DROITE | MUR_BAS | MUR_GAUCHE;
        }
    }
}

void appliquer_kruskal(int grille[HAUTEUR][LARGEUR]) {
    int taille = HAUTEUR * LARGEUR;
    int parent[taille];
    int rang[taille];
    for (int i = 0; i < taille; i++) rang[i] = 0;


    for (int i = 0; i < taille; i++) parent[i] = i;

    Arete aretes[2 * taille];
    int nb_aretes = 0;

    // Génération des arêtes (voisins droite et bas)
    for (int i = 0; i < HAUTEUR; i++) {
        for (int j = 0; j < LARGEUR; j++) {
            int index = i * LARGEUR + j;
            if (j < LARGEUR - 1) { // droite
                aretes[nb_aretes++] = (Arete){index, index + 1};
            }
            if (i < HAUTEUR - 1) { // bas
                aretes[nb_aretes++] = (Arete){index, index + LARGEUR};
            }
        }
    }

    melanger_aretes(aretes, nb_aretes);

    for (int k = 0; k < nb_aretes; k++) {
        int a = aretes[k].a;
        int b = aretes[k].b;
        int ra = trouver(parent, a);
        int rb = trouver(parent, b);

        if (ra != rb) {
            int ax = a / LARGEUR, ay = a % LARGEUR;
            int bx = b / LARGEUR, by = b % LARGEUR;

            if (a + 1 == b) {
                // droite
                grille[ax][ay] &= ~MUR_DROITE;
                grille[bx][by] &= ~MUR_GAUCHE;
            } else if (a + LARGEUR == b) {
                // bas
                grille[ax][ay] &= ~MUR_BAS;
                grille[bx][by] &= ~MUR_HAUT;
            }

            unir(parent, rang, ra, rb);
        }
    }
}

void afficher_labyrinthe(SDL_Renderer* renderer, SDL_Texture* tileset, int grille[HAUTEUR][LARGEUR]) {
    SDL_Rect src, dest;
    src.w = src.h = dest.w = dest.h = TAILLE_TUILE;

    for (int i = 0; i < HAUTEUR; i++) {
        for (int j = 0; j < LARGEUR; j++) {
            dest.x = j * TAILLE_TUILE;
            dest.y = i * TAILLE_TUILE;

            int tuile = grille[i][j];
            int tx = tuile % 4;
            int ty = tuile / 4;
            src.x = tx * TAILLE_TUILE;
            src.y = ty * TAILLE_TUILE;

            SDL_RenderCopy(renderer, tileset, &src, &dest);
        }
    }
}

int main() {
    srand(time(NULL));

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Labyrinthe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR * TAILLE_TUILE, HAUTEUR * TAILLE_TUILE, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* surface = IMG_Load("tileset 1.png");
    SDL_Texture* tileset = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    int grille[HAUTEUR][LARGEUR];
    initialiser_grille(grille);
    appliquer_kruskal(grille);

    SDL_Event e;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = 1;
        }

        SDL_RenderClear(renderer);
        afficher_labyrinthe(renderer, tileset, grille);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(tileset);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define N 10
#define MAX_EDGES (2 * N * (N - 1))
#define CELL_SIZE 25
#define WINDOW_SIZE (N * CELL_SIZE)
#define N 10

typedef struct {
    int parent[N * N];
    int rang[N * N];
} partition;

typedef struct {
    int u, v;
} arete;

void init_partition(partition* p) {
    for (int i = 0; i < N * N; i++) {
        p->parent[i] = i;
        p->rang[i] = 0;
    }
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
    srand(time(0));
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        arete tmp = G[i];
        G[i] = G[j];
        G[j] = tmp;
    }
}

int generation_grille_vide(arete G[]) {
    int edge_count = 0;
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            int i = x * N + y;
            if (x + 1 < N) G[edge_count++] = (arete){i, (x + 1) * N + y}; // bas
            if (y + 1 < N) G[edge_count++] = (arete){i, x * N + (y + 1)}; // droite
        }
    }
    return edge_count;
}

void construire_arbre_couvrant(arete G[], int edge_count, arete arbre[], int* tree_count) {
    partition p;
    init_partition(&p);
    *tree_count = 0;
    for (int i = 0; i < edge_count; i++) {
        if (fusion(&p, G[i].u, G[i].v)) {
            arbre[*tree_count] = G[i];
            (*tree_count)++;
        }
    }
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

void index_to_coord(int index, int* x, int* y) {
    *x = index % N;
    *y = index / N;
}

void supprimer_mur(int murs[N][N], int u, int v) {
    int x1, y1, x2, y2;
    index_to_coord(u, &x1, &y1);
    index_to_coord(v, &x2, &y2);
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx == 1) {
        murs[y1][x1] -= 2;
        murs[y2][x2] -= 8;
    } else if (dx == -1) {
        murs[y1][x1] -= 8;
        murs[y2][x2] -= 2;
    } else if (dy == 1) {
        murs[y1][x1] -= 4;
        murs[y2][x2] -= 1;
    } else if (dy == -1) {
        murs[y1][x1] -= 1;
        murs[y2][x2] -= 4;
    }
}

void dessiner_murs(SDL_Renderer* renderer, int x, int y, int murs[N][N]) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;

    if (murs[y][x] & 1) SDL_RenderDrawLine(renderer, px, py, px + CELL_SIZE, py); // haut
    if (murs[y][x] & 2) SDL_RenderDrawLine(renderer, px + CELL_SIZE, py, px + CELL_SIZE, py + CELL_SIZE); // droite
    if (murs[y][x] & 4) SDL_RenderDrawLine(renderer, px, py + CELL_SIZE, px + CELL_SIZE, py + CELL_SIZE); // bas
    if (murs[y][x] & 8) SDL_RenderDrawLine(renderer, px, py, px, py + CELL_SIZE); // gauche
}

void afficher_labyrinthe(arete arbre[], int nb_aretes) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }

    SDL_Window* win = SDL_CreateWindow("Labyrinthe",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // fond noir
    SDL_RenderClear(renderer);

    int murs[N][N];
    for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++)
            murs[y][x] = 15; // tous les murs présents

    for (int i = 0; i < nb_aretes; i++)
        supprimer_mur(murs, arbre[i].u, arbre[i].v);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // murs en blanc
    for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++)
            dessiner_murs(renderer, x, y, murs);

    SDL_RenderPresent(renderer);

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main() {
    arete graphe[MAX_EDGES], arbre[MAX_EDGES];
    int edge_count = generation_grille_vide(graphe);
    fisher_yates(graphe, edge_count);

    int tree_count;
    construire_arbre_couvrant(graphe, edge_count, arbre, &tree_count);

    generer_dot("graphe.dot", graphe, edge_count);
    generer_dot("arbre.dot", arbre, tree_count);
    printf("Fichiers DOT générés : graphe.dot et arbre.dot\n");

    afficher_labyrinthe(arbre, tree_count);
    return 0;
}