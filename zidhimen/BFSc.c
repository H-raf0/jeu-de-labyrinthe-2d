#include <stdio.h>
#include <stdlib.h>

#define MAXELE 100
#define MAX_NODES 100

// ---------- Structure de la FILE (circulaire) ----------
typedef struct {
    int front;
    int rear;
    int elements[MAXELE];
} Queue;

void initQueue(Queue *q) {
    q->front = 0;
    q->rear = 0;
}

int isEmpty(Queue *q) {
    return q->front == q->rear;
}

int isFull(Queue *q) {
    return (q->rear + 1) % MAXELE == q->front;
}

void enqueue(Queue *q, int value) {
    if (isFull(q)) {
        printf("Erreur : la file est pleine !\n");
        exit(EXIT_FAILURE);
    }
    q->elements[q->rear] = value;
    q->rear = (q->rear + 1) % MAXELE;
}

int dequeue(Queue *q) {
    if (isEmpty(q)) {
        printf("Erreur : la file est vide !\n");
        exit(EXIT_FAILURE);
    }
    int value = q->elements[q->front];
    q->front = (q->front + 1) % MAXELE;
    return value;
}

// ---------- BFS avec file circulaire ----------
void BFS(int adj[MAX_NODES][MAX_NODES], int numNodes, int origin) {
    Queue q;
    initQueue(&q);

    int distance[MAX_NODES];
    for (int i = 0; i < numNodes; i++) {
        distance[i] = -1;  // -1 signifie non visité
    }

    enqueue(&q, origin);
    distance[origin] = 0;

    while (!isEmpty(&q)) {
        int node = dequeue(&q);

        for (int neighbor = 0; neighbor < numNodes; neighbor++) {
            if (adj[node][neighbor] == 1 && distance[neighbor] == -1) {
                enqueue(&q, neighbor);
                distance[neighbor] = distance[node] + 1;
            }
        }
    }

    // Affichage des distances
    printf("Distances depuis le noeud %d :\n", origin);
    for (int i = 0; i < numNodes; i++) {
        if (distance[i] == -1)
            printf("Noeud %d : inatteignable\n", i);
        else
            printf("Noeud %d : %d\n", i, distance[i]);
    }
}

// ---------- Exemple d'utilisation ----------
int main() {
    int numNodes = 6;
    int adj[MAX_NODES][MAX_NODES] = {
        // 0 1 2 3 4 5
        {0,1,1,0,0,0}, // 0
        {1,0,1,1,0,0}, // 1
        {1,1,0,0,1,0}, // 2
        {0,1,0,0,1,1}, // 3
        {0,0,1,1,0,1}, // 4
        {0,0,0,1,1,0}  // 5
    };

    int origin = 0;
    BFS(adj, numNodes, origin);

    return 0;
}
