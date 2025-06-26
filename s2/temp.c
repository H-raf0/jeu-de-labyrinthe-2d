
labySDL.h
#ifndef LABYSDL_H
#define LABYSDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "laby.h"
#include "structures.h"
// Constantes
#define TUILE_TAILLE 16
#define TAILLE_CELLULE 50

#define EPES 3 // epesseur des murs 

// Tableau global contenant les zones du tileset
extern SDL_Rect src_murs[16];

// Fonctions pour afficher un labyrinthe en SDL
void dessiner_murs_connus(SDL_Renderer* rendu, int x, int y, int *murs, int colonnes); 
void afficher_labyrinthe_sdl(arete arbre[], int nb_aretes, int lignes, int colonnes);

//void dessiner_bg(SDL_Renderer* rendu, int *murs, int lignes, int colonnes);

// Version avec tuiles
void dessiner_tuile(SDL_Renderer* rendu, SDL_Texture* tileset, int* murs, int x, int y, int colonnes);
void dessiner_tuile_v2(SDL_Renderer* rendu, SDL_Texture* tileset, int x, int y);
void afficher_labyrinthe_sdl_tuiles(int *murs, int lignes, int colonnes);
void dessiner_bg(SDL_Renderer* rendu, int lignes, int colonnes);


// Fonctions de dessin modulaires pour l'animation
void dessiner_fond(SDL_Renderer* rendu, noeud* n, int lignes, int colonnes);
void dessiner_chemin(SDL_Renderer* rendu, int* chemin, int nb_etapes, int colonnes);
void dessiner_personnage(SDL_Renderer* rendu, SDL_Texture* perso_texture, float x_pixel, float y_pixel);
void dessiner_marqueurs(SDL_Renderer* rendu, int depart, int destination, int colonnes);
void dessiner_heatmap_passage(SDL_Renderer* rendu, int* passages, int lignes, int colonnes, int max_passages); 
void dessiner_rayon_detection(SDL_Renderer* rendu, int centre_pos, int rayon, int lignes, int colonnes);

//================== Affichage SDL avec solution =================
void afficher_labyrinthe_resolu_sdl(int *murs, int lignes, int colonnes, int depart, int destination);



#endif



labySDL.c
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



void dessiner_murs_connus(SDL_Renderer* rendu, int x, int y, int *murs, int colonnes) {
    // Garder la couleur originale pour la restaurer plus tard si besoin
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(rendu, &r, &g, &b, &a);

    SDL_SetRenderDrawColor(rendu, 255, 165, 0, 255);

    int px = x * TAILLE_CELLULE;
    int py = y * TAILLE_CELLULE;
    int val = murs[y * colonnes + x];
    for(int i=-EPES; i<=EPES; ++i){
        if (val & 1) SDL_RenderDrawLine(rendu, px, py + i, px + TAILLE_CELLULE, py + i);
        if (val & 2) SDL_RenderDrawLine(rendu, px + TAILLE_CELLULE + i, py, px + TAILLE_CELLULE + i, py + TAILLE_CELLULE);
        if (val & 4) SDL_RenderDrawLine(rendu, px, py + TAILLE_CELLULE + i, px + TAILLE_CELLULE, py + TAILLE_CELLULE + i);
        if (val & 8) SDL_RenderDrawLine(rendu, px + i, py, px + i, py + TAILLE_CELLULE);
    }
    // Restaurer la couleur originale
    SDL_SetRenderDrawColor(rendu, r, g, b, a);
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
            dessiner_murs_connus(rendu, x, y, murs, colonnes);
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






void dessiner_tuile_v2(SDL_Renderer* rendu, SDL_Texture* tileset, int x, int y){
    

    SDL_Rect dst; // Le rectangle de destination SUR l'écran
    int p_extra = 0;
    // Définition des rectangles source pour nos deux seules tuiles
    SDL_Rect src_sol = {2 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE};

    // On définit où dessiner la tuile à l'écran et sa taille
    dst.x = x * TAILLE_CELLULE + p_extra;
    dst.y = y * TAILLE_CELLULE + p_extra;
    dst.w = TAILLE_CELLULE - p_extra;
    dst.h = TAILLE_CELLULE - p_extra;

    SDL_RenderCopy(rendu, tileset, &src_sol, &dst);

}


void dessiner_bg(SDL_Renderer* rendu, int lignes, int colonnes) {
    // Charger l'image comme surface
    SDL_Surface* tileset_surface = IMG_Load("tileset2.png");
    if (!tileset_surface) {
        fprintf(stderr, "Erreur chargement tileset2.png : %s\n", IMG_GetError());
        return;
    }

    // Créer une texture depuis la surface
    SDL_Texture* tileset = SDL_CreateTextureFromSurface(rendu, tileset_surface);
    if (!tileset) {
        fprintf(stderr, "Erreur création texture : %s\n", SDL_GetError());
        SDL_FreeSurface(tileset_surface);
        return;
    }

    SDL_FreeSurface(tileset_surface);

    // Fond bleu foncé
    //SDL_SetRenderDrawColor(rendu, 11, 14, 42, 255);
    //SDL_RenderClear(rendu);

    // Boucle pour dessiner chaque tuile
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            dessiner_tuile_v2(rendu, tileset, x, y);
        }
    }

    SDL_DestroyTexture(tileset);
}









void dessiner_rayon_detection(SDL_Renderer* rendu, int centre_pos, int rayon, int lignes, int colonnes) {
    int cx, cy;
    indice_vers_coord(centre_pos, colonnes, &cx, &cy);

    // Activer le mode de dessin "blend" pour gérer la transparence (alpha)
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_BLEND);
    // Choisir une couleur semi-transparente (ici, un rouge très léger)
    SDL_SetRenderDrawColor(rendu, 255, 100, 100, 25);

    // On parcourt un carré de cases autour du monstre
    for (int y = cy - rayon; y <= cy + rayon; y++) {
        for (int x = cx - rayon; x <= cx + rayon; x++) {
            // On s'assure que la case est bien dans les limites du labyrinthe
            if (x >= 0 && x < colonnes && y >= 0 && y < lignes) {
                // On calcule la distance de Manhattan
                int dist = abs(x - cx) + abs(y - cy);
                
                // Si la case est dans le rayon de détection...
                if (dist <= rayon) {
                    // ... on dessine un rectangle de surbrillance dessus
                    SDL_Rect case_rect = {x * TAILLE_CELLULE, y * TAILLE_CELLULE, TAILLE_CELLULE, TAILLE_CELLULE};
                    SDL_RenderFillRect(rendu, &case_rect);
                }
            }
        }
    }

    // Rétablir le mode de dessin par défaut pour ne pas affecter le reste du rendu
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_NONE);
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
            dessiner_murs_connus(rendu, x, y, murs, colonnes);
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


void dessiner_fond(SDL_Renderer* rendu, noeud* n, int lignes, int colonnes) {
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

void dessiner_heatmap_passage(SDL_Renderer* rendu, int* passages, int lignes, int colonnes, int max_passages) {
    if (max_passages == 0) return;

    // Activer le blending pour la transparence
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < lignes * colonnes; i++) {
        if (passages[i] > 0) {
            // Le ratio détermine l'intensité. De 0 (pas passé) à 1 (le plus passé).
            float ratio = (float)passages[i] / max_passages;
            // On utilise la couleur rouge. L'alpha (transparence) crée l'effet de heatmap.
            // 200 est une bonne valeur max pour l'alpha pour ne pas cacher complètement le fond.
            Uint8 alpha = (Uint8)(ratio * 200);
            
            SDL_Rect case_rect = {(i % colonnes) * TAILLE_CELLULE, (i / colonnes) * TAILLE_CELLULE, TAILLE_CELLULE, TAILLE_CELLULE};
            SDL_SetRenderDrawColor(rendu, 255, 100, 0, alpha); // Orange/Rouge
            SDL_RenderFillRect(rendu, &case_rect);
        }
    }
    // Rétablir le mode de dessin par défaut
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_NONE);
}

// ======================================================= fin Sdl ========================================

laby.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "labySDL.h"
#include "laby.h"

#define MAX_COUT 10 // Coût maximal pour traverser une cellule


