#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// --- CONSTANTES POUR LE RENDU ---
#define TUILE_TAILLE_SOURCE 48
#define TAILLE_CELLULE_ECRAN 64 // Taille équilibrée pour un bon visuel
#define MUR_EPAISSEUR 5         // Épaisseur des murs

// --- Fonctions de l'algorithme (INCHANGÉES) ---
#pragma region Algorithme de Labyrinthe
typedef struct { int *parent; int *rang; int taille; } partition;
typedef struct { int u, v; } arete;

void init_partition(partition* p, int total) {
    p->taille = total;
    p->parent = malloc(sizeof(int) * total);
    p->rang = malloc(sizeof(int) * total);
    for (int i = 0; i < total; i++) { p->parent[i] = i; p->rang[i] = 0; }
}

void free_partition(partition* p) { free(p->parent); free(p->rang); }

int recuperer_classe(partition* p, int i) {
    if (p->parent[i] != i) p->parent[i] = recuperer_classe(p, p->parent[i]);
    return p->parent[i];
}

int fusion(partition* p, int i, int j) {
    int ri = recuperer_classe(p, i);
    int rj = recuperer_classe(p, j);
    if (ri != rj) {
        if (p->rang[ri] < p->rang[rj]) { p->parent[ri] = rj; }
        else if (p->rang[ri] > p->rang[rj]) { p->parent[rj] = ri; }
        else { p->parent[rj] = ri; p->rang[ri]++; }
        return 1;
    }
    return 0;
}

void fisher_yates(arete G[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        arete tmp = G[i]; G[i] = G[j]; G[j] = tmp;
    }
}

int generation_grille_vide(arete **G_ptr, int lignes, int colonnes) {
    int max_aretes = 2 * lignes * colonnes - lignes - colonnes;
    arete *G = malloc(sizeof(arete) * max_aretes);
    int compteur = 0;
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int i = y * colonnes + x;
            if (y + 1 < lignes) G[compteur++] = (arete){i, (y + 1) * colonnes + x};
            if (x + 1 < colonnes) G[compteur++] = (arete){i, y * colonnes + (x + 1)};
        }
    }
    *G_ptr = G;
    return compteur;
}

void construire_arbre_couvrant(arete G[], int nb_aretes, arete *arbre, int* nb_arbre, int nb_cellules) {
    partition p;
    init_partition(&p, nb_cellules);
    *nb_arbre = 0;
    for (int i = 0; i < nb_aretes; i++) {
        if (fusion(&p, G[i].u, G[i].v)) {
            arbre[(*nb_arbre)++] = G[i];
            if (*nb_arbre >= nb_cellules - 1) break;
        }
    }
    free_partition(&p);
}

void indice_vers_coord(int indice, int colonnes, int* x, int* y) {
    *y = indice / colonnes;
    *x = indice % colonnes;
}

void supprimer_mur(int *murs, int colonnes, int u, int v) {
    int x1, y1, x2, y2;
    indice_vers_coord(u, colonnes, &x1, &y1);
    indice_vers_coord(v, colonnes, &x2, &y2);
    int dx = x2 - x1, dy = y2 - y1;
    if (dx == 1) { murs[u] &= ~2; murs[v] &= ~8; }
    else if (dx == -1) { murs[u] &= ~8; murs[v] &= ~2; }
    else if (dy == 1) { murs[u] &= ~4; murs[v] &= ~1; }
    else if (dy == -1) { murs[u] &= ~1; murs[v] &= ~4; }
}
#pragma endregion

//============================== SDL (VERSION FINALE ET PROPRE) =================================

/**
 * @brief Affiche le labyrinthe sans plantes et avec des murs parfaitement joints.
 */
