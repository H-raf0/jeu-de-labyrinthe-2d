#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "labySDL.h"
#include "laby.h"


SDL_Rect src_murs[16] = {
    {1 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 0 : rien
    {1 * TUILE_TAILLE, 0 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 1 : haut
    {2 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 2 : droite
    {2 * TUILE_TAILLE, 0 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 3 : haut + droite
    {1 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 4 : bas
    {7 * TUILE_TAILLE, 4 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 5 : haut + bas
    {2 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 6 : droite + bas
    {8 * TUILE_TAILLE, 4 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 7 : haut + droite + bas
    {0 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 8 : gauche
    {0 * TUILE_TAILLE, 0 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 9 : haut + gauche
    {8 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 10 : droite + gauche
    {8 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 11 : haut + droite + gauche
    {0 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 12 : bas + gauche
    {6 * TUILE_TAILLE, 4 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 13 : haut + bas + gauche
    {8 * TUILE_TAILLE, 3 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 14 : droite + bas + gauche
    {7 * TUILE_TAILLE, 3 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}  // 15 : tous les murs
};


//============================== SDL =================================================================

// Dessine les murs d'une cellule donnée avec SDL
void dessiner_murs(SDL_Renderer* rendu, int x, int y, int *murs, int colonnes) {
    int px = x * TAILLE_CELLULE;
    int py = y * TAILLE_CELLULE;
    int val = murs[y * colonnes + x];
    if (val & 1) SDL_RenderDrawLine(rendu, px, py, px + TAILLE_CELLULE, py); // haut
    if (val & 2) SDL_RenderDrawLine(rendu, px + TAILLE_CELLULE, py, px + TAILLE_CELLULE, py + TAILLE_CELLULE); // droite
    if (val & 4) SDL_RenderDrawLine(rendu, px, py + TAILLE_CELLULE, px + TAILLE_CELLULE, py + TAILLE_CELLULE); // bas
    if (val & 8) SDL_RenderDrawLine(rendu, px, py, px, py + TAILLE_CELLULE); // gauche
}

// Affiche le labyrinthe avec SDL
void afficher_labyrinthe_sdl(arete arbre[], int nb_aretes, int lignes, int colonnes) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }
    int largeur = colonnes * TAILLE_CELLULE;
    int hauteur = lignes * TAILLE_CELLULE;
    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        largeur+1, hauteur+1, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
    SDL_RenderClear(rendu);
    int total = lignes * colonnes;
    int *murs = malloc(sizeof(int) * total);
    if (!murs) {
        fprintf(stderr, "Allocation mémoire impossible pour murs\n");
        SDL_DestroyRenderer(rendu);
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        return;
    }
    for (int i = 0; i < total; i++) murs[i] = 1|2|4|8; // = 15
    for (int i = 0; i < nb_aretes; i++) supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);
    SDL_SetRenderDrawColor(rendu, 0, 255, 0, 255);
    for (int y = 0; y < lignes; y++)
        for (int x = 0; x < colonnes; x++)
            dessiner_murs(rendu, x, y, murs, colonnes);
    SDL_RenderPresent(rendu);
    SDL_Event e;
    int quitter = 0;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quitter = 1;
        }
        SDL_Delay(10);
    }
    free(murs);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}


// Ajoute les bordures hautes et gauches dans la fonction dessiner_tuile
void dessiner_tuile(SDL_Renderer* rendu, SDL_Texture* tileset, int* murs, int x, int y, int colonnes) {
    SDL_Rect dst;
    SDL_Rect src;
    int val_murs = murs[y * colonnes + x];
    src = src_murs[val_murs];

    dst.x = x * TAILLE_CELLULE;
    dst.y = y * TAILLE_CELLULE;
    dst.w = TAILLE_CELLULE;
    dst.h = TAILLE_CELLULE;
    
    SDL_RenderCopy(rendu, tileset, &src, &dst);
}



void afficher_labyrinthe_sdl_tuiles(int *murs, int lignes, int colonnes) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }

    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe Tuiles",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    

    // Charger l'image comme surface
    SDL_Surface* tileset_surface = IMG_Load("tileset.png");
    if (!tileset_surface) {
        fprintf(stderr, "Erreur chargement tileset.png : %s\n", IMG_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Créer une texture depuis la surface
    SDL_Texture* tileset = SDL_CreateTextureFromSurface(rendu, tileset_surface);
    if (!tileset) {
        fprintf(stderr, "Erreur création texture : %s\n", SDL_GetError());
        SDL_FreeSurface(tileset_surface);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Libérer la surface car plus nécessaire
    SDL_FreeSurface(tileset_surface);

    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
    SDL_RenderClear(rendu);


    for (int y = 0; y < lignes ; y++) {
        for (int x = 0; x < colonnes; x++) {
            dessiner_tuile(rendu, tileset, murs, x, y, colonnes);
        }
    }


    SDL_RenderPresent(rendu);
    SDL_Event e;
    int quitter = 0;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quitter = 1;
        }
        SDL_Delay(10);
    }

    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}

// ======================================================= fin Sdl ========================================