// Initialise une partition de taille 'total'
void init_partition(partition* p, int total) {
    p->taille = total;
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

// Libère la mémoire d'une partition
void free_partition(partition* p) {
    free(p->parent);
    free(p->rang);
}

// Renvoie l'identifiant de la classe de l'élément i (avec compression de chemin)
int recuperer_classe(partition* p, int i) {
    if (p->parent[i] != i)
        p->parent[i] = recuperer_classe(p, p->parent[i]);
    return p->parent[i];
}

// Fusionne les classes des éléments i et j
int fusion(partition* p, int i, int j) {
    int ri = recuperer_classe(p, i);
    int rj = recuperer_classe(p, j);
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

// Mélange les arêtes aléatoirement (Fisher-Yates)
void fisher_yates(arete G[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        arete tmp = G[i];
        G[i] = G[j];
        G[j] = tmp;
    }
}




// Génère une grille vide avec toutes les arêtes possibles verticales et horizontales
int generation_grille_vide(arete **G_ptr, int lignes, int colonnes) {
    int max_aretes = 2 * lignes * colonnes - lignes - colonnes; // (n-1) * m + (m-1) * n = 2 * n * m - m - n
    arete *G = malloc(sizeof(arete) * max_aretes); // tab de tous les aretes possibles
    if (!G) {
        fprintf(stderr, "Allocation mémoire impossible pour arêtes\n");
        exit(EXIT_FAILURE);
    }
    int compteur = 0;
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int i = y * colonnes + x;  // indice du noeud de coord x,y
            if (y + 1 < lignes) G[compteur++] = (arete){i, (y + 1) * colonnes + x}; // si != dernier ligne alors ajoute l'arete vers le bas
            if (x + 1 < colonnes) G[compteur++] = (arete){i, y * colonnes + (x + 1)}; // si != dernier colonne alors ajoute l'arete vers le droite
        }
    }
    *G_ptr = G;
    return compteur; // nombre des aretes
}

// Construit un arbre couvrant minimal à partir des arêtes (kruskal)
void construire_arbre_couvrant(arete G[], int nb_aretes, arete *arbre, int* nb_arbre, int nb_cellules) {
    partition p;
    init_partition(&p, nb_cellules);
    *nb_arbre = 0;
    for (int i = 0; i < nb_aretes; i++) {
        if (fusion(&p, G[i].u, G[i].v)) { //s il ne sont pas deja dans la meme classe
            arbre[*nb_arbre] = G[i];
            (*nb_arbre)++;
            if (*nb_arbre >= nb_cellules - 1) break;
        }
    }
    free_partition(&p);
}

// Génère un fichier DOT pour visualisation de graphe
void generer_dot(const char* nom, arete aretes[], int nb) {
    FILE* f = fopen(nom, "w");
    if (!f) {
        perror("Erreur ouverture fichier DOT");
        return;
    }
    fprintf(f, "graph G {\n  node [shape=circle];\n");
    for (int i = 0; i < nb; i++) {
        fprintf(f, "  %d -- %d;\n", aretes[i].u, aretes[i].v);
    }
    fprintf(f, "}\n");
    fclose(f);
}

// Convertit un indice en coordonnées x, y
void indice_vers_coord(int indice, int colonnes, int* x, int* y) {
    *y = indice / colonnes;
    *x = indice % colonnes;
}



int** creer_matrice_adjacence_cout_altr(int* murs, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int** matrice = malloc(sizeof(int*) * nb_cellules);
    if (!matrice) return NULL;
    for (int i = 0; i < nb_cellules; i++) {
        matrice[i] = calloc(nb_cellules, sizeof(int));
        if (!matrice[i]) {
            printf("erreur lors de la création de la matrice d'adjacence\n");
            return NULL; 
        }
    }

    // Remplir la matrice
    for (int u = 0; u < nb_cellules; u++) {
        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);

        // Voisin du haut
        if (y > 0 && !(murs[u] & 1) && matrice[u][(y - 1) * colonnes + x] == 0) {
            int v = (y - 1) * colonnes + x;
            int cout_arete = (rand() % MAX_COUT) + 1; // Génère un coût aléatoire pour l'arête
            matrice[u][v] = cout_arete;
            matrice[v][u] = cout_arete; // Assigne le MÊME coût au chemin du retour
        }
        // Voisin de droite
        if (x < colonnes - 1 && !(murs[u] & 2) && matrice[u][y * colonnes + (x + 1)] == 0) {
            int v = y * colonnes + (x + 1);
            int cout_arete = (rand() % MAX_COUT) + 1;
            matrice[u][v] = cout_arete;
            matrice[v][u] = cout_arete;
        }
        // Voisin du bas
        if (y < lignes - 1 && !(murs[u] & 4) && matrice[u][(y + 1) * colonnes + x] == 0) {
            int v = (y + 1) * colonnes + x;
            int cout_arete = (rand() % MAX_COUT) + 1;
            matrice[u][v] = cout_arete;
            matrice[v][u] = cout_arete;
        }
        // Voisin de gauche
        if (x > 0 && !(murs[u] & 8) && matrice[u][y * colonnes + (x - 1)] == 0) {
            int v = y * colonnes + (x - 1);
            int cout_arete = (rand() % MAX_COUT) + 1;
            matrice[u][v] = cout_arete;
            matrice[v][u] = cout_arete;
        }
    }
    return matrice;
}


int** creer_matrice_adjacence_connue(int* murs_connus, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int** matrice = malloc(sizeof(int*) * nb_cellules);
    if (!matrice) return NULL;
    for (int i = 0; i < nb_cellules; i++) {
        matrice[i] = calloc(nb_cellules, sizeof(int));
        if (!matrice[i]) {
            printf("erreur lors de la création de la matrice d'adjacence\n");
            return NULL; 
        }
    }

    // Remplir la matrice
    for (int u = 0; u < nb_cellules; u++) {
        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);
        int v;

        // Si le mur HAUT n'est pas connu, le passage est considéré comme ouvert avec un coût de 1
        if (y > 0 && !(murs_connus[u] & 1)) {
            v = (y - 1) * colonnes + x;
            matrice[u][v] = 1;
            matrice[v][u] = 1;
        }
        // Si le mur DROIT n'est pas connu...
        if (x < colonnes - 1 && !(murs_connus[u] & 2)) {
            v = y * colonnes + (x + 1);
            matrice[u][v] = 1;
            matrice[v][u] = 1;
        }
        // etc. pour les autres directions
        if (y < lignes - 1 && !(murs_connus[u] & 4)) {
            v = (y + 1) * colonnes + x;
            matrice[u][v] = 1;
            matrice[v][u] = 1;
        }
        if (x > 0 && !(murs_connus[u] & 8)) {
            v = y * colonnes + (x - 1);
            matrice[u][v] = 1;
            matrice[v][u] = 1;
        }
    }
    return matrice;
}



int** creer_matrice_couts_dynamiques(int* murs_connus, const int* penalite_map, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int** matrice = malloc(sizeof(int*) * nb_cellules);
    if (!matrice) return NULL;
    for (int i = 0; i < nb_cellules; i++) {
        matrice[i] = calloc(nb_cellules, sizeof(int));
        if (!matrice[i]) {
            printf("erreur lors de la création de la matrice d'adjacence\n");
            return NULL; 
        }
    }

    for (int u = 0; u < nb_cellules; u++) {
        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);
        int v;
        int cout_aller, cout_retour;

        // Voisin du haut
        if (y > 0 && !(murs_connus[u] & 1)) {
            v = (y - 1) * colonnes + x;
            cout_aller = 1 + penalite_map[v];
            cout_retour = 1 + penalite_map[u];
            matrice[u][v] = cout_aller;
            matrice[v][u] = cout_retour;
        }
        // Voisin de droite
        if (x < colonnes - 1 && !(murs_connus[u] & 2)) {
            v = y * colonnes + (x + 1);
            cout_aller = 1 + penalite_map[v];
            cout_retour = 1 + penalite_map[u];
            matrice[u][v] = cout_aller;
            matrice[v][u] = cout_retour;
        }
        // Voisin du bas
        if (y < lignes - 1 && !(murs_connus[u] & 4)) {
            v = (y + 1) * colonnes + x;
            cout_aller = 1 + penalite_map[v];
            cout_retour = 1 + penalite_map[u];
            matrice[u][v] = cout_aller;
            matrice[v][u] = cout_retour;
        }
        // Voisin de gauche
        if (x > 0 && !(murs_connus[u] & 8)) {
            v = y * colonnes + (x - 1);
            cout_aller = 1 + penalite_map[v];
            cout_retour = 1 + penalite_map[u];
            matrice[u][v] = cout_aller;
            matrice[v][u] = cout_retour;
        }
    }
    return matrice;
}

