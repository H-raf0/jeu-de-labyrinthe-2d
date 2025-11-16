#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "structures.h"


/*----------------------------------------------*/
//fonctions sur les noeuds:

void initialiser_noeuds(noeud* n,int origine){
    for(int i=0;i<N;++i){
        n->visite[i]=0;
        n->distance[i]=INF;
    }
    n->distance[origine]=0;
}

//fonctions sur les files:

void initialiser_file(file * f){
    f->tete=0;
    f->queue=0;
}

int filevide(file *f){
    return f->tete==f->queue;
}

void enfiler(file * f , int x){
    if(f->queue==N){
        printf("file pleine\n");
        return;
    }
    f->tab[f->queue]=x;
    f->queue = (f->queue + 1) % N;
}

int defiler(file *f){
    if(filevide(f)){
        printf("file vide\n");
        return 0;
    }
    int x=f->tab[f->tete];
    f->tete = (f->tete + 1) % N;
    return x;
}

//fonctions sur les tas:
void echanger(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void entasser_haut(tas *t, int i, noeud *n) {
    while (i > 0 && n->distance[t->tab[i]] < n->distance[t->tab[(i - 1) / 2]]) {
        echanger(&t->tab[i], &t->tab[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void entasser_bas(tas *t, int i, noeud *n) {
    int min = i;
    int g = 2 * i + 1;
    int d = 2 * i + 2;

    if (g < t->taille && n->distance[t->tab[g]] < n->distance[t->tab[min]])
        min = g;
    if (d < t->taille && n->distance[t->tab[d]] < n->distance[t->tab[min]])
        min = d;

    if (min != i) {
        echanger(&t->tab[i], &t->tab[min]);
        entasser_bas(t, min, n);
    }
}

void inserer(tas *t, int sommet, noeud *n) {
    t->tab[t->taille] = sommet;
    entasser_haut(t, t->taille, n);
    t->taille++;
}

int extraire_min(tas *t, noeud *n) {
    int min = t->tab[0];
    t->tab[0] = t->tab[--t->taille];
    entasser_bas(t, 0, n);
    return min;
}
void afficher_tas(tas *t, noeud *n) {
    printf("Tas actuel : [ ");
    for (int i = 0; i < t->taille; i++) {
        int s = t->tab[i];
        printf("(%d, dist=%d) ", s, n->distance[s]);
    }
    printf("]\n");
}

//fonctions sur les AVL:

int max(int a, int b) { return a > b ? a : b; }
int hauteur(avl *n) { return n ? n->hauteur : 0; }

avl* rotation_gauche(avl *x) {
    avl *y = x->droit, *T2 = y->gauche;
    y->gauche = x; x->droit = T2;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droit)) + 1;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droit)) + 1;
    return y;
}

avl* rotation_droite(avl *y) {
    avl *x = y->gauche, *T2 = x->droit;
    x->droit = y; y->gauche = T2;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droit)) + 1;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droit)) + 1;
    return x;
}

int balance(avl *n) {
    return n ? hauteur(n->gauche) - hauteur(n->droit) : 0;
}

avl* inserer_avl(avl *racine, int sommet, int estimation) {
    if (!racine) {
        avl *nouv = malloc(sizeof(avl));
        nouv->sommet = sommet;
        nouv->estimation = estimation;
        nouv->gauche = nouv->droit = NULL;
        nouv->hauteur = 1;
        return nouv;
    }
    if (estimation < racine->estimation || (estimation == racine->estimation && sommet < racine->sommet))
        racine->gauche = inserer_avl(racine->gauche, sommet, estimation);
    else
        racine->droit = inserer_avl(racine->droit, sommet, estimation);

    racine->hauteur = 1 + max(hauteur(racine->gauche), hauteur(racine->droit));
    int b = balance(racine);
    if (b > 1 && estimation < racine->gauche->estimation)
        return rotation_droite(racine);
    if (b < -1 && estimation > racine->droit->estimation)
        return rotation_gauche(racine);
    if (b > 1 && estimation > racine->gauche->estimation) {
        racine->gauche = rotation_gauche(racine->gauche);
        return rotation_droite(racine);
    }
    if (b < -1 && estimation < racine->droit->estimation) {
        racine->droit = rotation_droite(racine->droit);
        return rotation_gauche(racine);
    }
    return racine;
}

avl* extraire_min_avl(avl *racine, int *sommet) {
    if (!racine)
        return NULL;

    if (!racine->gauche) {
        *sommet = racine->sommet;
        avl *temp = racine->droit;
        free(racine);
        return temp;
    }

    racine->gauche = extraire_min_avl(racine->gauche, sommet);

    racine->hauteur = 1 + max(hauteur(racine->gauche), hauteur(racine->droit));
    
    int b = balance(racine);

    if (b > 1 && balance(racine->gauche) >= 0)
        return rotation_droite(racine);

    if (b > 1 && balance(racine->gauche) < 0) {
        racine->gauche = rotation_gauche(racine->gauche);
        return rotation_droite(racine);
    }

    if (b < -1 && balance(racine->droit) <= 0)
        return rotation_gauche(racine);

    if (b < -1 && balance(racine->droit) > 0) {
        racine->droit = rotation_droite(racine->droit);
        return rotation_gauche(racine);
    }

    return racine;
}



//fonctions sur les distances:
int estimation(int x1,int x2, int y1, int y2,int type){
    int delta_x=abs(x1-x2);
    int delta_y=abs(y1-y2);
    switch(type){
        case 0: //Manhattan
            return delta_x+delta_y;
        case 1: //Euclidienne
            return (int)(sqrt(delta_x*delta_x+delta_y*delta_y));
        case 2: //Tchebychev
            return max(delta_x,delta_y);
        default:
            return 0;
    }
}