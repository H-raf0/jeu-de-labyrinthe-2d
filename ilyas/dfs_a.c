#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_NODES 100

// Structures fournies
typedef struct {
    int u, v;
} arete;

typedef struct {
    int *distance; // Pointeur pour allocation dynamique
    int *visite;   // Pointeur pour allocation dynamique
    int *parent;   // Pointeur pour reconstruire le chemin
} noeud;

typedef struct {
    int *tab;      // Pointeur pour allocation dynamique
    int tete;
    int queue;
    int capacite;  // Pour gérer la taille
} file;

typedef struct tas {
    int *tab;
    int taille;
} tas;

typedef struct avl {
    int sommet;
    int estimation; // Non utilisé ici, mais inclus pour conformité
    struct avl *gauche;
    struct avl *droit;
    int hauteur;
} avl;

// Fonctions utilitaires pour la file
file* initFile(int capacite) {
    file* f = (file*)malloc(sizeof(file));
    f->tab = (int*)malloc(capacite * sizeof(int));
    f->tete = 0;
    f->queue = 0;
    f->capacite = capacite;
    return f;
}

void enfiler(file* f, int x) {
    if ((f->queue + 1) % f->capacite != f->tete) {
        f->tab[f->queue] = x;
        f->queue = (f->queue + 1) % f->capacite;
    }
}

int defiler(file* f) {
    if (f->tete == f->queue) return -1;
    int x = f->tab[f->tete];
    f->tete = (f->tete + 1) % f->capacite;
    return x;
}

int fileVide(file* f) {
    return f->tete == f->queue;
}

void libererFile(file* f) {
    free(f->tab);
    free(f);
}

// Fonctions utilitaires pour le tas
tas* initTas(int capacite) {
    tas* t = (tas*)malloc(sizeof(tas));
    t->tab = (int*)malloc(capacite * sizeof(int));
    t->taille = 0;
    return t;
}