int** creer_matrice_couts_connus(int* murs_connus, int* passages_counts, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int** matrice = malloc(sizeof(int*) * nb_cellules);
    if (!matrice) return NULL;
    for (int i = 0; i < nb_cellules; i++) {
        matrice[i] = calloc(nb_cellules, sizeof(int));
        if (!matrice[i]) {
            printf("erreur lors de la création de la matrice d'adjacence\n");
            return NULL; 
        }
    }

    for (int u = 0; u < nb_cellules; u++) {
        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);
        int v;

        // Voisin du haut
        if (y > 0 && !(murs_connus[u] & 1)) {
            v = (y - 1) * colonnes + x;
            int cout = 1 + passages_counts[v]; // Coût pour entrer dans la cellule v
            matrice[u][v] = cout;
            matrice[v][u] = 1 + passages_counts[u]; // Coût pour revenir en u
        }
        // Voisin de droite
        if (x < colonnes - 1 && !(murs_connus[u] & 2)) {
            v = y * colonnes + (x + 1);
            int cout = 1 + passages_counts[v];
            matrice[u][v] = cout;
            matrice[v][u] = 1 + passages_counts[u];
        }
        // Voisin du bas
        if (y < lignes - 1 && !(murs_connus[u] & 4)) {
            v = (y + 1) * colonnes + x;
            int cout = 1 + passages_counts[v];
            matrice[u][v] = cout;
            matrice[v][u] = 1 + passages_counts[u];
        }
        // Voisin de gauche
        if (x > 0 && !(murs_connus[u] & 8)) {
            v = y * colonnes + (x - 1);
            int cout = 1 + passages_counts[v];
            matrice[u][v] = cout;
            matrice[v][u] = 1 + passages_counts[u];
        }
    }
    return matrice;
}


// Libère la mémoire allouée pour la matrice d'adjacence.
void liberer_matrice_adjacence(int** matrice, int nb_cellules) {
    if (!matrice) return;
    for (int i = 0; i < nb_cellules; i++) {
        free(matrice[i]);
    }
    free(matrice);
}


// la fonction supprime un mur entre deux cellules adjacentes
//=============================================================================================================================//
// 15 -> 1111     murs de tous les cotes                                                                                       //
//  8 -> 1000     mur à gauche                                                                                                 //
//  4 -> 0100     mur en bas                                                                                                   //
//  2 -> 0010     mur à droite                                                                                                 //
//  1 -> 0001     mur en haut                                                                                                  //
// chaque bit represente la presence d'un mur dans une position                                                                //
// si par exemple on a 4 murs autour de u, donc murs[u] sera égale à 1|2|3|4 = 0001|0010|0100|1000 = 1111 = 15                 //
// et pour supprimer un mur, on doit d'abord inverser le nombre qui represente le murs par '~'par exemple pour le mur droite   //
// droite ~2 = 1101 et si on fait murs[u]&~2, le seule bit qui va changer est le troisieme et deviendre 0                      //
//=============================================================================================================================//
void supprimer_mur(int *murs, int colonnes, int u, int v) {
    int x1, y1, x2, y2; //coord des cellules 
    indice_vers_coord(u, colonnes, &x1, &y1);
    indice_vers_coord(v, colonnes, &x2, &y2);
    //distance entre les 2 cellures pour determiner la position d'une par rapport à l'autre (haut, bas, droit,gauche)
    int dx = x2 - x1; 
    int dy = y2 - y1;
    int idx1 = y1 * colonnes + x1; // = u, juste pour quelle se voit
    int idx2 = y2 * colonnes + x2; // = v, juste pour quelle se voit
    if (dx == 1) { // v à droite de u
        murs[idx1] &= ~2; // supprimer mur à droite de u 
        murs[idx2] &= ~8; // supprimer mur à gauche de v 
    } else if (dx == -1) { // v à gauche de u
        murs[idx1] &= ~8;
        murs[idx2] &= ~2;
    } else if (dy == 1) { // v en bas de u
        murs[idx1] &= ~4; // bas
        murs[idx2] &= ~1; // haut
    } else if (dy == -1) { // v en haut de u
        murs[idx1] &= ~1;
        murs[idx2] &= ~4;
    }
}


void ajouter_mur(int *murs, int colonnes, int u, int v) {
    int x1, y1, x2, y2;
    indice_vers_coord(u, colonnes, &x1, &y1);
    indice_vers_coord(v, colonnes, &x2, &y2);
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx == 1) { // v à droite de u
        murs[u] |= 2; // ajouter mur à droite de u
        murs[v] |= 8; // ajouter mur à gauche de v
    } else if (dx == -1) { // v à gauche de u
        murs[u] |= 8;
        murs[v] |= 2;
    } else if (dy == 1) { // v en bas de u
        murs[u] |= 4; // bas
        murs[v] |= 1; // haut
    } else if (dy == -1) { // v en haut de u
        murs[u] |= 1;
        murs[v] |= 4;
    }
}






// Affiche le labyrinthe en mode texte avec Unicode
void afficher_labyrinthe_unicode(int *murs, int lignes, int colonnes) {
    // bordure haut
    printf("┌");
    for (int x = 0; x < colonnes; x++) {
        printf("──");
        printf(x < colonnes - 1 ? "┬" : "┐");
    }
    printf("\n");
    for (int y = 0; y < lignes; y++) {
        printf("│"); // bordure gauche
        for (int x = 0; x < colonnes; x++) {
            printf("  ");
            int val = murs[y * colonnes + x];
            printf(val & 2 ? "│" : " ");
        }
        printf("\n");
        if (y < lignes - 1) {
            printf("├");
            for (int x = 0; x < colonnes; x++) {
                int val = murs[y * colonnes + x];
                printf(val & 4 ? "──" : "  ");
                printf(x < colonnes - 1 ? "┼" : "┤");
            }
            printf("\n");
        }
    }
    // bordure bas
    printf("└");
    for (int x = 0; x < colonnes; x++) {
        printf("──");
        printf(x < colonnes - 1 ? "┴" : "┘");
    }
    printf("\n");
}

/*

/ **********************************************
                    BFS
*********************************************** /
void BFS(int m_adj[N][N], int origine,noeud* n){
    file * f = malloc(sizeof(file));
    initialiser_file(f); // <--- APPEL INCORRECT
    initialiser_noeuds(n ,origine); // <--- APPEL INCORRECT
    n->visite[origine]=1;
    enfiler(f,origine);
    while(!filevide(f)){
        int x=defiler(f);
        for(int k=0;k<N;++k){
            if(n->visite[k]==0 && m_adj[x][k]==1){
                enfiler(f,k);
                n->visite[k]=1;
                n->distance[k]=n->distance[x]+1;
            }
        }
    }
    free(f);
}

/ **********************************************
                DIJKSTRA    
*********************************************** /
void dijkstra(int graphe[N][N], int origine, noeud *n) {
    tas t;
    t.taille = 0;
    initialiser_noeuds(n,origine); // <--- APPEL INCORRECT
    inserer(&t, origine, n);
    while (t.taille > 0) {
        int u = extraire_min(&t, n);
        if (n->visite[u]) continue;
        n->visite[u] = 1;

        for (int v = 0; v < N; v++) {
            if (graphe[u][v] > 0 && !n->visite[v]) {
                int alt = n->distance[u] + graphe[u][v];
                if (alt < n->distance[v]) {
                    n->distance[v] = alt;
                    inserer(&t, v, n);
                }
            }
        }
    }
}

/ **********************************************
                A_etoile 
*********************************************** /

// ?
void A_etoile(int graphe[N][N], int depart, int arrivee, int positions[N][2]) {
    int g[N], parent[N];
    bool ferme[N] = {false};

    for (int i = 0; i < N; i++) {
        g[i] = INF;
        parent[i] = -1;
    }

    g[depart] = 0;
    int h = estimation(positions[depart][0], positions[arrivee][0], positions[depart][1], positions[arrivee][1], 1);
    avl* ouvert = NULL;
    ouvert = inserer_avl(ouvert, depart, g[depart] + h);

    while (ouvert != NULL) {
        int u;
        ouvert = extraire_min_avl(ouvert, &u);

        if (u == arrivee) {
            printf("Chemin trouve !\n");
            int temp = arrivee;
            while (temp != -1) {
                printf("%d <- ", temp);
                temp = parent[temp];
            }
            printf("Depart\n");
            return;
        }

        ferme[u] = true;

        for (int v = 0; v < N; v++) {
            if (graphe[u][v] && !ferme[v]) {
                int tentative_g = g[u] + graphe[u][v];
                if (tentative_g < g[v]) {
                    g[v] = tentative_g;
                    parent[v] = u;
                    int h = estimation(positions[v][0], positions[arrivee][0], positions[v][1], positions[arrivee][1], 1);
                    ouvert = inserer_avl(ouvert, v, g[v] + h);
                }
            }
        }
    }

    printf("Aucun chemin trouvé.\n");
}*/




