#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define LIGNES 20
#define COLONNES 20
#define CELL 30
#define LARGEUR (COLONNES * CELL)
#define HAUTEUR (LIGNES * CELL)
#define N (LIGNES * COLONNES)
#define INF 1000000000

// Structures pour la génération du labyrinthe et Dijkstra
typedef struct {
    int u, v;
} Arete;

typedef struct {
    int *parent, *rang;
    int taille;
} Partition;

typedef struct noeud {
    int distance[N];
    int visite[N];
} noeud;

typedef struct tas {
    int tab[N];
    int taille;
} tas;

// Fonctions de génération du labyrinthe
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
            if (y + 1 < lignes) G[k++] = (Arete){i, i + colonnes};
        }
    }
    *G_ptr = G;
    return k;
}

void supprimer_mur(int* murs, int cols, int u, int v) {
    int x1 = u % cols, y1 = u / cols;
    int x2 = v % cols, y2 = v / cols;
    if (x2 == x1 + 1) { murs[u] &= ~2; murs[v] &= ~8; }
    else if (x2 == x1 - 1) { murs[u] &= ~8; murs[v] &= ~2; }
    else if (y2 == y1 + 1) { murs[u] &= ~4; murs[v] &= ~1; }
    else if (y2 == y1 - 1) { murs[u] &= ~1; murs[v] &= ~4; }
}

// Fonctions de recherche de chemin (Dijkstra)
void echanger(int* a, int* b) {
    int tmp = *a; *a = *b; *b = tmp;
}

