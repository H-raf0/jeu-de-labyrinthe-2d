#include <stdio.h>
#include <stdlib.h>

#define MAX_EDGES 100
#define MAX_VERTICES 100

// Structure d'une arête
typedef struct {
    int src, dest, weight;
} Edge;

// Structure du graphe
typedef struct {
    int V, E;
    Edge edges[MAX_EDGES];
} Graph;

// Disjoint set
int parent[MAX_VERTICES];

// Trouver la racine de l'ensemble
int find(int i) {
    if (parent[i] == i)
        return i;
    return parent[i] = find(parent[i]); // compression de chemin
}

// Union de deux ensembles
void union_set(int x, int y) {
    int xroot = find(x);
    int yroot = find(y);
    parent[xroot] = yroot;
}

// Comparateur pour qsort (tri des poids)
int compare_edges(const void* a, const void* b) {
    Edge* e1 = (Edge*)a;
    Edge* e2 = (Edge*)b;
    return e1->weight - e2->weight;
}

// Fonction principale : Kruskal
void kruskal(Graph* graph) {
    Edge result[MAX_EDGES];
    int e = 0; // compteur pour les arêtes du résultat

    // 1. Trier les arêtes par poids
    qsort(graph->edges, graph->E, sizeof(graph->edges[0]), compare_edges);

    // 2. Initialiser les ensembles (disjoint sets)
    for (int i = 0; i < graph->V; i++)
        parent[i] = i;

    // 3. Parcourir les arêtes triées
    for (int i = 0; i < graph->E && e < graph->V - 1; i++) {
        Edge next = graph->edges[i];

        int x = find(next.src);
        int y = find(next.dest);

        // Si ajouter cette arête ne forme pas de cycle
        if (x != y) {
            result[e++] = next;
            union_set(x, y);
        }
    }

    // Affichage du résultat
    printf("Arêtes de l'ACPM (Kruskal) :\n");
    for (int i = 0; i < e; i++)
        printf("%d -- %d == %d\n", result[i].src, result[i].dest, result[i].weight);
}

int main() {
    Graph graph;
    graph.V = 4; // sommets
    graph.E = 5; // arêtes

    // Définir les arêtes : src, dest, poids
    graph.edges[0] = (Edge){0, 1, 10};
    graph.edges[1] = (Edge){0, 2, 6};
    graph.edges[2] = (Edge){0, 3, 5};
    graph.edges[3] = (Edge){1, 3, 15};
    graph.edges[4] = (Edge){2, 3, 4};

    kruskal(&graph);

    return 0;
}