// BFS adapté pour naviguer dans le labyrinthe en utilisant le tableau de murs
void BFS_laby(int *murs, int lignes, int colonnes, int origine, noeud* n) {
    int nb_cellules = lignes * colonnes;
    file f;
    initialiser_file(&f, nb_cellules);
    initialiser_noeuds(n, origine, nb_cellules);

    n->visite[origine] = 1;
    enfiler(&f, origine);

    while (!filevide(&f)) {
        int u = defiler(&f);

        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);

        // Voisin du haut (y-1)
        if (y > 0 && !(murs[u] & 1)) { // S'il n'y a pas de mur en haut
            int v = (y - 1) * colonnes + x;
            if (!n->visite[v]) {
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->parent[v] = u;
                enfiler(&f, v);
            }
        }
        // Voisin de droite (x+1)
        if (x < colonnes - 1 && !(murs[u] & 2)) { // S'il n'y a pas de mur à droite
            int v = y * colonnes + (x + 1);
            if (!n->visite[v]) {
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->parent[v] = u;
                enfiler(&f, v);
            }
        }
        // Voisin du bas (y+1)
        if (y < lignes - 1 && !(murs[u] & 4)) { // S'il n'y a pas de mur en bas
            int v = (y + 1) * colonnes + x;
            if (!n->visite[v]) {
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->parent[v] = u;
                enfiler(&f, v);
            }
        }
        // Voisin de gauche (x-1)
        if (x > 0 && !(murs[u] & 8)) { // S'il n'y a pas de mur à gauche
            int v = y * colonnes + (x - 1);
            if (!n->visite[v]) {
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->parent[v] = u;
                enfiler(&f, v);
            }
        }
    }
    free_file(&f);
}

// Dijkstra fonctionnant sur une matrice d'adjacence.
void Dijkstra_laby(int** graphe, int nb_cellules, int destination, noeud* n) {
    initialiser_noeuds(n, destination, nb_cellules);
    
    tas t;
    t.tab = malloc(sizeof(int) * nb_cellules);
    t.taille = 0;
    inserer(&t, destination, n);

    while (t.taille > 0) {
        int u = extraire_min(&t, n);
        if (n->visite[u]) continue;
        n->visite[u] = 1;

        // LA LOGIQUE CHANGE ICI
        // Au lieu de vérifier les murs, on parcourt la ligne de la matrice.
        // C'est moins efficace car on teste les N-1 cellules même si 'u' n'a que 4 voisins max.
        for (int v = 0; v < nb_cellules; v++) {
            // S'il y a une arête entre u et v (le coût est > 0)
            // Note: Pour un Dijkstra qui part de la destination, on cherche les arêtes qui "entrent" dans u.
            // Mais comme notre graphe est non-orienté, graphe[v][u] == graphe[u][v]
            if (graphe[v][u] > 0 && !n->visite[v]) {
                // Le coût du pas est le poids de l'arête (v, u)
                int cout_du_pas = graphe[v][u];

                if (n->distance[u] + cout_du_pas < n->distance[v]) {
                    n->distance[v] = n->distance[u] + cout_du_pas;
                    n->parent[v] = u;
                    inserer(&t, v, n);
                }
            }
        }
    }
    free(t.tab);
}







// Reconstruit le chemin depuis la destination vers le départ en utilisant le tableau parent.
// Le chemin est inversé pour être dans le bon sens (départ -> destination).
// Renvoie le nombre d'étapes dans le chemin.
int reconstruire_chemin(noeud* n, int depart, int destination, int* chemin_buffer) {
    if (n->distance[depart] == INF) {
        printf("Avertissement : Pas de chemin trouvé entre %d et %d\n", depart, destination);
        return 0; // Pas de chemin
    }

    int etapes = 0;
    int courant = depart;
    
    // On suit les parents depuis le départ jusqu'à atteindre la destination.
    // Le chemin est construit directement dans le bon ordre.
    while (courant != destination) {
        chemin_buffer[etapes++] = courant;
        courant = n->parent[courant];
        
        // Sécurité simple et efficace pour éviter les boucles infinies
        if (courant == -1) { 
            fprintf(stderr, "Erreur: Chemin cassé lors de la reconstruction.\n");
            return 0;
        }
    }
    // Ne pas oublier d'ajouter la destination elle-même à la fin du chemin.
    chemin_buffer[etapes++] = destination;
    
    return etapes;
}

int reconstruire_chemin_inverse(noeud* n, int depart, int destination, int nb_cellules, int* chemin_buffer) {
    // Pour A*, la recherche part de `depart`, donc on vérifie si `destination` a été atteinte.
    if (n->parent[destination] == -1 && depart != destination) {
        printf("Avertissement : Pas de chemin trouvé entre %d et %d\n", depart, destination);
        return 0; // Pas de chemin
    }

    int* chemin_temp = malloc(sizeof(int) * nb_cellules);
    if (!chemin_temp) return 0;

    int etapes = 0;
    int courant = destination;

    // On suit les parents depuis la destination jusqu'à atteindre le départ.
    while (courant != -1) {
        chemin_temp[etapes++] = courant;
        if (courant == depart) break; // On a atteint le début, on s'arrête.
        courant = n->parent[courant];
        
        // Sécurité pour les cas où le chemin serait cassé pour une autre raison
        if (etapes >= nb_cellules) {
             fprintf(stderr, "Erreur: Boucle infinie détectée dans reconstruire_chemin_inverse.\n");
             free(chemin_temp);
             return 0;
        }
    }
    
    // Si la boucle s'est terminée sans trouver le départ, il y a un problème.
    if (courant != depart) {
        fprintf(stderr, "Erreur: Chemin cassé (inverse), impossible de remonter de %d à %d.\n", destination, depart);
        free(chemin_temp);
        return 0;
    }


    // Le chemin est dans chemin_temp dans l'ordre inverse (destination -> depart).
    // On le copie dans le buffer final dans le bon ordre (depart -> destination).
    for (int i = 0; i < etapes; i++) {
        chemin_buffer[i] = chemin_temp[etapes - 1 - i];
    }

    free(chemin_temp);
    return etapes;
}

