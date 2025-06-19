#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// ========================= STRUCTURE PARTITION =========================

typedef struct {
    int  size;
    int *parent;
} Partition;

// Création d'une partition initiale (parent[i] = i)
Partition* creer_partition(int n) {
    Partition *p = malloc(sizeof *p);
    if (!p) { perror("malloc Partition"); exit(EXIT_FAILURE); }
    p->size   = n;
    p->parent = malloc(n * sizeof *p->parent);
    if (!p->parent) { perror("malloc parent"); exit(EXIT_FAILURE); }
    for (int i = 0; i < n; i++) {
        p->parent[i] = i;
    }
    return p;
}

// Find avec compression de chemin
int trouver(Partition *p, int x) {
    if (p->parent[x] != x) {
        p->parent[x] = trouver(p, p->parent[x]);
    }
    return p->parent[x];
}

// Union sans rank (on attache ry sous rx)
void fusion(Partition *p, int x, int y) {
    int rx = trouver(p, x);
    int ry = trouver(p, y);
    if (rx != ry) {
        p->parent[ry] = rx;
    }
}

void liberer_partition(Partition *p) {
    free(p->parent);
    free(p);
}

// ========================= MATRICE D'ADJACENCE =========================

int **matrice_adj(int N, float prob) {
    int **M = calloc(N, sizeof(int *));
    if (!M) { perror("calloc M"); exit(EXIT_FAILURE); }
    for (int i = 0; i < N; i++) {
        M[i] = calloc(N, sizeof(int));
        if (!M[i]) { perror("calloc M[i]"); exit(EXIT_FAILURE); }
    }

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if ((rand() / (float)RAND_MAX) < prob) {
                M[i][j] = M[j][i] = 1;
            }
        }
    }
    return M;
}

void liberer_matrice(int **M, int N) {
    for (int i = 0; i < N; i++) free(M[i]);
    free(M);
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

// ========================= COMPOSANTES CONNEXES =========================

void composantes_connexes_matrice(int **M, int N, Partition *p) {
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if (M[i][j]) fusion(p, i, j);
        }
    }
}

void afficher_composantes(Partition *p) {
    printf("Noeud -> Racine\n");
    for (int i = 0; i < p->size; i++) {
        printf("  %2d  ->   %2d\n", i, trouver(p, i));
    }
}

void afficher_nombres_composantes(Partition *p) {
    int *counts = calloc(p->size, sizeof(int));
    if (!counts) { perror("calloc counts"); exit(EXIT_FAILURE); }
    for (int i = 0; i < p->size; i++) {
        counts[trouver(p, i)]++;
    }
    printf("\nTaille des composantes :\n");
    for (int i = 0; i < p->size; i++) {
        if (counts[i] > 0) {
            printf("  Composante %2d : %2d noeud(s)\n", i, counts[i]);
        }
    }
    free(counts);
}

// ========================= EXPORT DOT =========================

void ecrire_graphe_dot(const char *fichier, int **M, int N) {
    FILE *f = fopen(fichier, "w");
    if (!f) { perror("fopen"); return; }
    fprintf(f, "graph G {\n");
    for (int i = 0; i < N; i++) {
        fprintf(f, "  %d;\n", i);
        for (int j = i + 1; j < N; j++) {
            if (M[i][j]) fprintf(f, "  %d -- %d;\n", i, j);
        }
    }
    fprintf(f, "}\n");
    fclose(f);
}

void ecrire_composantes_dot(const char *fichier, int **M, int N, Partition *p) {
    FILE *f = fopen(fichier, "w");
    if (!f) { perror("fopen"); return; }
    fprintf(f, "graph G {\n");

    // regrouper noeuds par racine
    int *root = malloc(N*sizeof(int));
    int *size = calloc(N,sizeof(int));
    for (int i = 0; i < N; i++) {
        root[i] = trouver(p, i);
        size[root[i]]++;
    }

    int *index = calloc(N,sizeof(int));
    int **lists = calloc(N, sizeof(int*));
    for (int r = 0; r < N; r++) {
        if (size[r]>0) lists[r] = malloc(size[r]*sizeof(int));
    }
    for (int i = 0; i < N; i++) {
        int r = root[i];
        lists[r][ index[r]++ ] = i;
    }

    for (int r = 0; r < N; r++) {
        if (size[r]==0) continue;
        fprintf(f, "  subgraph cluster_%d {\n", r);
        fprintf(f, "    label=\"Comp. %d\";\n", r);
        for (int k = 0; k < size[r]; k++) {
            int u = lists[r][k];
            fprintf(f, "    %d;\n", u);
        }
        for (int a = 0; a < size[r]; a++) {
            for (int b = a+1; b < size[r]; b++) {
                int u = lists[r][a], v = lists[r][b];
                if (M[u][v]) fprintf(f, "    %d -- %d;\n", u, v);
            }
        }
        fprintf(f, "  }\n");
    }

    fprintf(f, "}\n");
    fclose(f);

    // cleanup
    for (int r = 0; r < N; r++) {
        if (size[r]>0) free(lists[r]);
    }
    free(lists);
    free(root);
    free(size);
    free(index);
}

