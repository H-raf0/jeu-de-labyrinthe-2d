#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "labySDL.h"
#include "laby.h"


RenderConfig g_config;


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

    SDL_SetRenderDrawColor(rendu, 255, 165, 0, 255); // Couleur des murs (orange)

    // Calcul des coordonnées à l'écran en utilisant la configuration globale
    int px = g_config.offset_x + x * g_config.cell_size;
    int py = g_config.offset_y + y * g_config.cell_size;
    int size = g_config.cell_size;
    int thick = g_config.wall_thickness;

    int val = murs[y * colonnes + x];
    
    SDL_Rect wall_rect;

    // Mur du HAUT (1)
    if (val & 1) {
        wall_rect = (SDL_Rect){px, py, size, thick};
        SDL_RenderFillRect(rendu, &wall_rect);
    }
    // Mur de DROITE (2)
    if (val & 2) {
        wall_rect = (SDL_Rect){px + size - thick, py, thick, size};
        SDL_RenderFillRect(rendu, &wall_rect);
    }
    // Mur du BAS (4)
    if (val & 4) {
        wall_rect = (SDL_Rect){px, py + size - thick, size, thick};
        SDL_RenderFillRect(rendu, &wall_rect);
    }
    // Mur de GAUCHE (8)
    if (val & 8) {
        wall_rect = (SDL_Rect){px, py, thick, size};
        SDL_RenderFillRect(rendu, &wall_rect);
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

    // calcul de la configuration de rendu locale
    RenderConfig local_config;
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }
    local_config.window_w = dm.w;
    local_config.window_h = dm.h;

    // Création de la fenêtre en plein écran
    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe", 
                                           SDL_WINDOWPOS_CENTERED, 
                                           SDL_WINDOWPOS_CENTERED, 
                                           local_config.window_w, 
                                           local_config.window_h, 
                                           SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!fenetre) {
        fprintf(stderr, "Erreur création fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    if (!rendu) {
        fprintf(stderr, "Erreur création rendu: %s\n", SDL_GetError());
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        return;
    }

    // Calcul des dimensions proportionnelles
    int cell_w = local_config.window_w / colonnes;
    int cell_h = local_config.window_h / lignes;
    local_config.cell_size = (cell_w < cell_h) ? cell_w : cell_h;
    local_config.wall_thickness = (local_config.cell_size / 16.0f > 1) ? (int)(local_config.cell_size / 16.0f) : 1;
    local_config.offset_x = (local_config.window_w - (colonnes * local_config.cell_size)) / 2;
    local_config.offset_y = (local_config.window_h - (lignes * local_config.cell_size)) / 2;

    // Préparation de la structure de murs du labyrinthe
    int total = lignes * colonnes;
    int *murs = malloc(sizeof(int) * total);
    if (!murs) {
        fprintf(stderr, "Allocation mémoire impossible pour murs\n");
        SDL_DestroyRenderer(rendu);
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        return;
    }
    for (int i = 0; i < total; i++) murs[i] = 1|2|4|8; // Tous les murs présents
    for (int i = 0; i < nb_aretes; i++) supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);

    // Dessin
    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255); // Couleur de fond (noir)
    SDL_RenderClear(rendu);
    
    // Assigner la configuration locale à la variable globale pour que la fonction dessiner_murs_connus puisse l'utiliser.
    g_config = local_config;

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
            if (e.type == SDL_QUIT) {
                quitter = 1;
            }
            // Permet de quitter avec la touche Échap
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quitter = 1;
            }
        }
        SDL_Delay(10);
    }
    
    // Libération des ressources
    free(murs);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}


/*
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

*/



void dessiner_tuile(SDL_Renderer* rendu, SDL_Texture* tileset, int* murs, int x, int y, int colonnes) {
    SDL_Rect dst;
    int val_murs = murs[y * colonnes + x];
    SDL_Rect src = src_murs[val_murs];

    dst.x = g_config.offset_x + x * g_config.cell_size;
    dst.y = g_config.offset_y + y * g_config.cell_size;
    dst.w = g_config.cell_size;
    dst.h = g_config.cell_size;
    
    SDL_RenderCopy(rendu, tileset, &src, &dst);
}


void dessiner_tuile_v2(SDL_Renderer* rendu, SDL_Texture* tileset, int x, int y){
    SDL_Rect dst;
    int p_extra = 0; // Proportion du dimensions supprimé de la tuile dans la cellue
    
    // Définition des rectangles source pour les tuiles de sol
    SDL_Rect src_sol;
    SDL_Rect src_sol1 = {2 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE};
    SDL_Rect src_sol2 = {2 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE};
    SDL_Rect src_sol3 = {1 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE};
    int p = (x * 19 + y * 73) % 101;
    if(p <= 70){
        src_sol = src_sol1;
    }else if (p<=78){
        src_sol = src_sol2;
    }else{
        src_sol = src_sol3;
    }

    dst.x = g_config.offset_x + x * g_config.cell_size + p_extra;
    dst.y = g_config.offset_y + y * g_config.cell_size + p_extra;
    dst.w = g_config.cell_size - p_extra;
    dst.h = g_config.cell_size - p_extra;

    SDL_RenderCopy(rendu, tileset, &src_sol, &dst);
}