int A_etoile_laby(int *murs, int lignes, int colonnes, int depart, int destination, noeud* n, int type_heuristique) {
    int nb_cellules = lignes * colonnes;
    int *g_costs = malloc(sizeof(int) * nb_cellules); // Tableau pour les coûts g(n)

    if (!g_costs) {
        fprintf(stderr, "Allocation échouée pour g_costs dans A*\n");
        return -1;
    }

    // Initialisation
    initialiser_noeuds(n, depart, nb_cellules); // n->distance stockera f(n)
    for (int i = 0; i < nb_cellules; i++) {
        g_costs[i] = INF;
    }

    g_costs[depart] = 0;
    
    int x_dest, y_dest;
    indice_vers_coord(destination, colonnes, &x_dest, &y_dest);
    int x_depart, y_depart;
    indice_vers_coord(depart, colonnes, &x_depart, &y_depart);
    
    // Le f_score du départ est juste l'heuristique (car g=0)
    n->distance[depart] = estimation(x_depart, x_dest, y_depart, y_dest, type_heuristique);

    // Initialisation du tas (file de priorité)
    tas open_set;
    open_set.tab = malloc(sizeof(int) * nb_cellules);
    open_set.taille = 0;
    inserer(&open_set, depart, n);

    int noeuds_visites = 0;

    while (open_set.taille > 0) {
        int u = extraire_min(&open_set, n);
        noeuds_visites++;

        if (u == destination) {
            free(g_costs);
            free(open_set.tab);
            return noeuds_visites; // Chemin trouvé
        }

        // n->visite agit comme le "closed set" pour éviter de traiter un noeud plusieurs fois
        n->visite[u] = 1;

        int x_u, y_u;
        indice_vers_coord(u, colonnes, &x_u, &y_u);

        // Explorer les voisins
        int voisins[4];
        int nb_voisins = 0;
        if (y_u > 0 && !(murs[u] & 1)) voisins[nb_voisins++] = (y_u - 1) * colonnes + x_u; // Haut
        if (x_u < colonnes - 1 && !(murs[u] & 2)) voisins[nb_voisins++] = y_u * colonnes + (x_u + 1); // Droite
        if (y_u < lignes - 1 && !(murs[u] & 4)) voisins[nb_voisins++] = (y_u + 1) * colonnes + x_u; // Bas
        if (x_u > 0 && !(murs[u] & 8)) voisins[nb_voisins++] = y_u * colonnes + (x_u - 1); // Gauche

        for (int i = 0; i < nb_voisins; i++) {
            int v = voisins[i];
            
            if (n->visite[v]) continue; // Déjà dans le closed set

            // Le coût pour se déplacer à un voisin est de 1 dans un labyrinthe non-pondéré
            int tentative_g = g_costs[u] + 1;

            if (tentative_g < g_costs[v]) {
                // Ce chemin vers v est meilleur que le précédent
                n->parent[v] = u;
                g_costs[v] = tentative_g;

                int x_v, y_v;
                indice_vers_coord(v, colonnes, &x_v, &y_v);
                int h_cost = estimation(x_v, x_dest, y_v, y_dest, type_heuristique);
                
                n->distance[v] = g_costs[v] + h_cost; // n->distance est notre f_score
                
                // On insère v dans le tas. Si v y est déjà, le tas n'est pas mis à jour
                // mais la prochaine extraction prendra le v avec le f_score le plus bas.
                // Pour une implémentation parfaite, il faudrait une fonction "decrease_key".
                // Mais pour ce cas, ré-insérer est simple et fonctionnel.
                inserer(&open_set, v, n);
            }
        }
    }

    // Pas de chemin trouvé
    free(g_costs);
    free(open_set.tab);
    return noeuds_visites;
}

// Fonction pour comparer les heuristiques
void comparer_heuristiques_A_etoile(int* murs, int lignes, int colonnes, int depart, int destination) {
    noeud n;
    //int nb_cellules = lignes * colonnes;
    int noeuds_visites;

    printf("\n--- Comparaison des Heuristiques A* pour le trajet %d -> %d ---\n", depart, destination);

    // 1. Distance de Manhattan
    noeuds_visites = A_etoile_laby(murs, lignes, colonnes, depart, destination, &n, HEURISTIC_MANHATTAN);
    printf("Heuristique MANHATTAN : %d noeuds visités.\n", noeuds_visites);
    free_noeuds(&n);

    // 2. Distance Euclidienne
    noeuds_visites = A_etoile_laby(murs, lignes, colonnes, depart, destination, &n, HEURISTIC_EUCLIDEAN);
    printf("Heuristique EUCLIDIENNE : %d noeuds visités.\n", noeuds_visites);
    free_noeuds(&n);

    // 3. Distance de Tchebychev
    noeuds_visites = A_etoile_laby(murs, lignes, colonnes, depart, destination, &n, HEURISTIC_TCHEBYCHEV);
    printf("Heuristique TCHEBYCHEV: %d noeuds visités.\n", noeuds_visites);
    free_noeuds(&n);
    printf("-----------------------------------------------------------------\n\n");
}

laby.h

#ifndef LABY_H
#define LABY_H

#include "structures.h"





// Structure représentant une arête entre deux cellules
typedef struct {
    int u, v;
} arete;

// Structure pour les partitions (Union-Find)
typedef struct {
    int taille;
    int *parent;
    int *rang;
} partition;

//================== Fonctions de partition ==================
void init_partition(partition* p, int total);
void free_partition(partition* p);
int recuperer_classe(partition* p, int i);
int fusion(partition* p, int i, int j);

//================== Génération de graphe ====================
void fisher_yates(arete G[], int n);
int generation_grille_vide(arete **G_ptr, int lignes, int colonnes);
void construire_arbre_couvrant(arete G[], int nb_aretes, arete *arbre, int* nb_arbre, int nb_cellules);
void generer_dot(const char* nom, arete aretes[], int nb);

//================== Manipulation de murs ====================
void indice_vers_coord(int indice, int colonnes, int* x, int* y);
void supprimer_mur(int *murs, int colonnes, int u, int v);
void ajouter_mur(int *murs, int colonnes, int u, int v); 
//================== Manipulation de Graphe ====================
int** creer_matrice_adjacence_cout_altr(int* murs, int lignes, int colonnes);
int** creer_matrice_adjacence_connue(int* murs_connus, int lignes, int colonnes);

int** creer_matrice_couts_dynamiques(int* murs_connus, const int* penalite_map, int lignes, int colonnes);

int** creer_matrice_couts_connus(int* murs_connus, int* passages_counts, int lignes, int colonnes);
void liberer_matrice_adjacence(int** matrice, int nb_cellules);

//================== BFS/DIj/A* ==============================

/*
// à supprimer ???
void BFS(int m_adj[N][N], int origine,noeud* n);
void dijkstra(int graphe[N][N], int origine, noeud *n);
void A_etoile(int graphe[N][N], int depart, int arrivee, int positions[N][2]); //?????
*/

//================== Résolution de labyrinthe ====================
void BFS_laby(int *murs, int lignes, int colonnes, int origine, noeud* n);
void Dijkstra_laby(int** graphe, int nb_cellules, int destination, noeud* n);

int A_etoile_laby(int *murs, int lignes, int colonnes, int depart, int destination, noeud* n, int type_heuristique);
void comparer_heuristiques_A_etoile(int* murs, int lignes, int colonnes, int depart, int destination);



int reconstruire_chemin(noeud* n, int depart, int destination, int* chemin_buffer);
int reconstruire_chemin_inverse(noeud* n, int depart, int destination, int nb_cellules, int* chemin_buffer);
//================== Affichage ===============================
void afficher_labyrinthe_unicode(int *murs, int lignes, int colonnes);

#endif






jeu.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "laby.h"
#include "labySDL.h"

// --- Définitions pour le mode de jeu à 2 ÉTATS ---
#define AI_MODE_SEARCH_ZONE 0 // Le monstre explore une zone
#define AI_MODE_HUNT 1        // Le monstre voit le joueur et le chasse

#define NOMBRE_MONSTRES 3
#define SEUIL_DETECTION_HUNT 6      // Portée de la vue directe
#define DUREE_PISTE 500             // La piste reste "chaude" pendant . frames
#define RAPP_CLDWN 100
#define MEMOIRE_MAX 99999
#define VITESSE_MONSTRE 20


#define MONSTRE_PENALITE_RAYON 4 // How far the penalty spreads from a monster.
#define MONSTRE_PENALITE_COUT 20 // The base cost added to a cell near another monster.


#define SAUT_COOLDOWN 0

#define NOMBRE_PIECES 3 // Le joueur doit collecter . pièces pour gagner

#define HEIGHT 20
#define WIDTH 20


typedef struct {
    int pos;
    int mode;
    int* murs_connus; // G
    arete* memoire_murs;
    int memoire_tete, memoire_queue, memoire_taille_actuelle;
    int timer_piste;
    // Variables pour la recherche de zone
    bool* noeuds_visites_zone; // F
    int* frontier_nodes; // O
    int frontier_size;
    // Variables pour le chemin en cours
    int prochaine_position;

    int drnier_pos_jr_connu;

    int rapp_cooldown;
    int move_cooldown;
} Monstre;



typedef struct {
    int pos;
    int direction; // La direction vers laquelle le joueur fait face (1:Haut, 2:Droite, 4:Bas, 8:Gauche)
    int saut_cooldown;
} Joueur;




void melanger_voisins(int* tableau, int n) {
    if (n > 1) {
        for (int i = n - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int temp = tableau[i];
            tableau[i] = tableau[j];
            tableau[j] = temp;
        }
    }
}


