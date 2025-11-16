#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "structures.h"


/**********************************************
                    BFS
***********************************************/
void BFS(int m_adj[N][N], int origine,noeud* n){
    file * f = malloc(sizeof(file));
    initialiser_file(f);
    initialiser_noeuds(n ,origine);
    n->visite[origine]=1;
    enfiler(f,origine);
    while(!filevide(f)){
        int x=defiler(f);
        for(int k=0;k<N;++k){
            if(n->visite[k]==0 && m_adj[x][k]==1){
                enfiler(f,k);
                n->visite[k]=1;
                n->distance[k]=n->distance[x]+1;
            }
        }
    }
    free(f);
}

/**********************************************
                DIJKSTRA    
***********************************************/
void dijkstra(int graphe[N][N], int origine, noeud *n) {
    tas t;
    t.taille = 0;
    initialiser_noeuds(n,origine);
    inserer(&t, origine, n);
    while (t.taille > 0) {
        int u = extraire_min(&t, n);
        if (n->visite[u]) continue;
        n->visite[u] = 1;

        for (int v = 0; v < N; v++) {
            if (graphe[u][v] > 0 && !n->visite[v]) {
                int alt = n->distance[u] + graphe[u][v];
                if (alt < n->distance[v]) {
                    n->distance[v] = alt;
                    inserer(&t, v, n);
                }
            }
        }
    }
}
/**********************************************
                A_etoile 
***********************************************/
void A_etoile(int graphe[N][N], int depart, int arrivee, int positions[N][2]) {
    int g[N], parent[N];
    bool ferme[N] = {false};

    for (int i = 0; i < N; i++) {
        g[i] = INF;
        parent[i] = -1;
    }

    g[depart] = 0;
    int h = estimation(positions[depart][0], positions[arrivee][0], positions[depart][1], positions[arrivee][1], 1);
    avl* ouvert = NULL;
    ouvert = inserer_avl(ouvert, depart, g[depart] + h);

    while (ouvert != NULL) {
        int u;
        ouvert = extraire_min_avl(ouvert, &u);

        if (u == arrivee) {
            printf("Chemin trouve !\n");
            int temp = arrivee;
            while (temp != -1) {
                printf("%d <- ", temp);
                temp = parent[temp];
            }
            printf("Depart\n");
            return;
        }

        ferme[u] = true;

        for (int v = 0; v < N; v++) {
            if (graphe[u][v] && !ferme[v]) {
                int tentative_g = g[u] + graphe[u][v];
                if (tentative_g < g[v]) {
                    g[v] = tentative_g;
                    parent[v] = u;
                    int h = estimation(positions[v][0], positions[arrivee][0], positions[v][1], positions[arrivee][1], 1);
                    ouvert = inserer_avl(ouvert, v, g[v] + h);
                }
            }
        }
    }

    printf("Aucun chemin trouvé.\n");
}


int main() {
  
    int matrice[N][N] = {0};
    matrice[0][1] = matrice[1][0] = 1;
    matrice[0][2] = matrice[2][0] = 1;
    matrice[1][3] = matrice[3][1] = 1;
    matrice[1][4] = matrice[4][1] = 1;
    matrice[2][5] = matrice[5][2] = 1;
    matrice[2][6] = matrice[6][2] = 1;
    matrice[5][7] = matrice[7][5] = 1;

    noeud * n = malloc(sizeof(noeud));
    if (n == NULL) {
        printf("Erreur d'allocation mémoire\n");
        return 1;
    }
    BFS(matrice, 2, n);
    for (int i = 0; i < N; i++) {
        printf("Distance de 2 a %d : %d\n", i, n->distance[i]);
    }
    free(n);
 /*
    noeud * n=malloc(sizeof(noeud));
    int matrice[N][N] = {
        {0, 4, 2, 0, 0, 0, 0, 0},
        {4, 0, 1, 5, 3, 0, 0, 0},
        {2, 1, 0, 0, 0, 7, 4, 0},
        {0, 5, 0, 0, 0, 0, 0, 0},
        {0, 3, 0, 0, 0, 0, 0, 0},
        {0, 0, 7, 0, 0, 0, 2, 1},
        {0, 0, 4, 0, 0, 2, 0, 6},
        {0, 0, 0, 0, 0, 1, 6, 0}
    };
    dijkstra(matrice, 0, n);
    for (int i = 0; i < N; i++) {
        printf("Distance minimale de 0 à %d : %d\n", i, n->distance[i]);
    }
    free(n);

    // Graphe sous forme de matrice d'adjacence
    int graphe[N][N] = {
        {0, 1, 4, 0, 0},
        {1, 0, 2, 5, 0},
        {4, 2, 0, 1, 3},
        {0, 5, 1, 0, 2},
        {0, 0, 3, 2, 0}
    };

    // Position (x, y) de chaque sommet (utilisées pour l’estimation heuristique)
    int positions[N][2] = {
        {0, 0}, // sommet 0
        {1, 0}, // sommet 1
        {1, 1}, // sommet 2
        {2, 1}, // sommet 3
        {2, 2}  // sommet 4
    };

    int depart = 0;
    int arrivee = 4;

    A_etoile(graphe, depart, arrivee, positions);
*/
    return 0;
}
 