void afficher_labyrinthe_sdl_tuiles(int *murs, int lignes, int colonnes) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe Propre",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        colonnes * TAILLE_CELLULE_ECRAN, lignes * TAILLE_CELLULE_ECRAN, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* tileset = IMG_LoadTexture(rendu, "tileset.png");
    if (!fenetre || !rendu || !tileset) {
        printf("Erreur d'initialisation SDL: %s\n", SDL_GetError());
        return;
    }

    // --- Définition des tuiles utilisées ---
    SDL_Rect src_sol = {1 * TUILE_TAILLE_SOURCE, 2 * TUILE_TAILLE_SOURCE, TUILE_TAILLE_SOURCE, TUILE_TAILLE_SOURCE};
    SDL_Rect src_mur = {0 * TUILE_TAILLE_SOURCE, 3 * TUILE_TAILLE_SOURCE, TUILE_TAILLE_SOURCE, TUILE_TAILLE_SOURCE};
    
    // Aucune plante n'est définie.

    SDL_RenderClear(rendu);

    // === PASSE 1: Dessiner le sol partout ===
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            SDL_Rect dst = {x * TAILLE_CELLULE_ECRAN, y * TAILLE_CELLULE_ECRAN, TAILLE_CELLULE_ECRAN, TAILLE_CELLULE_ECRAN};
            SDL_RenderCopy(rendu, tileset, &src_sol, &dst);
        }
    }

    // === PASSE 2: Dessiner les murs (lignes droites) ===
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int val_murs = murs[y * colonnes + x];
            if (val_murs & 1) { // Mur HAUT
                SDL_Rect dst = {x * TAILLE_CELLULE_ECRAN, y * TAILLE_CELLULE_ECRAN, TAILLE_CELLULE_ECRAN, MUR_EPAISSEUR};
                SDL_RenderCopy(rendu, tileset, &src_mur, &dst);
            }
            if (val_murs & 2) { // Mur DROIT
                SDL_Rect dst = {(x + 1) * TAILLE_CELLULE_ECRAN - MUR_EPAISSEUR, y * TAILLE_CELLULE_ECRAN, MUR_EPAISSEUR, TAILLE_CELLULE_ECRAN};
                SDL_RenderCopy(rendu, tileset, &src_mur, &dst);
            }
            if (val_murs & 4) { // Mur BAS
                SDL_Rect dst = {x * TAILLE_CELLULE_ECRAN, (y + 1) * TAILLE_CELLULE_ECRAN - MUR_EPAISSEUR, TAILLE_CELLULE_ECRAN, MUR_EPAISSEUR};
                SDL_RenderCopy(rendu, tileset, &src_mur, &dst);
            }
            if (val_murs & 8) { // Mur GAUCHE
                SDL_Rect dst = {x * TAILLE_CELLULE_ECRAN, y * TAILLE_CELLULE_ECRAN, MUR_EPAISSEUR, TAILLE_CELLULE_ECRAN};
                SDL_RenderCopy(rendu, tileset, &src_mur, &dst);
            }
        }
    }
    
    // === PASSE 3: Remplir les coins pour joindre les murs parfaitement (corrige les "gaps") ===
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int val_murs = murs[y * colonnes + x];
            // Si mur en haut ET à gauche, remplir le coin haut-gauche
            if ((val_murs & 1) && (val_murs & 8)) {
                SDL_Rect dst_coin = {x*TAILLE_CELLULE_ECRAN, y*TAILLE_CELLULE_ECRAN, MUR_EPAISSEUR, MUR_EPAISSEUR};
                SDL_RenderCopy(rendu, tileset, &src_mur, &dst_coin);
            }
            // Si mur en haut ET à droite...
            if ((val_murs & 1) && (val_murs & 2)) {
                SDL_Rect dst_coin = {(x+1)*TAILLE_CELLULE_ECRAN - MUR_EPAISSEUR, y*TAILLE_CELLULE_ECRAN, MUR_EPAISSEUR, MUR_EPAISSEUR};
                SDL_RenderCopy(rendu, tileset, &src_mur, &dst_coin);
            }
            // etc. pour les autres coins
            if ((val_murs & 4) && (val_murs & 8)) {
                SDL_Rect dst_coin = {x*TAILLE_CELLULE_ECRAN, (y+1)*TAILLE_CELLULE_ECRAN - MUR_EPAISSEUR, MUR_EPAISSEUR, MUR_EPAISSEUR};
                SDL_RenderCopy(rendu, tileset, &src_mur, &dst_coin);
            }
            if ((val_murs & 4) && (val_murs & 2)) {
                SDL_Rect dst_coin = {(x+1)*TAILLE_CELLULE_ECRAN - MUR_EPAISSEUR, (y+1)*TAILLE_CELLULE_ECRAN - MUR_EPAISSEUR, MUR_EPAISSEUR, MUR_EPAISSEUR};
                SDL_RenderCopy(rendu, tileset, &src_mur, &dst_coin);
            }
        }
    }

    // Aucune passe pour les plantes. Elles ont été totalement supprimées.

    SDL_RenderPresent(rendu);

    // Boucle d'événements
    SDL_Event e;
    int quitter = 0;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quitter = 1;
        }
        SDL_Delay(10);
    }

    SDL_DestroyTexture(tileset);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    IMG_Quit();
    SDL_Quit();
}


// --- Fonction principale (INCHANGÉE) ---
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <lignes> <colonnes> [mode]\n", argv[0]);
        printf("  mode (optionnel): 1 pour graphique (par défaut)\n");
        return 1;
    }
    int lignes = atoi(argv[1]);
    int colonnes = atoi(argv[2]);
    int utiliser_sdl = (argc > 3) ? atoi(argv[3]) : 1;

    if (lignes <= 0 || colonnes <= 0) {
        fprintf(stderr, "Dimensions invalides : lignes et colonnes > 0\n");
        return 1;
    }

    srand(time(NULL));
    int total_cellules = lignes * colonnes;
    arete *graphe;
    int nb_aretes = generation_grille_vide(&graphe, lignes, colonnes);
    fisher_yates(graphe, nb_aretes);

    arete *arbre = malloc(sizeof(arete) * (total_cellules - 1));
    int nb_arbre;
    construire_arbre_couvrant(graphe, nb_aretes, arbre, &nb_arbre, total_cellules);

    int *murs = malloc(sizeof(int) * total_cellules);
    for (int i = 0; i < total_cellules; i++) murs[i] = 1 | 2 | 4 | 8;
    for (int i = 0; i < nb_arbre; i++) {
        supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);
    }

    if (utiliser_sdl) {
        afficher_labyrinthe_sdl_tuiles(murs, lignes, colonnes);
    } else {
        printf("Mode console non supporté dans cette version.\n");
    }

    free(graphe);
    free(arbre);
    free(murs);
    return 0;
}