void dessiner_bg(SDL_Renderer* rendu, int lignes, int colonnes) {
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

    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rendu, 255, 100, 100, 75);

    for (int y = cy - rayon; y <= cy + rayon; y++) {
        for (int x = cx - rayon; x <= cx + rayon; x++) {
            if (x >= 0 && x < colonnes && y >= 0 && y < lignes) {
                int dist = abs(x - cx) + abs(y - cy);
                if (dist <= rayon) {
                    SDL_Rect case_rect = {
                        g_config.offset_x + x * g_config.cell_size, 
                        g_config.offset_y + y * g_config.cell_size, 
                        g_config.cell_size, 
                        g_config.cell_size
                    };
                    SDL_RenderFillRect(rendu, &case_rect);
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_NONE);
}










// Affiche le labyrinthe avec des tuiles en plein écran et de manière proportionnelle
// old?
void afficher_labyrinthe_sdl_tuiles(int *murs, int lignes, int colonnes) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }

    // --- Début du calcul de la configuration de rendu locale ---
    RenderConfig local_config;
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }
    local_config.window_w = dm.w;
    local_config.window_h = dm.h;

    // Création de la fenêtre en plein écran
    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe Tuiles",
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           local_config.window_w,
                                           local_config.window_h,
                                           SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!fenetre) {
        fprintf(stderr, "Erreur création fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    if (!rendu) {
        fprintf(stderr, "Erreur création rendu: %s\n", SDL_GetError());
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        return;
    }

    // Calcul des dimensions proportionnelles
    int cell_w = local_config.window_w / colonnes;
    int cell_h = local_config.window_h / lignes;
    local_config.cell_size = (cell_w < cell_h) ? cell_w : cell_h;
    local_config.wall_thickness = (local_config.cell_size / 16.0f > 1) ? (int)(local_config.cell_size / 16.0f) : 1;
    local_config.offset_x = (local_config.window_w - (colonnes * local_config.cell_size)) / 2;
    local_config.offset_y = (local_config.window_h - (lignes * local_config.cell_size)) / 2;

    // Chargement de la texture du tileset
    SDL_Surface* tileset_surface = IMG_Load("tileset.png");
    if (!tileset_surface) {
        fprintf(stderr, "Erreur chargement tileset.png : %s\n", IMG_GetError());
        SDL_DestroyRenderer(rendu);
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    SDL_Texture* tileset = SDL_CreateTextureFromSurface(rendu, tileset_surface);
    SDL_FreeSurface(tileset_surface); // Libérer la surface, elle n'est plus nécessaire
    if (!tileset) {
        fprintf(stderr, "Erreur création texture : %s\n", SDL_GetError());
        SDL_DestroyRenderer(rendu);
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Dessin
    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
    SDL_RenderClear(rendu);

    g_config = local_config;

    for (int y = 0; y < lignes ; y++) {
        for (int x = 0; x < colonnes; x++) {
            dessiner_tuile(rendu, tileset, murs, x, y, colonnes);
        }
    }

    SDL_RenderPresent(rendu);

    // Boucle d'événements
    SDL_Event e;
    int quitter = 0;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitter = 1;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quitter = 1;
            }
        }
        SDL_Delay(10);
    }

    // Libération des ressources
    SDL_DestroyTexture(tileset);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}



