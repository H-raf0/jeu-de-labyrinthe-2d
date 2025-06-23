#ifndef STRUCTURES_H
#define STRUCTURES_H


#include <stdlib.h>
#include <stdio.h>
#include<stdbool.h>
#include <math.h>

#define N 8
#define INF 9999

//structures:

typedef struct noeud{
    int distance[N];
    int visite[N];
}noeud;

typedef struct file{
    int tab[N];
    int tete;
    int queue;
}file;

typedef struct tas {
    int tab[N];
    int taille;
}tas;

typedef struct avl {
    int sommet;
    int estimation;
    struct avl *gauche;
    struct avl *droit;
    int hauteur;
} avl;



//fonctions sur les noeuds:

void initialiser_noeuds(noeud* n,int origine);


//fonctions sur les files:

void initialiser_file(file * f);
int filevide(file *f);
void enfiler(file * f , int x);
int defiler(file *f);

//fonctions sur les tas:

void echanger(int *a, int *b);
void entasser_haut(tas *t, int i, noeud *n);
void entasser_bas(tas *t, int i, noeud *n);
void inserer(tas *t, int sommet, noeud *n);
int extraire_min(tas *t, noeud *n);
void afficher_tas(tas *t, noeud *n);

//fonctions sur les avl;

int max(int a, int b);
int hauteur(avl *n);
avl* rotation_gauche(avl *x);
avl* rotation_droite(avl *y);
int balance(avl *n);
avl* inserer_avl(avl *racine, int sommet, int estimation);
avl* extraire_min_avl(avl *racine, int *sommet);

//fonctions sur les distances:
int max(int a, int b);
int estimation(int x1,int x2, int y1, int y2,int type);

//algorithmes:
void BFS(int m_adj[N][N], int origine,noeud* n);
void dijkstra(int graphe[N][N], int origine, noeud *n);
void A_etoile(int graphe[N][N], int depart, int arrivee, int positions[N][2]);

#endif