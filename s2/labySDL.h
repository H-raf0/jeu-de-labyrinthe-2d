#ifndef LABYSDL_H
#define LABYSDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "laby.h"
#include "structures.h"
// Constantes
#define TUILE_TAILLE 16
#define TAILLE_CELLULE 50

//#define EPES 3 // epesseur des murs 




typedef struct {
    int window_w;         // Largeur de la fenêtre en pixels
    int window_h;         // Hauteur de la fenêtre en pixels
    int cell_size;        // Taille d'une cellule calculée en pixels
    int wall_thickness;   // Épaisseur d'un mur calculée en pixels
    int offset_x;         // Marge horizontale pour centrer le labyrinthe
    int offset_y;         // Marge verticale pour centrer le labyrinthe
} RenderConfig;


extern RenderConfig g_config;





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
