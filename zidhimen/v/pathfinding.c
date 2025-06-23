#include "pathfinding.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

// --- Implémentation d'une File (pour BFS) ---
// Note: une implémentation simple non-circulaire pour la clarté.
typedef struct {
    Point* items;
    int front, rear, capacity;
} Queue;

Queue* create_queue(int capacity) {
    Queue* q = malloc(sizeof(Queue));
    q->capacity = capacity;
    q->front = q->rear = -1;
    q->items = malloc(q->capacity * sizeof(Point));
    return q;
}

int is_queue_empty(Queue* q) { return q->front == -1; }

void enqueue(Queue* q, Point value) {
    if (q->rear == q->capacity - 1) return; // Overflow
    if (q->front == -1) q->front = 0;
    q->items[++(q->rear)] = value;
}

Point dequeue(Queue* q) {
    Point item = q->items[q->front];
    if (q->front == q->rear) q->front = q->rear = -1;
    else q->front++;
    return item;
}

void destroy_queue(Queue* q) {
    free(q->items);
    free(q);
}

// --- Implémentation d'un Tas Binaire (File de Priorité pour Dijkstra/A*) ---
typedef struct {
    Point point;
    int priority;
} HeapNode;

typedef struct {
    HeapNode* nodes;
    int* pos; // Pour mettre à jour la priorité
    int size;
    int capacity;
} PriorityQueue;

// ... [L'implémentation complète du Tas Binaire est longue. Je la simplifie ici]
// Une version simple mais moins efficace: trouver le min à chaque fois.
// Pour un projet réel, un vrai tas binaire est nécessaire.

// --- Fonctions utilitaires ---
int point_to_index(Point p, int width) { return p.y * width + p.x; }
Point index_to_point(int index, int width) { return (Point){index % width, index / width}; }

PathResult* create_path_result(int num_cells) {
    PathResult* res = malloc(sizeof(PathResult));
    res->path = malloc(num_cells * sizeof(Point));
    res->visited = malloc(num_cells * sizeof(Point));
    res->distances = malloc(num_cells * sizeof(int));
    res->path_len = 0;
    res->visited_count = 0;
    for(int i = 0; i < num_cells; ++i) res->distances[i] = -1;
    return res;
}

void destroy_path_result(PathResult* result) {
    if (!result) return;
    free(result->path);
    free(result->visited);
    free(result->distances);
    free(result);
}

void reconstruct_path(PathResult* res, Point* came_from, Point start, Point current, int width) {
    Point p = current;
    while (p.x != start.x || p.y != start.y) {
        res->path[res->path_len++] = p;
        p = came_from[point_to_index(p, width)];
    }
    res->path[res->path_len++] = start;
    // Inverser le chemin
    for(int i = 0; i < res->path_len / 2; ++i) {
        Point temp = res->path[i];
        res->path[i] = res->path[res->path_len - 1 - i];
        res->path[res->path_len - 1 - i] = temp;
    }
}

// --- BFS ---
PathResult* bfs_path(Maze* maze, Point start, Point end) {
    int w = maze->width;
    int h = maze->height;
    int num_cells = w * h;
    
    Queue* q = create_queue(num_cells);
    bool* visited = calloc(num_cells, sizeof(bool));
    Point* came_from = malloc(num_cells * sizeof(Point));
    
    PathResult* result = create_path_result(num_cells);
    result->distances[point_to_index(start, w)] = 0;

    enqueue(q, start);
    visited[point_to_index(start, w)] = true;
    
    while (!is_queue_empty(q)) {
        Point current = dequeue(q);
        result->visited[result->visited_count++] = current;

        if (current.x == end.x && current.y == end.y) {
            reconstruct_path(result, came_from, start, end, w);
            break;
        }

        // Voisins
        int dx[] = {0, 1, 0, -1}; // N, E, S, W
        int dy[] = {-1, 0, 1, 0};

        for (int i = 0; i < 4; ++i) {
            if (!maze->walls[current.y][current.x][i]) {
                Point neighbor = {current.x + dx[i], current.y + dy[i]};
                int neighbor_idx = point_to_index(neighbor, w);
                
                if (!visited[neighbor_idx]) {
                    visited[neighbor_idx] = true;
                    enqueue(q, neighbor);
                    came_from[neighbor_idx] = current;
                    result->distances[neighbor_idx] = result->distances[point_to_index(current, w)] + 1;
                }
            }
        }
    }
    
    destroy_queue(q);
    free(visited);
    free(came_from);
    return result;
}

