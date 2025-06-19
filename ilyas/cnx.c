#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

typedef struct {
    int *parent;
    int *rank;
    int size;
} Partition;


Partition* creer_partition(int n) {
    Partition *p = malloc(sizeof(Partition));
    p->parent = malloc(n * sizeof(int));
    p->rank = calloc(n,sizeof(int));  // Initialisé à 0
    p->size = n;
    for (int i = 0; i < n; i++) p->parent[i] = i;
    return p;
}

int trouver(Partition *p, int x) {
    if (p->parent[x] != x)
        p->parent[x] = trouver(p, p->parent[x]); // compression de chemin
    return p->parent[x];
}

void fusion(Partition *p, int x, int y) {
    int rx = trouver(p, x); //trouver le parent de x
    int ry = trouver(p, y); // trouver le parent de y
    if (rx == ry) return;
    //distinction des cas selon le rang de x et y;
    if (p->rank[rx] < p->rank[ry]) {
        p->parent[rx] = ry;
    } else {
        p->parent[ry] = rx;
        if (p->rank[rx] == p->rank[ry]) p->rank[rx]++;
    }
}
//libération de la partition et les rangs
void liberer_partition(Partition *p) {
    free(p->parent);
    free(p->rank);
    free(p);
}

// Creation aleatoire de la matrice d'adjacence;
int **matrice_adj(int N, float p) {
    int **L = malloc(sizeof(int *) * N);
    for (int i = 0; i < N; i++) {
        L[i] = malloc(sizeof(int) * N);
        for (int j = 0; j < N; j++) {
            L[i][j] = 0; // Initialisation sûre de toute la matrice(tous par 0)
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if (((float)rand() / RAND_MAX) < p) {
                L[i][j] = 1; //générer la matrice d'une manière aléatoire (si noeud x et y en un chemin alors en pose 1 (la matrice est symétrique))
                L[j][i] = 1;
            }
        }
    }

    return L;
}

//La recherche des composantes connexes.
void composantes_connexes_matrice(int **mat, int n, Partition *p) {
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {//matrice sym c pour ca on parcout de i+1
            if (mat[i][j]) {
                fusion(p, i, j);//si il sont 1 cad mat[i][j]==1 ; on les fusionne afin de trouver tous les composantes connexes
            }
        }
    }
}
//génération du graphe .
void ecrire_graphe_dot(const char *filename, int **mat, int n) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Erreur d'ouverture du fichier");
        return;
    }
    fprintf(f, "graph G {\n");
    for (int i = 0; i < n; i++) {
        fprintf(f, "    %d;\n", i);
        for (int j = i + 1; j < n; j++) {
            if (mat[i][j]) {
                fprintf(f, "    %d -- %d;\n", i, j);
            }
        }
    }
    fprintf(f, "}\n");
    fclose(f);
}


void afficher_composantes(Partition *p) {
    for (int i = 0; i < p->size; i++) {
        printf("Noeud %d → classe %d\n", i, trouver(p, i));
    }
}
void afficher_matrice(int **mat, int n) {
    printf("Matrice d’adjacence :\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", mat[i][j]);
        }
        printf("\n");
    }
}

void ecrire_composantes_dot(const char *filename, int **mat, int n, Partition *p) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Erreur d'ouverture du fichier");
        return;
    }
    fprintf(f, "graph G {\n");

    // Grouper les noeuds par composante
    int *racines = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) racines[i] = trouver(p, i);

    for (int r = 0; r < n; r++) {
        // Vérifie si c'est une racine principale (évite doublons)
        bool est_racine = true;
        for (int i = 0; i < r; i++) {
            if (racines[i] == racines[r]) {
                est_racine = false;
                break;
            }
        }
        if (!est_racine) continue;

        fprintf(f, "  les sous graphes _%d {\n", racines[r]);
        fprintf(f, "    label = \"Composante %d\";\n", racines[r]);
        for (int i = 0; i < n; i++) {
            if (trouver(p, i) == racines[r]) {
                fprintf(f, "    %d;\n", i);
                for (int j = i + 1; j < n; j++) {
                    if (mat[i][j] && trouver(p, j) == racines[r]) {
                        fprintf(f, "    %d -- %d;\n", i, j);
                    }
                }
            }
        }
        fprintf(f, "  }\n");
    }

    fprintf(f, "}\n");
    fclose(f);
    free(racines);
}



void afficher_nombres_composantes(Partition *p) {
    int *comptes = calloc(p->size, sizeof(int));
    for (int i = 0; i < p->size; i++) {
        int r = trouver(p, i);
        comptes[r]++;
    }

    printf("\nTaille des composantes connexes :\n");
    for (int i = 0; i < p->size; i++) {
        if (comptes[i] > 0) {
            printf("Composante de racine %d : %d noeud(s)\n", i, comptes[i]);
        }
    }
    free(comptes);
}




/*

int main() {
    srand(time(NULL));
    int n = 12;
    float proba = 0.2;

    int **mat = matrice_adj(n, proba);
    afficher_matrice(mat, n);

    Partition *p = creer_partition(n);
    composantes_connexes_matrice(mat, n, p);
    afficher_composantes(p);
    ecrire_graphe_dot("graphe.dot", mat, n);
    ecrire_composantes_dot("composantes.dot", mat, n, p);
    afficher_nombres_composantes(p);

    // Libération mémoire
    for (int i = 0; i < n; i++) free(mat[i]);
    free(mat);
    liberer_partition(p);
    return 0;
}
*/

typedef struct {
    int u, v;
} Arete;

typedef struct {
    int nb_sommets;
    int nb_aretes;
    Arete *aretes;
} Graphe;

Graphe* generer_graphe_aretes(int n, float p) {
    Graphe *g = malloc(sizeof(Graphe));
    g->nb_sommets = n;
    g->nb_aretes = 0;

    int max_aretes = n * (n - 1) / 2;
    g->aretes = malloc(sizeof(Arete) * max_aretes);

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (((float)rand() / RAND_MAX) < p) {
                g->aretes[g->nb_aretes].u = i;
                g->aretes[g->nb_aretes].v = j;
                g->nb_aretes++;
            }
        }
    }

    return g;
}


void composantes_connexes_aretes(Graphe *g, Partition *p) {
    for (int i = 0; i < g->nb_aretes; i++) {
        fusion(p, g->aretes[i].u, g->aretes[i].v);
    }
}


void ecrire_graphe_aretes_dot(const char *filename, Graphe *g) {
    FILE *f = fopen(filename, "w");
    fprintf(f, "graph G {\n");
    for (int i = 0; i < g->nb_sommets; i++) {
        fprintf(f, "    %d;\n", i);
    }
    for (int i = 0; i < g->nb_aretes; i++) {
        fprintf(f, "    %d -- %d;\n", g->aretes[i].u, g->aretes[i].v);
    }
    fprintf(f, "}\n");
    fclose(f);
}

int main() {
    srand(time(NULL));
    int n = 12;
    float proba = 0.2;

    Graphe *g = generer_graphe_aretes(n, proba);
    Partition *p = creer_partition(n);

    composantes_connexes_aretes(g, p);
    afficher_composantes(p);
    afficher_nombres_composantes(p);
    ecrire_graphe_aretes_dot("graphe_aretes.dot", g);

    // Nettoyage
    free(g->aretes);
    free(g);
    liberer_partition(p);
    return 0;
}