void echanger(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void entasserMin(tas* t, int i, int* distances) {
    int gauche = 2 * i + 1;
    int droit = 2 * i + 2;
    int min = i;

    if (gauche < t->taille && distances[t->tab[gauche]] < distances[t->tab[min]])
        min = gauche;
    if (droit < t->taille && distances[t->tab[droit]] < distances[t->tab[min]])
        min = droit;

    if (min != i) {
        echanger(&t->tab[i], &t->tab[min]);
        entasserMin(t, min, distances);
    }
}

void insererTas(tas* t, int x, int* distances) {
    t->tab[t->taille] = x;
    int i = t->taille++;
    while (i > 0 && distances[t->tab[(i - 1) / 2]] > distances[t->tab[i]]) {
        echanger(&t->tab[i], &t->tab[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

int extraireMin(tas* t, int* distances) {
    if (t->taille == 0) return -1;
    int min = t->tab[0];
    t->tab[0] = t->tab[--t->taille];
    entasserMin(t, 0, distances);
    return min;
}

void libererTas(tas* t) {
    free(t->tab);
    free(t);
}

// Fonctions utilitaires pour l'AVL
int max(int a, int b) {
    return a > b ? a : b;
}

int hauteur(avl* a) {
    return a ? a->hauteur : 0;
}

avl* rotationGauche(avl* a) {
    avl* b = a->droit;
    a->droit = b->gauche;
    b->gauche = a;
    a->hauteur = max(hauteur(a->gauche), hauteur(a->droit)) + 1;
    b->hauteur = max(hauteur(b->gauche), hauteur(b->droit)) + 1;
    return b;
}

avl* rotationDroite(avl* a) {
    avl* b = a->gauche;
    a->gauche = b->droit;
    b->droit = a;
    a->hauteur = max(hauteur(a->gauche), hauteur(a->droit)) + 1;
    b->hauteur = max(hauteur(b->gauche), hauteur(b->droit)) + 1;
    return b;
}

int equilibre(avl* a) {
    return a ? hauteur(a->gauche) - hauteur(a->droit) : 0;
}

avl* insererAVL(avl* a, int sommet, int* equilibreModifie) {
    if (!a) {
        a = (avl*)malloc(sizeof(avl));
        a->sommet = sommet;
        a->estimation = 0; // Non utilisé
        a->gauche = a->droit = NULL;
        a->hauteur = 1;
        *equilibreModifie = 1;
        return a;
    }
    if (sommet < a->sommet) {
        a->gauche = insererAVL(a->gauche, sommet, equilibreModifie);
    } else if (sommet > a->sommet) {
        a->droit = insererAVL(a->droit, sommet, equilibreModifie);
    } else {
        *equilibreModifie = 0;
        return a;
    }

    if (*equilibreModifie) {
        a->hauteur = max(hauteur(a->gauche), hauteur(a->droit)) + 1;
        int eq = equilibre(a);
        if (eq > 1) {
            if (equilibre(a->gauche) < 0)
                a->gauche = rotationGauche(a->gauche);
            a = rotationDroite(a);
        } else if (eq < -1) {
            if (equilibre(a->droit) > 0)
                a->droit = rotationDroite(a->droit);
            a = rotationGauche(a);
        }
    }
    return a;
}

int estDansAVL(avl* a, int sommet) {
    if (!a) return 0;
    if (a->sommet == sommet) return 1;
    return estDansAVL(sommet < a->sommet ? a->gauche : a->droit, sommet);
}

void collecterAVL(avl* a, int* tab, int* index) {
    if (!a) return;
    collecterAVL(a->gauche, tab, index);
    tab[(*index)++] = a->sommet;
    collecterAVL(a->droit, tab, index);
}

void libererAVL(avl* a) {
    if (!a) return;
    libererAVL(a->gauche);
    libererAVL(a->droit);
    free(a);
}

// Initialisation du noeud
noeud* initNoeud(int n) {
    noeud* nd = (noeud*)malloc(sizeof(noeud));
    nd->distance = (int*)malloc(n * sizeof(int));
    nd->visite = (int*)malloc(n * sizeof(int));
    nd->parent = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        nd->distance[i] = INT_MAX;
        nd->visite[i] = 0;
        nd->parent[i] = -1;
    }
    return nd;
}

void libererNoeud(noeud* nd) {
    free(nd->distance);
    free(nd->visite);
    free(nd->parent);
    free(nd);
}

// Dijkstra avec tas
int dijkstra(arete* aretes, int nbAretes, int n, int src, int dest, int* chemin, noeud* nd) {
    tas* t = initTas(n);
    for (int i = 0; i < n; i++) {
        nd->distance[i] = INT_MAX;
        nd->visite[i] = 0;
        nd->parent[i] = -1;
    }
    nd->distance[src] = 0;
    insererTas(t, src, nd->distance);

    while (t->taille > 0) {
        int u = extraireMin(t, nd->distance);
        if (u == -1 || u == dest) break;
        nd->visite[u] = 1;

        for (int i = 0; i < nbAretes; i++) {
            int v = -1, w = 1; // Poids = 1 pour toutes les arêtes
            if (aretes[i].u == u && !nd->visite[aretes[i].v]) {
                v = aretes[i].v;
            } else if (aretes[i].v == u && !nd->visite[aretes[i].u]) {
                v = aretes[i].u;
            }
            if (v != -1 && nd->distance[u] + w < nd->distance[v]) {
                nd->distance[v] = nd->distance[u] + w;
                nd->parent[v] = u;
                insererTas(t, v, nd->distance);
            }
        }
    }

    int longueur = 0;
    if (nd->distance[dest] != INT_MAX) {
        int courant = dest;
        while (courant != -1) {
            chemin[longueur++] = courant;
            courant = nd->parent[courant];
        }
        for (int i = 0; i < longueur / 2; i++) {
            int temp = chemin[i];
            chemin[i] = chemin[longueur - 1 - i];
            chemin[longueur - 1 - i] = temp;
        }
    }

    libererTas(t);
    return nd->distance[dest] == INT_MAX ? -1 : longueur;
}

// Vérifier si un noeud est la destination (exemple)
int estDestination(int noeud) {
    return noeud == 15; // Exemple : noeud 15
}

// Algorithme principal
void explorerGraphe(int debut, int nbSommets, arete* aretes, int nbAretes) {
    avl* F = NULL; // Noeuds visités
    avl* O = NULL; // Noeuds découverts non visités
    noeud* nd = initNoeud(nbSommets);
    int courant = debut;
    int equilibreModifie;

    while (!estDestination(courant)) {
        F = insererAVL(F, courant, &equilibreModifie); // Ajouter à F
        int suivant = -1;
        int minDist = INT_MAX;

        // Découvrir les voisins
        for (int i = 0; i < nbAretes; i++) {
            int v = -1;
            if (aretes[i].u == courant && !estDansAVL(F, aretes[i].v)) {
                v = aretes[i].v;
            } else if (aretes[i].v == courant && !estDansAVL(F, aretes[i].u)) {
                v = aretes[i].u;
            }
            if (v != -1) {
                if (!estDansAVL(O, v)) {
                    O = insererAVL(O, v, &equilibreModifie);
                }
                if (1 < minDist) { // Poids = 1
                    minDist = 1;
                    suivant = v;
                }
            }
        }

        // Si aucun voisin direct, trouver le noeud le plus proche dans O
        if (suivant == -1) {
            int noeuds[MAX_NODES], index = 0;
            collecterAVL(O, noeuds, &index);
            for (int i = 0; i < index; i++) {
                int chemin[MAX_NODES];
                int longueur = dijkstra(aretes, nbAretes, nbSommets, courant, noeuds[i], chemin, nd);
                if (longueur != -1 && nd->distance[noeuds[i]] < minDist) {
                    minDist = nd->distance[noeuds[i]];
                    suivant = noeuds[i];
                }
            }
        }

        if (suivant == -1) {
            printf("Aucun chemin restant à explorer.\n");
            break;
        }

        // Se déplacer vers le suivant
        int chemin[MAX_NODES];
        int longueur = dijkstra(aretes, nbAretes, nbSommets, courant, suivant, chemin, nd);
        if (longueur != -1) {
            printf("Déplacement de %d à %d via : ", courant, suivant);
            for (int i = 0; i < longueur; i++) {
                printf("%d ", chemin[i]);
            }
            printf("\n");
        }
        courant = suivant;
    }

    if (estDestination(courant)) {
        printf("Destination %d atteinte !\n", courant);
    }

    libererNoeud(nd);
    libererAVL(F);
    libererAVL(O);
}

int main() {
    int nbSommets = 25;
    arete aretes[] = {
        {24, 1}, {1, 10}, {10, 11},
        {23, 2}, {2, 9}, {9, 12},
        {22, 3}, {3, 8}, {8, 13},
        {21, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 14},
        {20, 19}, {19, 18}, {18, 17}, {17, 16}, {16, 15}
    };
    int nbAretes = sizeof(aretes) / sizeof(aretes[0]);
    explorerGraphe(24, nbSommets, aretes, nbAretes);
    return 0;
}
*/