void afficher_labyrinthe_resolu_sdl(int *murs, int lignes, int colonnes, int depart, int destination) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }


    RenderConfig local_config;
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }
    local_config.window_w = dm.w;
    local_config.window_h = dm.h;
    
    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe Résolu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, local_config.window_w, local_config.window_h, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);

    int cell_w = local_config.window_w / colonnes;
    int cell_h = local_config.window_h / lignes;
    local_config.cell_size = (cell_w < cell_h) ? cell_w : cell_h;
    local_config.wall_thickness = (local_config.cell_size / 16.0f > 1) ? (int)(local_config.cell_size / 16.0f) : 1;
    local_config.offset_x = (local_config.window_w - (colonnes * local_config.cell_size)) / 2;
    local_config.offset_y = (local_config.window_h - (lignes * local_config.cell_size)) / 2;


    noeud n;
    int nb_cellules = lignes * colonnes;
    BFS_laby(murs, lignes, colonnes, destination, &n);

    int max_dist = 0;
    for (int i = 0; i < nb_cellules; i++) {
        if (n.distance[i] > max_dist && n.distance[i] != INF) max_dist = n.distance[i];
    }
    if (max_dist == 0) max_dist = 1;


    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
    SDL_RenderClear(rendu);

    // Dessiner le dégradé de couleurs
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int i = y * colonnes + x;
            if (n.distance[i] != INF) {
                float ratio = (float)n.distance[i] / max_dist;
                Uint8 r = 255 * ratio;
                Uint8 b = 255 * (1.0f - ratio);
                SDL_Rect case_rect = {
                    local_config.offset_x + x * local_config.cell_size, 
                    local_config.offset_y + y * local_config.cell_size, 
                    local_config.cell_size, 
                    local_config.cell_size
                };
                SDL_SetRenderDrawColor(rendu, r, 50, b, 255);
                SDL_RenderFillRect(rendu, &case_rect);
            }
        }
    }

    // Dessiner le chemin
    if (n.distance[depart] != INF) {
        SDL_SetRenderDrawColor(rendu, 255, 255, 0, 255);
        int courant = depart;
        while (courant != destination) {
            int parent = n.parent[courant];
            if (parent == -1) break;

            int x1, y1, x2, y2;
            indice_vers_coord(courant, colonnes, &x1, &y1);
            indice_vers_coord(parent, colonnes, &x2, &y2);
            
            // Calculer les centres
            int centre_x1 = local_config.offset_x + x1 * local_config.cell_size + local_config.cell_size / 2;
            int centre_y1 = local_config.offset_y + y1 * local_config.cell_size + local_config.cell_size / 2;
            int centre_x2 = local_config.offset_x + x2 * local_config.cell_size + local_config.cell_size / 2;
            int centre_y2 = local_config.offset_y + y2 * local_config.cell_size + local_config.cell_size / 2;
            

            SDL_RenderDrawLine(rendu, centre_x1, centre_y1, centre_x2, centre_y2);
            SDL_RenderDrawLine(rendu, centre_x1-1, centre_y1, centre_x2-1, centre_y2);
            SDL_RenderDrawLine(rendu, centre_x1+1, centre_y1, centre_x2+1, centre_y2);
            courant = parent;
        }
    }
    
    // Dessiner les murs 
    g_config = local_config; 
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            dessiner_murs_connus(rendu, x, y, murs, colonnes);
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
            Uint8 r = 255 * ratio;
            Uint8 b = 255 * (1.0f - ratio);
            
            SDL_Rect case_rect = {
                g_config.offset_x + (i % colonnes) * g_config.cell_size, 
                g_config.offset_y + (i / colonnes) * g_config.cell_size, 
                g_config.cell_size, 
                g_config.cell_size
            };
            SDL_SetRenderDrawColor(rendu, r, 20, b, 255);
            SDL_RenderFillRect(rendu, &case_rect);
        }
    }
}



void dessiner_chemin(SDL_Renderer* rendu, int* chemin, int nb_etapes, int colonnes) {
    SDL_SetRenderDrawColor(rendu, 255, 255, 0, 100); // Jaune semi-transparent
    for (int i = 0; i < nb_etapes; i++) {
        int cell = chemin[i];
        
        SDL_Rect case_rect = {
            g_config.offset_x + (cell % colonnes) * g_config.cell_size,
            g_config.offset_y + (cell / colonnes) * g_config.cell_size,
            g_config.cell_size,
            g_config.cell_size
        };
        SDL_RenderFillRect(rendu, &case_rect);
    }
}

void dessiner_marqueurs(SDL_Renderer* rendu, int depart, int destination, int colonnes) {
    // Départ en vert
    SDL_Rect depart_rect = {
        g_config.offset_x + (depart % colonnes) * g_config.cell_size,
        g_config.offset_y + (depart / colonnes) * g_config.cell_size,
        g_config.cell_size,
        g_config.cell_size
    };
    SDL_SetRenderDrawColor(rendu, 0, 255, 0, 150);
    SDL_RenderFillRect(rendu, &depart_rect);

    // Destination en rouge
    SDL_Rect dest_rect = {
        g_config.offset_x + (destination % colonnes) * g_config.cell_size,
        g_config.offset_y + (destination / colonnes) * g_config.cell_size,
        g_config.cell_size,
        g_config.cell_size
    };
    SDL_SetRenderDrawColor(rendu, 255, 0, 0, 150);
    SDL_RenderFillRect(rendu, &dest_rect);
}

// Dessine le sprite du personnage à sa position en pixels
void dessiner_personnage(SDL_Renderer* rendu, SDL_Texture* perso_texture, float x_pixel, float y_pixel) {
    SDL_Rect dst_rect = {
        (int)(x_pixel - g_config.cell_size / 2),
        (int)(y_pixel - g_config.cell_size / 2),
        g_config.cell_size,
        g_config.cell_size
    };
    SDL_RenderCopy(rendu, perso_texture, NULL, &dst_rect);
}

void dessiner_heatmap_passage(SDL_Renderer* rendu, int* passages, int lignes, int colonnes, int max_passages) {
    if (max_passages == 0) return;
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < lignes * colonnes; i++) {
        if (passages[i] > 0) {
            float ratio = (float)passages[i] / max_passages;
            Uint8 alpha = (Uint8)(ratio * 200);
            
            
            SDL_Rect case_rect = {
                g_config.offset_x + (i % colonnes) * g_config.cell_size,
                g_config.offset_y + (i / colonnes) * g_config.cell_size,
                g_config.cell_size,
                g_config.cell_size
            };
            SDL_SetRenderDrawColor(rendu, 255, 100, 0, alpha);
            SDL_RenderFillRect(rendu, &case_rect);
        }
    }
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_NONE);
}

// ======================================================= fin Sdl ========================================
