#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// Génère une matrice d’adjacence aléatoire pour un graphe non orienté
int** matrice_adj(int N) {
    int** mat = (int**) malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        mat[i] = (int*) malloc(N* sizeof(int));
    }

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            int edge = rand() % 2;
            mat[i][j] = edge;
            mat[j][i] = edge;
        }
    }
    return mat;
}

// Affiche la matrice
void afficher_matrice(int** mat, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", mat[i][j]);
        }
        printf("\n");
    }
}

// Export Graphviz
void export_graphviz(int** mat, int N, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Erreur d'ouverture de fichier");
        return;
    }

    fprintf(f, "graph G {\n");
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if (mat[i][j]) {
                fprintf(f, "    %d -- %d;\n", i, j);
            }
        }
    }
    fprintf(f, "}\n");
    fclose(f);
}

// DFS pour les composantes connexes
void dfs(int node, int** mat, int N, bool* visited, int* composante, int comp_id) {
    visited[node] = true;
    composante[node] = comp_id;
    for (int i = 0; i < N; i++) {
        if (mat[node][i] && !visited[i]) {
            dfs(i, mat, N, visited, composante, comp_id);
        }
    }
}

int* trouver_composantes(int** mat, int N, int* nb_composantes) {
    bool* visited = (bool*) malloc(N*sizeof(bool));
    int* composante = (int*) malloc(N * sizeof(int));
    *nb_composantes = 0;

    for (int i = 0; i < N; i++) {
        if (!visited[i]) {
            dfs(i, mat, N, visited, composante, *nb_composantes);
            (*nb_composantes)++;
        }
    }

    free(visited);
    return composante;
}

// Exporte les sous-graphes de chaque composante
void export_sous_graphes(int** mat, int N, int* composante, int nb_composantes) {
    for (int c = 0; c < nb_composantes; c++) {
        char filename[100];
        sprintf(filename, "composante_%d.dot", c);
        FILE* f = fopen(filename, "w");
        if (!f) {
            perror("Erreur d'ouverture de fichier");
            continue;
        }

        fprintf(f, "graph G {\n");
        for (int i = 0; i < N; i++) {
            if (composante[i] != c) continue;
            for (int j = i + 1; j < N; j++) {
                if (mat[i][j] && composante[j] == c) {
                    fprintf(f, "    %d -- %d;\n", i, j);
                }
            }
        }
        fprintf(f, "}\n");
        fclose(f);
    }
}

int main() {
    srand(time(NULL));
    int N = 10;

    printf(">>> Génération avec matrice d'adjacence...\n");
    int** mat = matrice_adj(N);
    afficher_matrice(mat, N);
    export_graphviz(mat, N, "graphe_matrice.dot");

    int nb_composantes;
    int* composantes = trouver_composantes(mat, N, &nb_composantes);
    printf("\nComposantes connexes :\n");
    for (int i = 0; i < N; i++) {
        printf("Sommet %d -> composante %d\n", i, composantes[i]);
    }
    export_sous_graphes(mat, N, composantes, nb_composantes);

    for (int i = 0; i < N; i++) free(mat[i]);
    free(mat);
    free(composantes);

    return 0;
}
