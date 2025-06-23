#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "laby.h"
#include "labySDL.h"


// main.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "laby.h"
#include "labySDL.h"

int main(int argc, char* argv[]) {
    srand(time(NULL));

    // Définir la taille du labyrinthe
    int lignes = 20;
    int colonnes = 30;
    int nb_cellules = lignes * colonnes;

    // 1. Génération du labyrinthe (votre code existant)
    arete *toutes_aretes;
    int nb_total_aretes = generation_grille_vide(&toutes_aretes, lignes, colonnes);
    
    fisher_yates(toutes_aretes, nb_total_aretes);

    arete *arbre = malloc(sizeof(arete) * (nb_cellules - 1));
    int nb_aretes_arbre;
    construire_arbre_couvrant(toutes_aretes, nb_total_aretes, arbre, &nb_aretes_arbre, nb_cellules);
    
    free(toutes_aretes);

    // Créer la représentation avec les murs
    int *murs = malloc(sizeof(int) * nb_cellules);
    for (int i = 0; i < nb_cellules; i++) {
        murs[i] = 1 | 2 | 4 | 8; // Tous les murs sont présents au début
    }
    for (int i = 0; i < nb_aretes_arbre; i++) {
        supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);
    }
    free(arbre);
    
    // Définir le point de départ et d'arrivée
    int depart = 0; // Coin supérieur gauche
    int destination = nb_cellules - 1; // Coin inférieur droit

    // 2. Afficher le labyrinthe résolu avec SDL
    printf("Affichage du labyrinthe résolu avec le chemin et le dégradé de distance...\n");
    afficher_labyrinthe_resolu_sdl(murs, lignes, colonnes, depart, destination);
    
    // Libérer la mémoire des murs
    free(murs);

    printf("Programme terminé.\n");
    return 0;
}



/*
//affichage seul
// Fonction principale
int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <lignes> <colonnes> <SDL-0/1>\n", argv[0]);
        return 1;
    }
    int lignes = atoi(argv[1]);
    int colonnes = atoi(argv[2]);
    int utiliser_sdl = atoi(argv[3]); // console ou SDL
    if (lignes <= 0 || colonnes <= 0) {
        fprintf(stderr, "Dimensions invalides : lignes et colonnes doivent être > 0\n");
        return 1;
    }

    
    srand(time(NULL));
    int total_cellules = lignes * colonnes;
    arete *graphe;
    int nb_aretes = generation_grille_vide(&graphe, lignes, colonnes);
    fisher_yates(graphe, nb_aretes);

    arete *arbre = malloc(sizeof(arete) * total_cellules);
    if (!arbre) {
        fprintf(stderr, "Allocation mémoire impossible pour arbre\n");
        free(graphe);
        return 1;
    }
    int nb_arbre;
    construire_arbre_couvrant(graphe, nb_aretes, arbre, &nb_arbre, total_cellules);
    //generer_dot("graphe.dot", graphe, nb_aretes);
    //generer_dot("arbre.dot", arbre, nb_arbre);
    //printf("Fichiers DOT générés : graphe.dot et arbre.dot\n");

    int *murs = malloc(sizeof(int) * total_cellules);
    if (!murs) {
        fprintf(stderr, "Allocation mémoire impossible pour murs\n");
        free(graphe);
        free(arbre);
        return 1;
    }
    for (int i = 0; i < total_cellules; i++) murs[i] = 1|2|4|8;
    for (int i = 0; i < nb_arbre; i++) {
        supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);
    }

    if (utiliser_sdl) {
        afficher_labyrinthe_sdl(arbre, nb_arbre, lignes, colonnes);
        afficher_labyrinthe_sdl_tuiles(murs, lignes, colonnes);
    } else {
        afficher_labyrinthe_unicode(murs, lignes, colonnes);
    }

    free(graphe);
    free(arbre);
    free(murs);
    return 0;
}
*/