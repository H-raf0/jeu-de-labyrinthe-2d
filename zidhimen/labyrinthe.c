#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
int ** matrice_adj(int N){
    // Allouer une matrice N x N
    int** mat = (int**) malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        mat[i] = (int*) calloc(N, sizeof(int)); // initialise à 0
    }
    // Générer les arêtes aléatoirement
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            int edge = rand() % 2; // soit 0, soit 1
            mat[i][j] = edge;
            mat[j][i] = edge; // symétrie
        }
    }
    return mat;
}
void afficher_matrice(int ** mat,int N){
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ",mat[i][j]);

        }
        printf("\n");
    }

}

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

int main(){
    export_graphviz(matrice_adj(10),10,"graphviz.dot");

    afficher_matrice(matrice_adj(10),10);

}