void inserer(tas* t, int sommet, noeud* n) {
    t->tab[t->taille++] = sommet;
    int i = t->taille - 1;
    while (i > 0 && n->distance[t->tab[i]] < n->distance[t->tab[(i - 1) / 2]]) {
        echanger(&t->tab[i], &t->tab[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

int extraire_min(tas* t, noeud* n) {
    if (t->taille == 0) return -1;
    int min = t->tab[0];
    t->tab[0] = t->tab[--t->taille];
    int i = 0;
    while (2 * i + 1 < t->taille) {
        int gauche = 2 * i + 1;
        int droite = 2 * i + 2;
        int plus_petit = gauche;
        if (droite < t->taille && n->distance[t->tab[droite]] < n->distance[t->tab[gauche]]) {
            plus_petit = droite;
        }
        if (n->distance[t->tab[i]] <= n->distance[t->tab[plus_petit]]) break;
        echanger(&t->tab[i], &t->tab[plus_petit]);
        i = plus_petit;
    }
    return min;
}

void initialiser_noeuds(noeud* n, int origine) {
    for (int i = 0; i < N; ++i) {
        n->visite[i] = 0;
        n->distance[i] = INF;
    }
    n->distance[origine] = 0;
}

void dijkstra(int graphe[N][N], int origine, noeud* n) {
    tas t; t.taille = 0;
    initialiser_noeuds(n, origine);
    inserer(&t, origine, n);

    while (t.taille > 0) {
        int u = extraire_min(&t, n);
        if (u == -1 || n->visite[u]) continue;
        n->visite[u] = 1;

        for (int v = 0; v < N; v++) {
            if (graphe[u][v] == 0) continue;

            if (!n->visite[v]) {
                int alt = n->distance[u] + 1;
                if (alt < n->distance[v]) {
                    n->distance[v] = alt;
                    inserer(&t, v, n);
                }
            }
        }
    }
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Labyrinthe - Recherche du plus court chemin",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       LARGEUR, HAUTEUR, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    srand(time(NULL));

    // Génération du labyrinthe
    int nb_cells = LIGNES * COLONNES;
    int* murs = malloc(sizeof(int) * nb_cells);
    for (int i = 0; i < nb_cells; i++) murs[i] = 15;

    Arete* G_aretes;
    int nb_aretes = generer_aretes(&G_aretes, LIGNES, COLONNES);
    shuffle(G_aretes, nb_aretes);

    Partition p;
    init_partition(&p, nb_cells);

    for (int i = 0, count = 0; i < nb_aretes && count < nb_cells - 1; i++) {
        if (union_sets(&p, G_aretes[i].u, G_aretes[i].v)) {
            supprimer_mur(murs, COLONNES, G_aretes[i].u, G_aretes[i].v);
            count++;
        }
    }

    // Construction du graphe
    int graphe[N][N] = {0};
    for (int i = 0; i < N; i++) {
        int x = i % COLONNES, y = i / COLONNES;
        if (x < COLONNES - 1 && !(murs[i] & 2)) {
            graphe[i][i + 1] = graphe[i + 1][i] = 1;
        }
        if (y < LIGNES - 1 && !(murs[i] & 4)) {
            graphe[i][i + COLONNES] = graphe[i + COLONNES][i] = 1;
        }
    }

    // Initialisation de la recherche
    int origine = 0;
    int destination = N - 1;
    int courant = origine;

    bool chemin_trouve = false;
    int chemin_parcouru[N] = {0};
    chemin_parcouru[origine] = 1;

    noeud n;

    // Boucle principale
    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }

        if (!chemin_trouve && courant != destination) {
            dijkstra(graphe, destination, &n);

            int meilleur_voisin = -1;
            int distance_min = INF;

            for (int v = 0; v < N; v++) {
                if (graphe[courant][v] == 1) {
                    if (n.distance[v] < distance_min) {
                        distance_min = n.distance[v];
                        meilleur_voisin = v;
                    }
                }
            }

            if (meilleur_voisin != -1) {
                courant = meilleur_voisin;
                chemin_parcouru[courant] = 1;
            } else {
                printf("Agent bloque ! Arret.\n");
                quit = true;
            }
        }

        if (courant == destination && !chemin_trouve) {
            printf("Destination atteinte !\n");
            chemin_trouve = true;
        }

        // Rendu graphique
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        // Chemin parcouru
        SDL_SetRenderDrawColor(renderer, 200, 255, 200, 255);
        for (int i = 0; i < N; i++) {
            if (chemin_parcouru[i]) {
                int x = i % COLONNES;
                int y = i / COLONNES;
                SDL_Rect rect = {x * CELL + 1, y * CELL + 1, CELL - 2, CELL - 2};
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        // Murs du labyrinthe
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int y = 0; y < LIGNES; y++) {
            for (int x = 0; x < COLONNES; x++) {
                int i = y * COLONNES + x;
                int mx = x * CELL;
                int my = y * CELL;
                if (murs[i] & 1) SDL_RenderDrawLine(renderer, mx, my, mx + CELL, my);
                if (murs[i] & 2) SDL_RenderDrawLine(renderer, mx + CELL, my, mx + CELL, my + CELL);
                if (murs[i] & 4) SDL_RenderDrawLine(renderer, mx, my + CELL, mx + CELL, my + CELL);
                if (murs[i] & 8) SDL_RenderDrawLine(renderer, mx, my, mx, my + CELL);
            }
        }

        // Origine (vert)
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);
        SDL_Rect orig_rect = {0, 0, CELL, CELL};
        SDL_RenderFillRect(renderer, &orig_rect);

        // Destination (bleu)
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect dest_rect = {(COLONNES - 1) * CELL, (LIGNES - 1) * CELL, CELL, CELL};
        SDL_RenderFillRect(renderer, &dest_rect);
        
        // Position courante (rouge)
        int x_pos = courant % COLONNES;
        int y_pos = courant / COLONNES;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect current_rect = {x_pos * CELL + 5, y_pos * CELL + 5, CELL - 10, CELL - 10};
        SDL_RenderFillRect(renderer, &current_rect);

        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    // Nettoyage
    free_partition(&p);
    free(murs);
    free(G_aretes);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}