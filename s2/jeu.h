#ifndef JEU_H
#define JEU_H

#include "laby.h"
#include "labySDL.h"
#include "audio.h"
#include "endscreen.h" 


extern int NOMBRE_MONSTRES;
extern int SEUIL_DETECTION_HUNT;
extern int DUREE_PISTE;
extern int RAPP_CLDWN;
extern int MEMOIRE_MAX;
extern int VITESSE_MONSTRE;
extern int MONSTRE_PENALITE_RAYON;
extern int MONSTRE_PENALITE_COUT;

extern int SAUT_COOLDOWN; // Cooldown en frames

extern int NOMBRE_PIECES;


// Déclaration de la fonction qui lance le jeu.
// Elle prend en paramètre le rendu SDL pour ne pas avoir à le recréer.
// Elle prend aussi le labyrinthe déjà généré.
GameResult lancer_jeu(SDL_Renderer* rendu, int* murs_reels, int lignes, int colonnes, AudioData* audio);;

#endif