// --- Heuristiques pour A* ---
int heuristic_manhattan(Point a, Point b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}
double heuristic_euclidean(Point a, Point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
int heuristic_chebyshev(Point a, Point b) {
    return fmax(abs(a.x - b.x), abs(a.y - b.y));
}

// --- A* ---
// Dijkstra est un cas particulier de A* avec une heuristique nulle.
PathResult* a_star_path(Maze* maze, Point start, Point end, int heuristic_choice) {
    int w = maze->width;
    int h = maze->height;
    int num_cells = w * h;

    // Structure de données simple pour la file de priorité (pas un vrai tas)
    Point* open_set = malloc(num_cells * sizeof(Point));
    int open_set_count = 0;

    Point* came_from = malloc(num_cells * sizeof(Point));
    int* g_score = malloc(num_cells * sizeof(int));
    int* f_score = malloc(num_cells * sizeof(int));

    for (int i = 0; i < num_cells; ++i) {
        g_score[i] = INT_MAX;
        f_score[i] = INT_MAX;
    }

    g_score[point_to_index(start, w)] = 0;
    
    int h_score;
    if (heuristic_choice == MANHATTAN) h_score = heuristic_manhattan(start, end);
    else if (heuristic_choice == EUCLIDEAN) h_score = (int)heuristic_euclidean(start, end);
    else if (heuristic_choice == CHEBYSHEV) h_score = heuristic_chebyshev(start, end);
    else h_score = 0; // Dijkstra

    f_score[point_to_index(start, w)] = h_score;

    open_set[open_set_count++] = start;
    
    PathResult* result = create_path_result(num_cells);

    while (open_set_count > 0) {
        // Trouver le noeud dans open_set avec le f_score le plus bas
        int current_idx_in_set = 0;
        for (int i = 1; i < open_set_count; i++) {
            if (f_score[point_to_index(open_set[i], w)] < f_score[point_to_index(open_set[current_idx_in_set], w)]) {
                current_idx_in_set = i;
            }
        }
        Point current = open_set[current_idx_in_set];

        // Retirer de l'open set
        open_set[current_idx_in_set] = open_set[--open_set_count];
        
        result->visited[result->visited_count++] = current;

        if (current.x == end.x && current.y == end.y) {
            reconstruct_path(result, came_from, start, end, w);
            // Copier les distances (g_score)
            for(int i=0; i<num_cells; ++i) if(g_score[i] != INT_MAX) result->distances[i] = g_score[i];
            break;
        }

        int dx[] = {0, 1, 0, -1};
        int dy[] = {-1, 0, 1, 0};

        for (int i = 0; i < 4; i++) {
            if (!maze->walls[current.y][current.x][i]) {
                Point neighbor = {current.x + dx[i], current.y + dy[i]};
                int neighbor_idx = point_to_index(neighbor, w);
                
                int tentative_g_score = g_score[point_to_index(current, w)] + 1;

                if (tentative_g_score < g_score[neighbor_idx]) {
                    came_from[neighbor_idx] = current;
                    g_score[neighbor_idx] = tentative_g_score;
                    
                    if (heuristic_choice == MANHATTAN) h_score = heuristic_manhattan(neighbor, end);
                    else if (heuristic_choice == EUCLIDEAN) h_score = (int)heuristic_euclidean(neighbor, end);
                    else if (heuristic_choice == CHEBYSHEV) h_score = heuristic_chebyshev(neighbor, end);
                    else h_score = 0; // Dijkstra

                    f_score[neighbor_idx] = g_score[neighbor_idx] + h_score;
                    
                    // Ajouter à l'open set s'il n'y est pas déjà
                    bool in_open_set = false;
                    for (int j = 0; j < open_set_count; j++) {
                        if (open_set[j].x == neighbor.x && open_set[j].y == neighbor.y) {
                            in_open_set = true;
                            break;
                        }
                    }
                    if (!in_open_set) {
                        open_set[open_set_count++] = neighbor;
                    }
                }
            }
        }
    }
    
    free(open_set);
    free(came_from);
    free(g_score);
    free(f_score);
    return result;
}

PathResult* dijkstra_path(Maze* maze, Point start, Point end) {
    return a_star_path(maze, start, end, 0); // Heuristique nulle
}