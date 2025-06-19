#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int u, v;
} Arete;

typedef struct {
    int n;          // Nombre de sommets
    int m;          // Nombre d’arêtes
    Arete* aretes;  // Tableau des arêtes
} GrapheAretes;

// Génère un graphe aléatoire représenté par une liste d’arêtes
GrapheAretes generer_graphe_aretes(int N) {
    Arete* temp = (Arete*) malloc(N * N * sizeof(Arete));
    int count = 0;

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if (rand() % 2) {
                temp[count].u = i;
                temp[count].v = j;
                count++;
            }
        }
    }

    GrapheAretes g;
    g.n = N;
    g.m = count;
    g.aretes = (Arete*) malloc(count * sizeof(Arete));
    for (int i = 0; i < count; i++) {
        g.aretes[i] = temp[i];
    }
    free(temp);
    return g;
}

// Exporte vers un fichier Graphviz
void export_graphe_aretes(GrapheAretes g, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Erreur d'ouverture");
        return;
    }

    fprintf(f, "graph G {\n");
    for (int i = 0; i < g.m; i++) {
        fprintf(f, "    %d -- %d;\n", g.aretes[i].u, g.aretes[i].v);
    }
    fprintf(f, "}\n");
    fclose(f);
}

int main() {
    srand(time(NULL));
    int N = 10;

    printf(">>> Génération avec liste d’arêtes...\n");
    GrapheAretes g = generer_graphe_aretes(N);
    export_graphe_aretes(g, "graphe_aretes.dot");

    free(g.aretes);
    return 0;
}
