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


void afficher_labyrinthe_resolu_sdl(int *murs, int lignes, int colonnes, int depart, int destination) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }

    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe Résolu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    
    // 1. Lancer BFS depuis la destination pour calculer toutes les distances
    noeud n;
    int nb_cellules = lignes * colonnes;
    BFS_laby(murs, lignes, colonnes, destination, &n);

    // Trouver la distance maximale pour le dégradé
    int max_dist = 0;
    for (int i = 0; i < nb_cellules; i++) {
        if (n.distance[i] > max_dist) {
            max_dist = n.distance[i];
        }
    }

    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
    SDL_RenderClear(rendu);

    // 2. Dessiner le dégradé de couleurs
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int i = y * colonnes + x;
            if (n.distance[i] != INF) {
                // Ratio de distance : 0.0 (près) à 1.0 (loin)
                float ratio = (float)n.distance[i] / max_dist;
                Uint8 r = 255 * ratio;         // Devient rouge quand on s'éloigne
                Uint8 b = 255 * (1.0f - ratio); // Devient bleu quand on s'approche
                SDL_Rect case_rect = {x * TAILLE_CELLULE, y * TAILLE_CELLULE, TAILLE_CELLULE, TAILLE_CELLULE};
                SDL_SetRenderDrawColor(rendu, r, 50, b, 255); // On garde un peu de vert pour éviter le noir
                SDL_RenderFillRect(rendu, &case_rect);
            }
        }
    }

        // 3. Dessiner le chemin du départ à la destination avec des lignes connectées
    if (n.distance[depart] != INF) { // Si le départ est atteignable
        SDL_SetRenderDrawColor(rendu, 255, 255, 0, 255); // Couleur du chemin : Jaune 

        int courant = depart;
        while (courant != destination) {
            int parent = n.parent[courant];
            if (parent == -1) break; // Sécurité, si on atteint le bout du chemin

            int x1, y1, x2, y2;
            indice_vers_coord(courant, colonnes, &x1, &y1);
            indice_vers_coord(parent, colonnes, &x2, &y2);

            // Calculer les centres des cellules
            int centre_x1 = x1 * TAILLE_CELLULE + TAILLE_CELLULE / 2;
            int centre_y1 = y1 * TAILLE_CELLULE + TAILLE_CELLULE / 2;
            int centre_x2 = x2 * TAILLE_CELLULE + TAILLE_CELLULE / 2;
            int centre_y2 = y2 * TAILLE_CELLULE + TAILLE_CELLULE / 2;
            
            // Pour une ligne plus épaisse, on en dessine plusieurs côte à côte
            SDL_RenderDrawLine(rendu, centre_x1, centre_y1, centre_x2, centre_y2);
            SDL_RenderDrawLine(rendu, centre_x1-1, centre_y1-1, centre_x2-1, centre_y2-1);
            SDL_RenderDrawLine(rendu, centre_x1+1, centre_y1+1, centre_x2+1, centre_y2+1);


            courant = parent;
        }
    }

    // 4. Dessiner les murs par-dessus
    SDL_SetRenderDrawColor(rendu, 255, 255, 255, 255); // Murs en blanc
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            dessiner_murs(rendu, x, y, murs, colonnes);
        }
    }

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
    
    // Libération de la mémoire
    free_noeuds(&n);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}

void dessiner_fond_bfs(SDL_Renderer* rendu, noeud* n, int lignes, int colonnes) {
    int max_dist = 0;
    for (int i = 0; i < lignes * colonnes; i++) {
        if (n->distance[i] > max_dist) max_dist = n->distance[i];
    }
    if (max_dist == 0) max_dist = 1;

    for (int i = 0; i < lignes * colonnes; i++) {
        if (n->distance[i] != INF) {
            float ratio = (float)n->distance[i] / max_dist;
            // Un dégradé très sombre et subtil de bleu à rouge
            Uint8 r = 255 * ratio;
            Uint8 b = 255 * (1.0f - ratio);
            SDL_Rect case_rect = {(i % colonnes) * TAILLE_CELLULE, (i / colonnes) * TAILLE_CELLULE, TAILLE_CELLULE, TAILLE_CELLULE};
            SDL_SetRenderDrawColor(rendu, r, 20, b, 255);
            SDL_RenderFillRect(rendu, &case_rect);
        }
    }
}

