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