// ========================= GRAPHE PAR LISTE D'ARÊTES =========================

typedef struct {
    int u, v;
    float w;      // poids de l'arête
} Arete;


typedef struct {
    int nb_sommets;
    int nb_aretes;
    // Pas de pointeur ici : on réserve directement les éléments après la structure
    Arete aretes[];
} Graphe;

// --- Génération du graphe aléatoire sous forme de tableau d'arêtes ---

Graphe* generer_graphe_aretes(int N, float prob) {
    int maxA = N*(N-1)/2;
    // On alloue la structure + maxA arêtes
    Graphe *g = malloc(sizeof *g + maxA * sizeof(Arete));
    if (!g) { perror("malloc Graphe"); exit(EXIT_FAILURE); }

    g->nb_sommets = N;
    g->nb_aretes  = 0;

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if ((rand()/(float)RAND_MAX) < prob) {  // (rand()/(float)RAND_MAX) entre 0 et 1
                float poids = rand()/(float)RAND_MAX;
                g->aretes[g->nb_aretes++] = (Arete){ i, j, poids };
            }
        }
    }
    return g;
}


void composantes_connexes_aretes(Graphe *g, Partition *p) {
    for (int i = 0; i < g->nb_aretes; i++)
        fusion(p, g->aretes[i].u, g->aretes[i].v);
}

void ecrire_graphe_aretes_dot(const char *fichier, Graphe *g) {
    FILE *f = fopen(fichier, "w");
    if (!f) { perror("fopen"); return; }

    fprintf(f, "graph G {\n");
    for (int i = 0; i < g->nb_sommets; i++)
        fprintf(f, "  %d;\n", i);
    for (int i = 0; i < g->nb_aretes; i++)
        fprintf(f, "  %d -- %d;\n",
                g->aretes[i].u,
                g->aretes[i].v);
    fprintf(f, "}\n");

    fclose(f);
}


// ======================== Kruskal ==============================


int cmp_arete(const void *a, const void *b) {
    float wa = ((Arete*)a)->w;
    float wb = ((Arete*)b)->w;
    if (wa < wb) return -1;
    else if (wa > wb) return +1;
    return 0;
}






// ========================= MAIN =========================

int main(void) {
    srand(time(NULL));
    const int N   = 12;
    const float P = 0.2f;

    printf("=== Graphe par matrice ===\n");
    int **mat = matrice_adj(N, P);
    afficher_matrice(mat, N);

    // Création & traitement par matrice
    Partition *p1 = creer_partition(N);
    composantes_connexes_matrice(mat, N, p1);
    afficher_composantes(p1);
    afficher_nombres_composantes(p1);
    ecrire_graphe_dot("graphe.dot", mat, N);
    ecrire_composantes_dot("composantes.dot", mat, N, p1);

    // Nettoyage de la matrice et de la partition
    liberer_matrice(mat, N);
    liberer_partition(p1);

    
    printf("\n=== Graphe par liste d'arêtes ===\n");
    // Générer le graphe
    Graphe *g = generer_graphe_aretes(N, P);

    // Partition et calcul
    Partition *p = creer_partition(N);
    composantes_connexes_aretes(g, p);

    // Affichage
    printf("=== Graphe par tableau d'arêtes ===\n");
    afficher_composantes(p);
    afficher_nombres_composantes(p);

    // Export Graphviz
    ecrire_graphe_aretes_dot("graphe_aretes.dot", g);
    printf("\nFichier DOT créé : graphe_aretes.dot\n");

    // Libération mémoire
    free(g);            // g embarque aussi aretes[]
    liberer_partition(p);

    return 0;
}