// Gère l'apprentissage et l'oubli des murs
void apprendre_mur(Monstre* monstre, int u, int v, int colonnes) {
    int diff = v - u;
    int dir_flag = 0;
    if (diff == -colonnes) dir_flag = 1; else if (diff == 1) dir_flag = 2; else if (diff == colonnes) dir_flag = 4; else if (diff == -1) dir_flag = 8;
    if (monstre->murs_connus[u] & dir_flag) return;

    printf("Monstre %d apprend le mur entre %d et %d.\n", monstre->pos, u, v);
    ajouter_mur(monstre->murs_connus, colonnes, u, v);
    if (monstre->memoire_taille_actuelle >= MEMOIRE_MAX) {
        arete mur_oublie = monstre->memoire_murs[monstre->memoire_tete];
        supprimer_mur(monstre->murs_connus, colonnes, mur_oublie.u, mur_oublie.v);
        monstre->memoire_tete = (monstre->memoire_tete + 1) % MEMOIRE_MAX;
        monstre->memoire_taille_actuelle--;
    }
    monstre->memoire_murs[monstre->memoire_queue] = (arete){u, v};
    monstre->memoire_queue = (monstre->memoire_queue + 1) % MEMOIRE_MAX;
    monstre->memoire_taille_actuelle++;
}



// GIDC: Graphe Inconnu, Destination Connue
int gidc(Monstre* monstre, int* murs_reels, int lignes, int colonnes, int destination, const int* penalite_map) {
    int nb_cellules = lignes * colonnes;
    int depart = monstre->pos;

    // Planifier le chemin sur la carte connue
    int** graphe_connu = creer_matrice_couts_dynamiques(monstre->murs_connus, penalite_map, lignes, colonnes);
    if (!graphe_connu) { return depart; } 

    noeud plan;
    Dijkstra_laby(graphe_connu, nb_cellules, destination, &plan);
    
    // Trouver le meilleur voisin pour se rapprocher
    int meilleur_voisin = -1;
    int min_dist_estimee = INF;
    int x_act, y_act;
    indice_vers_coord(depart, colonnes, &x_act, &y_act);
    int voisins[4] = {
        (y_act > 0) ? (y_act - 1) * colonnes + x_act : -1, (x_act < colonnes - 1) ? y_act * colonnes + (x_act + 1) : -1,
        (y_act < lignes - 1) ? (y_act + 1) * colonnes + x_act : -1, (x_act > 0) ? y_act * colonnes + (x_act - 1) : -1
    };

    for (int i = 0; i < 4; i++) {
        int v = voisins[i]; 
        if (v == -1) continue;
        
        int diff_temp = v - depart; 
        int dir_flag_temp = 0;
        if (diff_temp == -colonnes) dir_flag_temp = 1; else if (diff_temp == 1) dir_flag_temp = 2; else if (diff_temp == colonnes) dir_flag_temp = 4; else if (diff_temp == -1) dir_flag_temp = 8;
        
        if (!(monstre->murs_connus[depart] & dir_flag_temp)) {
            if (plan.distance[v] < min_dist_estimee) { 
                min_dist_estimee = plan.distance[v]; 
                meilleur_voisin = v; 
            }
        }
    }
    free_noeuds(&plan);
    liberer_matrice_adjacence(graphe_connu, nb_cellules);

    if (meilleur_voisin == -1) { 
        return depart; // Bloqué selon sa carte, il ne bouge pas
    }

    // Valider le mouvement contre la réalité
    int diff = meilleur_voisin - depart; 
    int direction_flag = 0;
    if (diff == -colonnes) direction_flag = 1; else if (diff == 1) direction_flag = 2; else if (diff == colonnes) direction_flag = 4; else if (diff == -1) direction_flag = 8;

    if (murs_reels[depart] & direction_flag) {
        // Collision ! Apprendre et ne pas bouger.
        apprendre_mur(monstre, depart, meilleur_voisin, colonnes);
        return depart;
    } else {
        // Pas de collision, le mouvement est valide.
        return meilleur_voisin;
    }
}



int gidi(Monstre* monstre, int* murs_reels, int lignes, int colonnes, const int* penalite_map) {
    int nb_cellules = lignes * colonnes;

    // --- 1. Observer les environs et mettre à jour la frontière ---
    // Cette partie reste inchangée, car elle ne fait pas de pathfinding.
    monstre->noeuds_visites_zone[monstre->pos] = true;
    for (int k = 0; k < monstre->frontier_size; k++) {
        if (monstre->frontier_nodes[k] == monstre->pos) {
            monstre->frontier_nodes[k] = monstre->frontier_nodes[--monstre->frontier_size];
            break;
        }
    }
    int x_act, y_act;
    indice_vers_coord(monstre->pos, colonnes, &x_act, &y_act);
    int voisins[4] = {
        (y_act > 0) ? (y_act - 1) * colonnes + x_act : -1, (x_act < colonnes - 1) ? y_act * colonnes + (x_act + 1) : -1,
        (y_act < lignes - 1) ? (y_act + 1) * colonnes + x_act : -1, (x_act > 0) ? y_act * colonnes + (x_act - 1) : -1
    };
    melanger_voisins(voisins, 4);
    for (int i = 0; i < 4; i++) {
        int v = voisins[i];
        if (v == -1) continue;
        int diff = v - monstre->pos;
        int dir_flag = 0;
        if (diff == -colonnes) dir_flag = 1; else if (diff == 1) dir_flag = 2; else if (diff == colonnes) dir_flag = 4; else if (diff == -1) dir_flag = 8;
        if (murs_reels[monstre->pos] & dir_flag) {
            apprendre_mur(monstre, monstre->pos, v, colonnes);
        } else {
            if (!monstre->noeuds_visites_zone[v]) {
                bool deja_dans_frontiere = false;
                for (int j = 0; j < monstre->frontier_size; j++) { if (monstre->frontier_nodes[j] == v) { deja_dans_frontiere = true; break; } }
                if (!deja_dans_frontiere) { monstre->frontier_nodes[monstre->frontier_size++] = v; }
            }
        }
    }

    if (monstre->frontier_size == 0) return monstre->pos;

    // --- 2. Trouver la cible la plus proche sur la frontière avec DIJKSTRA ---
    noeud plan_vers_frontiere;
    

    //BFS_laby(monstre->murs_connus, lignes, colonnes, monstre->pos, &plan_vers_frontiere);

    // On crée une matrice d'adjacence à partir de la connaissance du monstre
    int** graphe_connu_1 = creer_matrice_couts_dynamiques(monstre->murs_connus, penalite_map, lignes, colonnes);
    if (!graphe_connu_1) { return monstre->pos; } // Sécurité
    // On lance Dijkstra depuis la position actuelle
    Dijkstra_laby(graphe_connu_1, nb_cellules, monstre->pos, &plan_vers_frontiere);
    // On libère la mémoire de la matrice
    liberer_matrice_adjacence(graphe_connu_1, nb_cellules);

    int target_node = -1;
    int min_dist = INF;
    for (int i = 0; i < monstre->frontier_size; i++) {
        int node_f = monstre->frontier_nodes[i];
        if (plan_vers_frontiere.distance[node_f] < min_dist) {
            min_dist = plan_vers_frontiere.distance[node_f];
            target_node = node_f;
        }
    }
    free_noeuds(&plan_vers_frontiere); // On libère le plan qui n'est plus utile
    if (target_node == -1) return monstre->pos;

    // --- 3. Calculer le premier pas vers cette cible avec DIJKSTRA ---
    noeud plan_vers_cible;

    // On recrée une matrice d'adjacence (nécessaire car la première a été libérée)
    int** graphe_connu_2 = creer_matrice_couts_dynamiques(monstre->murs_connus, penalite_map, lignes, colonnes);
    if (!graphe_connu_2) { return monstre->pos; } // Sécurité
    // On lance Dijkstra depuis la cible pour trouver le chemin retour
    Dijkstra_laby(graphe_connu_2, nb_cellules, target_node, &plan_vers_cible);
    // On libère la mémoire
    liberer_matrice_adjacence(graphe_connu_2, nb_cellules);

    int prochain_pas_planifie = -1;
    if (plan_vers_cible.distance[monstre->pos] != INF) {
        prochain_pas_planifie = plan_vers_cible.parent[monstre->pos];
    }
    free_noeuds(&plan_vers_cible);

    if (prochain_pas_planifie == -1 || prochain_pas_planifie == monstre->pos) {
        return monstre->pos;
    }
    
    // --- 4. Vérifier le mouvement contre la réalité ---
    int diff = prochain_pas_planifie - monstre->pos;
    int direction_flag = 0;
    if (diff == -colonnes) direction_flag = 1; else if (diff == 1) direction_flag = 2; else if (diff == colonnes) direction_flag = 4; else if (diff == -1) direction_flag = 8;

    if (murs_reels[monstre->pos] & direction_flag) {
        apprendre_mur(monstre, monstre->pos, prochain_pas_planifie, colonnes);
        return monstre->pos;
    } else {
        return prochain_pas_planifie;
    }
}


