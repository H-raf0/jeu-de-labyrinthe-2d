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

//================== BFS/DIj/A* ==============================

/*
// à supprimer ???
void BFS(int m_adj[N][N], int origine,noeud* n);
void dijkstra(int graphe[N][N], int origine, noeud *n);
void A_etoile(int graphe[N][N], int depart, int arrivee, int positions[N][2]); //?????
*/
//================== Résolution de labyrinthe ====================
void BFS_laby(int *murs, int lignes, int colonnes, int origine, noeud* n);

//================== Affichage ===============================
void afficher_labyrinthe_unicode(int *murs, int lignes, int colonnes);

#endif
