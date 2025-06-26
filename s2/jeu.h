#ifndef JEU_H
#define JEU_H

#include "laby.h"
#include "labySDL.h"

// Déclaration de la fonction qui lance le jeu.
// Elle prend en paramètre le rendu SDL pour ne pas avoir à le recréer.
// Elle prend aussi le labyrinthe déjà généré.
void lancer_jeu(SDL_Renderer* rendu, int* murs_reels, int lignes, int colonnes);

#endif