void mettre_a_jour_monstre(Monstre* monstres, int monstre_index, int joueur_pos, int* murs_reels, int lignes, int colonnes) {
    Monstre* monstre = &monstres[monstre_index];
    int nb_cellules = lignes * colonnes;
    
    // --- PERCEPTION & DÉCISION DU MODE ---
    int j_x, j_y, m_x, m_y;
    indice_vers_coord(joueur_pos, colonnes, &j_x, &j_y);
    indice_vers_coord(monstre->pos, colonnes, &m_x, &m_y);
    int distance = abs(j_x - m_x) + abs(j_y - m_y);

    if (monstre->timer_piste > 0) monstre->timer_piste--;
    

    int old_mode = monstre->mode;
    
    
    if (distance <= SEUIL_DETECTION_HUNT) {
        monstre->mode = AI_MODE_HUNT;
        if (old_mode != AI_MODE_HUNT) printf("Monstre %d -> MODE CHASSE (Détection directe)\n", monstre->pos);
        monstre->drnier_pos_jr_connu = joueur_pos;
        monstre->timer_piste = DUREE_PISTE;
    } 
    else if (monstre->timer_piste > 0) {
        monstre->mode = AI_MODE_HUNT;
        if (old_mode != AI_MODE_HUNT) printf("Monstre %d -> MODE CHASSE (Poursuite de la piste)\n", monstre->pos);
    } 
    else {
        monstre->mode = AI_MODE_SEARCH_ZONE;
        if (old_mode != AI_MODE_SEARCH_ZONE){
            printf("Monstre %d -> MODE RECHERCHE (Piste perdue)\n", monstre->pos);
            // On s'assure que la cible de patrouille est la position actuelle pour commencer par explorer localement
            monstre->drnier_pos_jr_connu = monstre->pos; 
            monstre->frontier_size = 0;
            memset(monstre->noeuds_visites_zone, 0, sizeof(bool) * (lignes*colonnes));
            monstre->frontier_nodes[monstre->frontier_size++] = monstre->pos;
        }
    }

    
    // --- ACTION ---
    if (monstre->move_cooldown > 0) {
        monstre->move_cooldown--;
        return; // Pas encore l'heure de bouger
    }
    monstre->move_cooldown = VITESSE_MONSTRE;


    int* penalite_map = calloc(nb_cellules, sizeof(int));
    if (!penalite_map) return; // Sécurité

    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        if (i == monstre_index) continue; // Ne pas se pénaliser soi-même

        int autre_monstre_pos = monstres[i].pos;
        int am_x, am_y;
        indice_vers_coord(autre_monstre_pos, colonnes, &am_x, &am_y);

        // Appliquer une pénalité dans un rayon autour de l'autre monstre
        for (int y_p = am_y - MONSTRE_PENALITE_RAYON; y_p <= am_y + MONSTRE_PENALITE_RAYON; y_p++) {
            for (int x_p = am_x - MONSTRE_PENALITE_RAYON; x_p <= am_x + MONSTRE_PENALITE_RAYON; x_p++) {
                if (x_p >= 0 && x_p < colonnes && y_p >= 0 && y_p < lignes) {
                    int dist = abs(x_p - am_x) + abs(y_p - am_y);
                    if (dist <= MONSTRE_PENALITE_RAYON) {
                        int penalite = MONSTRE_PENALITE_COUT * (MONSTRE_PENALITE_RAYON - dist) / MONSTRE_PENALITE_RAYON;
                        int cell_idx = y_p * colonnes + x_p;
                        penalite_map[cell_idx] += penalite; // On ajoute, au cas où plusieurs monstres sont proches
                    }
                }
            }
        }
    }

    // --- PLANIFICATION ET MISE À JOUR DE la `prochaine_position` ---
    int prochaine_position_planifiee = monstre->pos;
    
    switch (monstre->mode) {
        case AI_MODE_HUNT:
            // GIDC: Calcule le prochain pas optimal vers le joueur
            prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, joueur_pos, penalite_map);
            monstre->rapp_cooldown = 0;
            break;
        case AI_MODE_SEARCH_ZONE:
            // On vérifie si les conditions de repositionnement sont remplies
            
            // 2. Phase de déplacement ou d'exploration
            if (monstre->pos != monstre->drnier_pos_jr_connu && monstre->rapp_cooldown == 0) {
                // Si on n'est pas encore à notre point de patrouille, on s'y dirige.
                prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, monstre->drnier_pos_jr_connu, penalite_map);
                
                /*
                // Si on est bloqué en route, on explore pour trouver une issue
                if (prochaine_position_planifiee == monstre->pos) {
                    printf("Monstre %d : Bloqué en route vers la cible de patrouille, exploration GIDI en secours.\n", monstre->pos);
                    prochaine_position_planifiee = gidi(monstre, murs_reels, lignes, colonnes);
                }*/

                if (prochaine_position_planifiee == monstre->drnier_pos_jr_connu) {
                    monstre->rapp_cooldown = RAPP_CLDWN;
                }
            } else {
                // On est arrivé à notre point de patrouille, on explore localement avec GIDI.
                //printf("Monstre %d : Cible de patrouille atteinte, exploration locale (GIDI).\n", monstre->pos);
                prochaine_position_planifiee = gidi(monstre, murs_reels, lignes, colonnes, penalite_map);
                if (monstre->rapp_cooldown >= 1){
                    printf("\ncooldown :%d\n", monstre->rapp_cooldown);
                    monstre->rapp_cooldown--;
                }
                if (monstre->rapp_cooldown == 1){
                    //monstre->rapp_cooldown = RAPP_CLDWN;
                    monstre->drnier_pos_jr_connu = joueur_pos;
                }
            }
            /*
            // 3. Dernier recours si GIDI ne trouve rien à explorer
            if (prochaine_position_planifiee == monstre->pos) { 
                printf("Monstre %d a fini sa recherche locale, choisit une destination de patrouille aléatoire.\n", monstre->pos);
                monstre->drnier_pos_jr_connu = joueur_pos; // La nouvelle cible est aléatoire
                prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, monstre->drnier_pos_jr_connu);
            }*/
            break;
    }

    // Libérer la mémoire de la carte de pénalités
    free(penalite_map);

    // --- EXÉCUTION DU MOUVEMENT ---  
    monstre->pos = prochaine_position_planifiee;
}