// Dessine le plus court chemin en surbrillance
void dessiner_chemin(SDL_Renderer* rendu, int* chemin, int nb_etapes, int colonnes) {
    SDL_SetRenderDrawColor(rendu, 255, 255, 0, 100); // Jaune semi-transparent
    for (int i = 0; i < nb_etapes; i++) {
        int cell = chemin[i];
        SDL_Rect case_rect = {(cell % colonnes) * TAILLE_CELLULE, (cell / colonnes) * TAILLE_CELLULE, TAILLE_CELLULE, TAILLE_CELLULE};
        SDL_RenderFillRect(rendu, &case_rect);
    }
}

// Dessine les marqueurs de départ (vert) et d'arrivée (rouge)
void dessiner_marqueurs(SDL_Renderer* rendu, int depart, int destination, int colonnes) {
    // Départ en vert
    SDL_Rect depart_rect = {(depart % colonnes) * TAILLE_CELLULE, (depart / colonnes) * TAILLE_CELLULE, TAILLE_CELLULE, TAILLE_CELLULE};
    SDL_SetRenderDrawColor(rendu, 0, 255, 0, 150);
    SDL_RenderFillRect(rendu, &depart_rect);

    // Destination en rouge
    SDL_Rect dest_rect = {(destination % colonnes) * TAILLE_CELLULE, (destination / colonnes) * TAILLE_CELLULE, TAILLE_CELLULE, TAILLE_CELLULE};
    SDL_SetRenderDrawColor(rendu, 255, 0, 0, 150);
    SDL_RenderFillRect(rendu, &dest_rect);
}

// Dessine le sprite du personnage à sa position en pixels
void dessiner_personnage(SDL_Renderer* rendu, SDL_Texture* perso_texture, float x_pixel, float y_pixel) {
    SDL_Rect dst_rect = {
        (int)(x_pixel - TAILLE_CELLULE / 2),
        (int)(y_pixel - TAILLE_CELLULE / 2),
        TAILLE_CELLULE,
        TAILLE_CELLULE
    };
    SDL_RenderCopy(rendu, perso_texture, NULL, &dst_rect);
}

void dessiner_noeuds_explores(SDL_Renderer* rendu, noeud* n, int lignes, int colonnes) {
    // 1. Définir une couleur fixe pour toutes les lignes d'exploration.
    // Un bleu-vert (cyan) est un excellent choix sur un fond noir/bleu foncé.
    const Uint8 R_COULEUR = 0;
    const Uint8 G_COULEUR = 200;
    const Uint8 B_COULEUR = 255;
    const Uint8 A_COULEUR = 150; // Transparence pour un effet de "lueur"

    // 2. Activer le blending pour que les lignes qui se superposent s'illuminent.
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rendu, R_COULEUR, G_COULEUR, B_COULEUR, A_COULEUR);

    // 3. Parcourir toutes les cellules.
    for (int i = 0; i < lignes * colonnes; i++) {
        // Si la cellule 'i' a été découverte et a un parent
        if (n->parent[i] != -1) {
            int parent_idx = n->parent[i];

            // Calcul des coordonnées des centres
            int x_enfant, y_enfant, x_parent, y_parent;
            indice_vers_coord(i, colonnes, &x_enfant, &y_enfant);
            indice_vers_coord(parent_idx, colonnes, &x_parent, &y_parent);
            int centre_x_enfant = x_enfant * TAILLE_CELLULE + TAILLE_CELLULE / 2;
            int centre_y_enfant = y_enfant * TAILLE_CELLULE + TAILLE_CELLULE / 2;
            int centre_x_parent = x_parent * TAILLE_CELLULE + TAILLE_CELLULE / 2;
            int centre_y_parent = y_parent * TAILLE_CELLULE + TAILLE_CELLULE / 2;
            
            // Dessiner la ligne centrale
            SDL_RenderDrawLine(rendu, centre_x_enfant, centre_y_enfant, centre_x_parent, centre_y_parent);
            
            // Dessiner les lignes décalées pour l'épaisseur
            SDL_RenderDrawLine(rendu, centre_x_enfant - 1, centre_y_enfant, centre_x_parent - 1, centre_y_parent);
            SDL_RenderDrawLine(rendu, centre_x_enfant + 1, centre_y_enfant, centre_x_parent + 1, centre_y_parent);
            SDL_RenderDrawLine(rendu, centre_x_enfant, centre_y_enfant - 1, centre_x_parent, centre_y_parent - 1);
            SDL_RenderDrawLine(rendu, centre_x_enfant, centre_y_enfant + 1, centre_x_parent, centre_y_parent + 1);
        }
    }

    // 4. Désactiver le blend mode.
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_NONE);
}
// ======================================================= fin Sdl ========================================
