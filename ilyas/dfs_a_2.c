#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "labySDL.h"
#include "laby.h"
#define MAX_NODES 1000


int ba(avl *n) {
    return n ? hauteur(n->gauche) - hauteur(n->droit) : 0;
}

avl *insererAVL(avl *node, int sommet, int *equilibre) {
    if (!node) {
        avl *nouveau = malloc(sizeof(avl));
        nouveau->sommet = sommet;
        nouveau->estimation = 0;
        nouveau->gauche = nouveau->droit = NULL;
        nouveau->hauteur = 1;
        return nouveau;
    }

    if (sommet < node->sommet)
        node->gauche = insererAVL(node->gauche, sommet, equilibre);
    else if (sommet > node->sommet)
        node->droit = insererAVL(node->droit, sommet, equilibre);
    else
        return node;

    node->hauteur = 1 + max(hauteur(node->gauche), hauteur(node->droit));
    int balance = ba(node);

    if (balance > 1 && sommet < node->gauche->sommet)
        return rotationDroite(node);
    if (balance < -1 && sommet > node->droit->sommet)
        return rotationGauche(node);
    if (balance > 1 && sommet > node->gauche->sommet) {
        node->gauche = rotationGauche(node->gauche);
        return rotationDroite(node);
    }
    if (balance < -1 && sommet < node->droit->sommet) {
        node->droit = rotationDroite(node->droit);
        return rotationGauche(node);
    }

    return node;
}

int estDansAVL(avl *racine, int sommet) {
    if (!racine) return 0;
    if (sommet == racine->sommet) return 1;
    return (sommet < racine->sommet)
        ? estDansAVL(racine->gauche, sommet)
        : estDansAVL(racine->droit, sommet);
}

void collecterAVL(avl *racine, int *tableau, int *indice) {
    if (!racine) return;
    collecterAVL(racine->gauche, tableau, indice);
    tableau[(*indice)++] = racine->sommet;
    collecterAVL(racine->droit, tableau, indice);
}

void libererAVL(avl *racine) {
    if (!racine) return;
    libererAVL(racine->gauche);
    libererAVL(racine->droit);
    free(racine);
}
noeud* initNoeud(int n) {
    noeud* nd = malloc(sizeof(noeud));
    nd->distance = malloc(n * sizeof(int));
    nd->visite = malloc(n * sizeof(int));
    nd->parent = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        nd->distance[i] = INT_MAX;
        nd->visite[i] = 0;
        nd->parent[i] = -1;
    }
    return nd;
}

void libererNoeud(noeud* nd) {
    if (nd) {
        free(nd->distance);
        free(nd->visite);
        free(nd->parent);
        free(nd);
    }
}

int dijkstra(arete *aretes, int nbAretes, int n, int src, int dest, int *chemin, noeud *nd) {
    for (int i = 0; i < n; i++) {
        nd->distance[i] = INT_MAX;
        nd->visite[i] = 0;
        nd->parent[i] = -1;
    }
    nd->distance[src] = 0;

    for (int i = 0; i < n; i++) {
        int u = -1, min = INT_MAX;
        for (int j = 0; j < n; j++) {
            if (!nd->visite[j] && nd->distance[j] < min) {
                u = j;
                min = nd->distance[j];
            }
        }

        if (u == -1) break;
        nd->visite[u] = 1;

        for (int k = 0; k < nbAretes; k++) {
            int v = -1, poids = aretes[k].poids;
            if (aretes[k].u == u) v = aretes[k].v;
            else if (aretes[k].v == u) v = aretes[k].u;

            if (v != -1 && !nd->visite[v] && nd->distance[u] + poids < nd->distance[v]) {
                nd->distance[v] = nd->distance[u] + poids;
                nd->parent[v] = u;
            }
        }
    }

    if (nd->distance[dest] == INT_MAX) return 0;

    int index = 0, temp = dest;
    while (temp != -1) {
        chemin[index++] = temp;
        temp = nd->parent[temp];
    }

    for (int i = 0; i < index / 2; i++) {
        int tmp = chemin[i];
        chemin[i] = chemin[index - i - 1];
        chemin[index - i - 1] = tmp;
    }

    return index;
}
int destinationGlobale = -1;

