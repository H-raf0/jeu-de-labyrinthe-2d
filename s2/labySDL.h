#ifndef LABYSDL_H
#define LABYSDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "laby.h"
#include "structures.h"
// Constantes
#define TUILE_TAILLE 16
#define TAILLE_CELLULE 50



// Tableau global contenant les zones du tileset
extern SDL_Rect src_murs[16];

// Fonctions pour afficher un labyrinthe en SDL
void dessiner_murs(SDL_Renderer* rendu, int x, int y, int *murs, int colonnes);
void dessiner_murs_connus(SDL_Renderer* rendu, int x, int y, int *murs, int colonnes); 
void afficher_labyrinthe_sdl(arete arbre[], int nb_aretes, int lignes, int colonnes);

// Version avec tuiles
void dessiner_tuile(SDL_Renderer* rendu, SDL_Texture* tileset, int* murs, int x, int y, int colonnes);
void afficher_labyrinthe_sdl_tuiles(int *murs, int lignes, int colonnes);


// Fonctions de dessin modulaires pour l'animation
void dessiner_fond(SDL_Renderer* rendu, noeud* n, int lignes, int colonnes);
void dessiner_chemin(SDL_Renderer* rendu, int* chemin, int nb_etapes, int colonnes);
void dessiner_personnage(SDL_Renderer* rendu, SDL_Texture* perso_texture, float x_pixel, float y_pixel);
void dessiner_marqueurs(SDL_Renderer* rendu, int depart, int destination, int colonnes);

//================== Affichage SDL avec solution =================
void afficher_labyrinthe_resolu_sdl(int *murs, int lignes, int colonnes, int depart, int destination);



#endif