// Fonction de jeu principale
void lancer_jeu(int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* fenetre = SDL_CreateWindow("Le Jeu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE + 1, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");
    SDL_Texture* monstre_texture = IMG_LoadTexture(rendu, "monstre.png");
    SDL_Texture* piece_texture = IMG_LoadTexture(rendu, "piece.png");

    // Initialisation du Joueur
    Joueur joueur;
    joueur.pos = 0;
    joueur.direction = 4; // Commence en regardant vers le bas
    joueur.saut_cooldown = 0;

    Monstre monstres[NOMBRE_MONSTRES];
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        Monstre* m = &monstres[i];
        m->pos = nb_cellules - 1 - i * 2;
        m->mode = AI_MODE_SEARCH_ZONE;
        m->timer_piste = 0;
        m->murs_connus = calloc(nb_cellules, sizeof(int));
        m->memoire_murs = malloc(sizeof(arete) * MEMOIRE_MAX);
        m->memoire_tete = 0; m->memoire_queue = 0; m->memoire_taille_actuelle = 0;
        
        m->noeuds_visites_zone = calloc(nb_cellules, sizeof(bool));
        m->frontier_nodes = malloc(sizeof(int) * nb_cellules);
        m->frontier_size = 0;

        m->prochaine_position = nb_cellules - 1 - i * 2;
        m->drnier_pos_jr_connu = 0; // = pos de depart du jr 
        m->rapp_cooldown = 0;
        m->move_cooldown = 1;//i * 2;
    }

    
    //generation des pieces
    int* pieces_pos = malloc(sizeof(int) * NOMBRE_PIECES);
    int pieces_collectees = 0;
    printf("Génération de %d pièces...\n", NOMBRE_PIECES);
    for (int i = 0; i < NOMBRE_PIECES; i++) {
        int pos;
        bool position_valide;
        do {
            position_valide = true;
            pos = rand() % nb_cellules; // Position aléatoire

            // Vérifier que la position n'est pas celle du joueur
            if (pos == joueur.pos) { position_valide = false; continue; }

            // Vérifier que la position n'est pas celle d'un monstre
            for(int j = 0; j < NOMBRE_MONSTRES; j++) {
                if (pos == monstres[j].pos) { position_valide = false; break; }
            }
            if (!position_valide) continue;

            // Vérifier que la position n'est pas déjà prise par une autre pièce
            for (int k = 0; k < i; k++) {
                if (pos == pieces_pos[k]) { position_valide = false; break; }
            }
        } while (!position_valide);
        pieces_pos[i] = pos;
    }


    bool quitter = false;
    SDL_Event e;
    while (!quitter) {
        // Décrémenter le cooldown du saut du joueur à chaque frame
        if (joueur.saut_cooldown > 0) {
            joueur.saut_cooldown--;
        }
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitter = true;
            }
            if (e.type == SDL_KEYDOWN) {
                int nouvelle_pos = joueur.pos;
                int direction_flag = 0;

                switch (e.key.keysym.sym) {
                    case SDLK_UP:    nouvelle_pos -= colonnes; direction_flag = 1; break;
                    case SDLK_DOWN:  nouvelle_pos += colonnes; direction_flag = 4; break;
                    case SDLK_LEFT:  nouvelle_pos -= 1;        direction_flag = 8; break;
                    case SDLK_RIGHT: nouvelle_pos += 1;        direction_flag = 2; break;
                    case SDLK_SPACE: 
                        // On ne peut sauter que si le cooldown est terminé
                        if (joueur.saut_cooldown == 0) {
                            int pos_apres_saut = -1;
                            // On vérifie s'il y a bien un mur dans la direction où le joueur regarde et qu'il ne sort pas du labyrinthe
                            if (murs_reels[joueur.pos] & joueur.direction) {
                                // Calculer la position de l'autre côté du mur
                                if (joueur.direction == 1) pos_apres_saut = joueur.pos - colonnes; // HAUT
                                else if (joueur.direction == 4) pos_apres_saut = joueur.pos + colonnes; // BAS
                                else if (joueur.direction == 8) pos_apres_saut = joueur.pos - 1;       // GAUCHE
                                else if (joueur.direction == 2) pos_apres_saut = joueur.pos + 1;       // DROITE
                                
                                // Vérifier que la case d'atterrissage est valide (dans le labyrinthe)
                                int x, y;
                                indice_vers_coord(pos_apres_saut, colonnes, &x, &y);
                                if (x >= 0 && x < colonnes && y >= 0 && y < lignes) {
                                    if(((joueur.direction == 2 || joueur.direction == 8) && joueur.pos/colonnes == y) || joueur.direction==1 || joueur.direction==4) {
                                    printf("Le joueur a sauté par-dessus un mur !\n");
                                    joueur.pos = pos_apres_saut;
                                    joueur.saut_cooldown = SAUT_COOLDOWN; // Activer le cooldown
                                    }
                                }
                            }
                        }
                        // On met le flag à 0 pour ne pas entrer dans la logique de marche normale
                        direction_flag = 0;
                        break;
                }
                // Si une touche de direction a été pressée
                if (direction_flag != 0) {
                    // On met à jour la direction dans laquelle le joueur regarde
                    joueur.direction = direction_flag;
                    // On vérifie si le mouvement est valide (pas de mur)
                    if (!(murs_reels[joueur.pos] & direction_flag)) {
                        joueur.pos = nouvelle_pos;
                    }
                }
            }
        }

        // --- LOGIQUE DE COLLECTE DES PIÈCES ---
        for (int i = 0; i < NOMBRE_PIECES; i++) {
            // Si la pièce existe encore et que le joueur est dessus
            if (pieces_pos[i] != -1 && joueur.pos == pieces_pos[i]) {
                pieces_pos[i] = -1; // "Supprimer" la pièce
                pieces_collectees++;
                printf("Pièce collectée ! (%d / %d)\n", pieces_collectees, NOMBRE_PIECES);
            }
        }
        
        // --- CONDITION DE VICTOIRE ---
        if (pieces_collectees == NOMBRE_PIECES) {
            printf("VICTOIRE ! Toutes les pièces ont été collectées !\n");
            quitter = true; // Termine la boucle de jeu
        }

        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            mettre_a_jour_monstre(monstres, i, joueur.pos, murs_reels, lignes, colonnes);
            if (monstres[i].pos == joueur.pos) { printf("GAME OVER !\n"); quitter = true; }
        }

        // --- DESSIN ---
        SDL_SetRenderDrawColor(rendu, 20, 0, 30, 255);
        SDL_RenderClear(rendu);

        //dessin de bg
        dessiner_bg(rendu, lignes, colonnes);

        for (int i = 0; i < NOMBRE_PIECES; i++) {
            if (pieces_pos[i] != -1) {
                // On réutilise la fonction dessiner_personnage, car elle dessine un sprite à une position
                dessiner_personnage(rendu, piece_texture, (pieces_pos[i] % colonnes + 0.5f) * TAILLE_CELLULE, (pieces_pos[i] / colonnes + 0.5f) * TAILLE_CELLULE);
            }
        }


        SDL_SetRenderDrawColor(rendu, 100, 80, 200, 255);
        

        for (int i = 0; i < nb_cellules; i++) dessiner_murs_connus(rendu, i % colonnes, i / colonnes, murs_reels, colonnes);

        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            if (monstres[i].mode == AI_MODE_HUNT) {
                dessiner_rayon_detection(rendu, monstres[i].pos, SEUIL_DETECTION_HUNT, lignes, colonnes);
            }
        }

        
        dessiner_personnage(rendu, perso_texture, (joueur.pos % colonnes + 0.5f) * TAILLE_CELLULE, (joueur.pos / colonnes + 0.5f) * TAILLE_CELLULE);
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            if(monstres[i].mode == AI_MODE_HUNT) SDL_SetTextureColorMod(monstre_texture, 255, 100, 100); // Rouge
            else if (monstres[i].mode == AI_MODE_SEARCH_ZONE) SDL_SetTextureColorMod(monstre_texture, 100, 255, 100); // Jaune
            dessiner_personnage(rendu, monstre_texture, (monstres[i].pos % colonnes + 0.5f) * TAILLE_CELLULE, (monstres[i].pos / colonnes + 0.5f) * TAILLE_CELLULE);
        }
        SDL_RenderPresent(rendu);
    }

    free(pieces_pos); // libérer le tableau des pièces

    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        free(monstres[i].murs_connus);
        free(monstres[i].memoire_murs);
        free(monstres[i].noeuds_visites_zone);
        free(monstres[i].frontier_nodes);
    }
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyTexture(monstre_texture);
    SDL_DestroyTexture(piece_texture); 
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre); SDL_Quit();
}

int main() {
    unsigned int seed = time(NULL);
    srand(seed);
    printf("seed de la labyrinth est : %u\n", seed);
    int lignes = HEIGHT;
    int colonnes = WIDTH;
    int nb_cellules = lignes * colonnes;
    
    arete *toutes_aretes;
    int nb_total_aretes = generation_grille_vide(&toutes_aretes, lignes, colonnes);
    fisher_yates(toutes_aretes, nb_total_aretes);
    arete *arbre = malloc(sizeof(arete) * (nb_cellules - 1));
    int nb_aretes_arbre;
    construire_arbre_couvrant(toutes_aretes, nb_total_aretes, arbre, &nb_aretes_arbre, nb_cellules);
    free(toutes_aretes);
    
    int *murs_reels = malloc(sizeof(int) * nb_cellules);
    for (int i = 0; i < nb_cellules; i++) murs_reels[i] = 1 | 2 | 4 | 8;
    for (int i = 0; i < nb_aretes_arbre; i++) supprimer_mur(murs_reels, colonnes, arbre[i].u, arbre[i].v);
    free(arbre);
    lancer_jeu(murs_reels, lignes, colonnes);
    free(murs_reels);
    printf("Programme terminé.\n");
    return 0;
}