int estDestination(int sommet) {
    return sommet == destinationGlobale;
}
void explorerGrapheIntelligent(int origine, int nbSommets, arete* aretes, int nbAretes) {
    avl *F = NULL; // Fermés : déjà visités
    avl *O = NULL; // Ouverts : découverts mais non visités
    noeud* nd = initNoeud(nbSommets); // Pour distances, parents, etc.
    int courant = origine;
    int equilibre;

    while (!estDestination(courant)) {
        // 1. Marquer comme visité
        F = insererAVL(F, courant, &equilibre);

        // 2. Mettre à jour O : découvrir les voisins de courant
        int voisins[MAX_NODES];
        int nbVoisins = 0;

        for (int i = 0; i < nbAretes; i++) {
            int voisin = -1;
            if (aretes[i].u == courant && !estDansAVL(F, aretes[i].v)) {
                voisin = aretes[i].v;
            } else if (aretes[i].v == courant && !estDansAVL(F, aretes[i].u)) {
                voisin = aretes[i].u;
            }

            if (voisin != -1 && !estDansAVL(O, voisin)) {
                voisins[nbVoisins++] = voisin;
                O = insererAVL(O, voisin, &equilibre);
            }
        }

        int suivant = -1;

        // 3. Si on a des voisins directs non visités, en choisir un
        if (nbVoisins > 0) {
            suivant = voisins[0]; // DFS: prend le premier non visité
        } else {
            // 4. Sinon, on va chercher le plus proche dans O via Dijkstra
            int candidats[MAX_NODES], nCandidats = 0;
            collecterAVL(O, candidats, &nCandidats);

            int minDist = INT_MAX;

            for (int i = 0; i < nCandidats; i++) {
                int cible = candidats[i];

                if (estDansAVL(F, cible)) continue;

                int chemin[MAX_NODES];
                int longueur = dijkstra(aretes, nbAretes, nbSommets, courant, cible, chemin, nd);

                if (longueur > 0 && nd->distance[cible] < minDist) {
                    minDist = nd->distance[cible];
                    suivant = cible;
                }
            }

            if (suivant != -1) {
                int chemin[MAX_NODES];
                int longueur = dijkstra(aretes, nbAretes, nbSommets, courant, suivant, chemin, nd);

                printf("Chemin indirect de %d à %d via : ", courant, suivant);
                for (int i = 0; i < longueur; i++) {
                    printf("%d ", chemin[i]);
                }
                printf("\n");
            }
        }

        // 5. Si aucun suivant possible
        if (suivant == -1 || suivant == courant) {
            printf("Impossible de continuer l'exploration depuis %d\n", courant);
            break;
        }

        printf("Déplacement de %d à %d\n", courant, suivant);
        courant = suivant;
    }

    if (estDestination(courant)) {
        printf("Destination atteinte : %d\n", courant);
    }

    libererNoeud(nd);
    libererAVL(F);
    libererAVL(O);
}
// Assure-toi d’avoir inclus toutes les définitions précédentes ici...

int main() {
    int nbSommets = 12;
    int origine = 0;
    destinationGlobale = 11;  // définir la destination globale ici

    // Graphe : exemple d’un labyrinthe ou réseau 3x4
    arete aretes[] = {
        {0, 1, 1}, {1, 2, 1}, {2, 3, 1},
        {0, 4, 1}, {1, 5, 1}, {2, 6, 1}, {3, 7, 1},
        {4, 5, 1}, {5, 6, 1}, {6, 7, 1},
        {4, 8, 1}, {5, 9, 1}, {6, 10, 1}, {7, 11, 1},
        {8, 9, 1}, {9, 10, 1}, {10, 11, 1}
    };
    int nbAretes = sizeof(aretes) / sizeof(aretes[0]);

    printf("Départ depuis le sommet %d vers la destination %d.\n", origine, destinationGlobale);

    explorerGrapheIntelligent(aretes, nbAretes, nbSommets, origine);

    return 0;
}
*/