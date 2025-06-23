#include <stdio.h>

#include <stdlib.h>

#include <time.h>

#include <string.h>

#include <math.h>

#include <limits.h>

#include <stdbool.h>

#include <SDL2/SDL.h>

#include <SDL2/SDL_image.h>

// ... (TOUT LE CODE PRÉCÉDENT DES SECTIONS 1 ET 2 RESTE INCHANGÉ) ...
// (DSU, Edge, TILESET_MAPPING, generate_maze, PathResult, find_path_A_star, etc.)
// Je vais le ré-inclure pour que vous puissiez copier-coller le fichier entier.

// =========================================================================================
// SECTION 1 : CODE DU COLLÈGUE (GÉNÉRATION DU LABYRINTHE)
// =========================================================================================

#define MAZE_WIDTH 40
#define MAZE_HEIGHT 30
#define TILE_SIZE 16

typedef struct {
    int * parent;
    int * rank;
}
DSU;
typedef struct {
    int u, v;
}
Edge;

SDL_Rect TILESET_MAPPING[16] = {
    {
        1 * TILE_SIZE, 1 * TILE_SIZE, TILE_SIZE, TILE_SIZE
    },
    {
        1 * TILE_SIZE,
        0 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        2 * TILE_SIZE,
        1 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        2 * TILE_SIZE,
        0 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        1 * TILE_SIZE,
        2 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        7 * TILE_SIZE,
        4 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        2 * TILE_SIZE,
        2 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        8 * TILE_SIZE,
        4 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        0 * TILE_SIZE,
        1 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        0 * TILE_SIZE,
        0 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        8 * TILE_SIZE,
        2 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        8 * TILE_SIZE,
        1 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        0 * TILE_SIZE,
        2 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        6 * TILE_SIZE,
        4 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        8 * TILE_SIZE,
        3 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    },
    {
        7 * TILE_SIZE,
        3 * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    }
};

void dsu_init(DSU * dsu, int n) {
    dsu -> parent = malloc(sizeof(int) * n);
    dsu -> rank = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        dsu -> parent[i] = i;
        dsu -> rank[i] = 0;
    }
}
void dsu_free(DSU * dsu) {
    free(dsu -> parent);
    free(dsu -> rank);
}
int dsu_find(DSU * dsu, int i) {
    if (dsu -> parent[i] != i) dsu -> parent[i] = dsu_find(dsu, dsu -> parent[i]);
    return dsu -> parent[i];
}
void dsu_union(DSU * dsu, int i, int j) {
    int root_i = dsu_find(dsu, i);
    int root_j = dsu_find(dsu, j);
    if (root_i != root_j) {
        if (dsu -> rank[root_i] < dsu -> rank[root_j]) dsu -> parent[root_i] = root_j;
        else if (dsu -> rank[root_i] > dsu -> rank[root_j]) dsu -> parent[root_j] = root_i;
        else {
            dsu -> parent[root_j] = root_i;
            dsu -> rank[root_i]++;
        }
    }
}
void shuffle_edges(Edge edges[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Edge temp = edges[i];
        edges[i] = edges[j];
        edges[j] = temp;
    }
}
int * generate_maze(int rows, int cols, float p_cycle) {
    int num_cells = rows * cols;
    int max_edges = 2 * rows * cols - rows - cols;
    Edge * all_edges = malloc(sizeof(Edge) * max_edges);
    int edge_count = 0;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int i = y * cols + x;
            if (y + 1 < rows) all_edges[edge_count++] = (Edge) {
                i,
                (y + 1) * cols + x
            };
            if (x + 1 < cols) all_edges[edge_count++] = (Edge) {
                i,
                y * cols + (x + 1)
            };
        }
    }
    shuffle_edges(all_edges, edge_count);
    int * walls = malloc(sizeof(int) * num_cells);
    for (int i = 0; i < num_cells; i++) walls[i] = 15;
    DSU dsu;
    dsu_init( & dsu, num_cells);
    for (int i = 0; i < edge_count; i++) {
        Edge e = all_edges[i];
        if (dsu_find( & dsu, e.u) != dsu_find( & dsu, e.v)) {
            dsu_union( & dsu, e.u, e.v);
            int dx = (e.v % cols) - (e.u % cols);
            if (dx == 1) {
                walls[e.u] &= ~2;
                walls[e.v] &= ~8;
            } else {
                walls[e.u] &= ~4;
                walls[e.v] &= ~1;
            }
        } else {
            if ((float) rand() / RAND_MAX < p_cycle) {
                int dx = (e.v % cols) - (e.u % cols);
                if (dx == 1) {
                    walls[e.u] &= ~2;
                    walls[e.v] &= ~8;
                } else {
                    walls[e.u] &= ~4;
                    walls[e.v] &= ~1;
                }
            }
        }
    }
    dsu_free( & dsu);
    free(all_edges);
    return walls;
}

// =========================================================================================
// SECTION 2 : MON CODE (PATHFINDING ET VISUALISATION), ADAPTÉ POUR ÊTRE COMPATIBLE
// =========================================================================================

typedef struct {
    int x;
    int y;
}
Point;
typedef struct {
    Point * path;
    int path_len;
    Point * visited;
    int visited_count;
    int * distances;
}
PathResult;

PathResult * create_path_result(int num_cells) {
    PathResult * res = malloc(sizeof(PathResult));
    res -> path = malloc(num_cells * sizeof(Point));
    res -> visited = malloc(num_cells * sizeof(Point));
    res -> distances = malloc(num_cells * sizeof(int));
    res -> path_len = 0;
    res -> visited_count = 0;
    for (int i = 0; i < num_cells; ++i) res -> distances[i] = -1;
    return res;
}
void destroy_path_result(PathResult * result) {
    if (!result) return;
    free(result -> path);
    free(result -> visited);
    free(result -> distances);
    free(result);
}
void reconstruct_path(PathResult * res, Point * came_from, Point start, Point current, int width) {
    Point p = current;
    while (p.x != start.x || p.y != start.y) {
        res -> path[res -> path_len++] = p;
        p = came_from[p.y * width + p.x];
    }
    res -> path[res -> path_len++] = start;
    for (int i = 0; i < res -> path_len / 2; ++i) {
        Point temp = res -> path[i];
        res -> path[i] = res -> path[res -> path_len - 1 - i];
        res -> path[res -> path_len - 1 - i] = temp;
    }
}
PathResult * find_path_A_star(int * walls, int w, int h, Point start, Point end, bool use_heuristic) {
    int num_cells = w * h;
    Point * open_set = malloc(num_cells * sizeof(Point));
    int open_set_count = 0;
    Point * came_from = malloc(num_cells * sizeof(Point));
    int * g_score = malloc(num_cells * sizeof(int));
    int * f_score = malloc(num_cells * sizeof(int));
    for (int i = 0; i < num_cells; ++i) g_score[i] = INT_MAX;
    g_score[start.y * w + start.x] = 0;
    int h_score = use_heuristic ? abs(start.x - end.x) + abs(start.y - end.y) : 0;
    f_score[start.y * w + start.x] = h_score;
    open_set[open_set_count++] = start;
    PathResult * result = create_path_result(num_cells);
    while (open_set_count > 0) {
        int current_idx_in_set = 0;
        for (int i = 1; i < open_set_count; i++) {
            if (f_score[open_set[i].y * w + open_set[i].x] < f_score[open_set[current_idx_in_set].y * w + open_set[current_idx_in_set].x]) {
                current_idx_in_set = i;
            }
        }
        Point current = open_set[current_idx_in_set];
        open_set[current_idx_in_set] = open_set[--open_set_count];
        result -> visited[result -> visited_count++] = current;
        if (current.x == end.x && current.y == end.y) {
            reconstruct_path(result, came_from, start, end, w);
            for (int i = 0; i < num_cells; ++i)
                if (g_score[i] != INT_MAX) result -> distances[i] = g_score[i];
            break;
        }
        int dx[] = {
            0,
            0,
            1,
            -1
        };
        int dy[] = {
            1,
            -1,
            0,
            0
        };
        int wall_masks[] = {
            4,
            1,
            2,
            8
        };
        for (int i = 0; i < 4; i++) {
            if (!(walls[current.y * w + current.x] & wall_masks[i])) {
                Point neighbor = {
                    current.x + dx[i],
                    current.y + dy[i]
                };
                int neighbor_idx = neighbor.y * w + neighbor.x;
                int tentative_g_score = g_score[current.y * w + current.x] + 1;
                if (tentative_g_score < g_score[neighbor_idx]) {
                    came_from[neighbor_idx] = current;
                    g_score[neighbor_idx] = tentative_g_score;
                    h_score = use_heuristic ? abs(neighbor.x - end.x) + abs(neighbor.y - end.y) : 0;
                    f_score[neighbor_idx] = g_score[neighbor_idx] + h_score;
                    bool in_open_set = false;
                    for (int j = 0; j < open_set_count; j++) {
                        if (open_set[j].x == neighbor.x && open_set[j].y == neighbor.y) {
                            in_open_set = true;
                            break;
                        }
                    }
                    if (!in_open_set) open_set[open_set_count++] = neighbor;
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
PathResult * find_path_bfs(int * walls, int w, int h, Point start, Point end) {
    int num_cells = w * h;
    Point * q = malloc(num_cells * sizeof(Point));
    int head = 0, tail = 0;
    bool * visited = calloc(num_cells, sizeof(bool));
    PathResult * result = create_path_result(num_cells);
    result -> distances[start.y * w + start.x] = 0;
    q[tail++] = start;
    visited[start.y * w + start.x] = true;
    while (head != tail) {
        Point current = q[head++];
        int dx[] = {
            0,
            0,
            1,
            -1
        };
        int dy[] = {
            1,
            -1,
            0,
            0
        };
        int wall_masks[] = {
            4,
            1,
            2,
            8
        };
        for (int i = 0; i < 4; ++i) {
            if (!(walls[current.y * w + current.x] & wall_masks[i])) {
                Point neighbor = {
                    current.x + dx[i],
                    current.y + dy[i]
                };
                int neighbor_idx = neighbor.y * w + neighbor.x;
                if (!visited[neighbor_idx]) {
                    visited[neighbor_idx] = true;
                    q[tail++] = neighbor;
                    result -> distances[neighbor_idx] = result -> distances[current.y * w + current.x] + 1;
                }
            }
        }
    }
    free(q);
    free(visited);
    return result;
}

// =========================================================================================
// SECTION 3 : FONCTIONS D'AFFICHAGE ET MAIN CORRIGÉS
// =========================================================================================

void draw_floor_tiles(SDL_Renderer * renderer, SDL_Texture * tileset, int rows, int cols) {
    // On utilise la tuile '0' (sans mur) comme sol de base
    SDL_Rect src_floor = TILESET_MAPPING[0];
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            SDL_Rect dst = {
                x * TILE_SIZE,
                y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };
            SDL_RenderCopy(renderer, tileset, & src_floor, & dst);
        }
    }
}

// *** NOUVELLE FONCTION ***
// Dessine les murs en utilisant des lignes SDL, pour qu'ils soient nets et par-dessus tout.
void draw_walls_from_bitmask(SDL_Renderer * renderer, int * walls, int rows, int cols) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Murs noirs
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int px = x * TILE_SIZE;
            int py = y * TILE_SIZE;
            int val = walls[y * cols + x];
            if (val & 1) SDL_RenderDrawLine(renderer, px, py, px + TILE_SIZE, py); // Haut
            if (val & 2) SDL_RenderDrawLine(renderer, px + TILE_SIZE, py, px + TILE_SIZE, py + TILE_SIZE); // Droite
            if (val & 4) SDL_RenderDrawLine(renderer, px, py + TILE_SIZE, px + TILE_SIZE, py + TILE_SIZE); // Bas
            if (val & 8) SDL_RenderDrawLine(renderer, px, py, px, py + TILE_SIZE); // Gauche
        }
    }
}

void draw_distance_gradient(SDL_Renderer * renderer, PathResult * result, int w, int h) {
    if (!result || !result -> distances) return;
    int max_dist = 0;
    for (int i = 0; i < w * h; ++i)
        if (result -> distances[i] > max_dist) max_dist = result -> distances[i];
    if (max_dist == 0) return;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int dist = result -> distances[y * w + x];
            if (dist != -1) {
                Uint8 r = (Uint8)(255 * dist / max_dist);
                Uint8 b = (Uint8)(255 * (max_dist - dist) / max_dist);
                SDL_SetRenderDrawColor(renderer, r, 0, b, 100);
                SDL_Rect cell_rect = {
                    x * TILE_SIZE,
                    y * TILE_SIZE,
                    TILE_SIZE,
                    TILE_SIZE
                };
                SDL_RenderFillRect(renderer, & cell_rect);
            }
        }
    }
}
void draw_path_info(SDL_Renderer * renderer, PathResult * result, Point start, Point end) {
    if (!result) return;
    SDL_Rect cell_rect = {
        0,
        0,
        TILE_SIZE,
        TILE_SIZE
    };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 80);
    for (int i = 0; i < result -> visited_count; i++) {
        cell_rect.x = result -> visited[i].x * TILE_SIZE;
        cell_rect.y = result -> visited[i].y * TILE_SIZE;
        SDL_RenderFillRect(renderer, & cell_rect);
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 180);
    for (int i = 0; i < result -> path_len; i++) {
        cell_rect.x = result -> path[i].x * TILE_SIZE;
        cell_rect.y = result -> path[i].y * TILE_SIZE;
        SDL_RenderFillRect(renderer, & cell_rect);
    }
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    cell_rect.x = start.x * TILE_SIZE;
    cell_rect.y = start.y * TILE_SIZE;
    SDL_RenderFillRect(renderer, & cell_rect);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    cell_rect.x = end.x * TILE_SIZE;
    cell_rect.y = end.y * TILE_SIZE;
    SDL_RenderFillRect(renderer, & cell_rect);
}

// *** FONCTION MAIN MODIFIÉE ***
int main(int argc, char * argv[]) {
    srand(time(NULL));

    printf("Génération du labyrinthe...\n");
    int * maze_walls = generate_maze(MAZE_HEIGHT, MAZE_WIDTH, 0.05 f);

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    SDL_Window * window = SDL_CreateWindow("Projet Labyrinthe Corrigé", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAZE_WIDTH * TILE_SIZE, MAZE_HEIGHT * TILE_SIZE, SDL_WINDOW_SHOWN);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface * tileset_surface = IMG_Load("tileset.png");
    SDL_Texture * tileset = SDL_CreateTextureFromSurface(renderer, tileset_surface);
    SDL_FreeSurface(tileset_surface);

    Point start = {
        0,
        0
    };
    Point end = {
        MAZE_WIDTH - 1,
        MAZE_HEIGHT - 1
    };

    PathResult * bfs_res = find_path_bfs(maze_walls, MAZE_WIDTH, MAZE_HEIGHT, end, start);
    PathResult * anim_path_res = find_path_A_star(maze_walls, MAZE_WIDTH, MAZE_HEIGHT, start, end, true);

    SDL_Event e;
    bool quit = false;
    int frame_count = 0;

    while (!quit) {
        while (SDL_PollEvent( & e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
        }

        // --- NOUVEL ORDRE DE RENDU ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 1. Dessiner le sol de base avec une tuile de sol.
        draw_floor_tiles(renderer, tileset, MAZE_HEIGHT, MAZE_WIDTH);

        // 2. Superposer le dégradé de distance.
        draw_distance_gradient(renderer, bfs_res, MAZE_WIDTH, MAZE_HEIGHT);

        // 3. Superposer les infos du chemin (visités, chemin, départ/arrivée).
        draw_path_info(renderer, anim_path_res, start, end);

        // 4. Animer le personnage.
        if (anim_path_res && anim_path_res -> path_len > 0) {
            // ... (logique d'animation inchangée)
            int anim_pos = (frame_count / 15) % anim_path_res -> path_len;
            Point char_pos = anim_path_res -> path[anim_pos];
            SDL_Rect char_rect = {
                char_pos.x * TILE_SIZE + TILE_SIZE / 4,
                char_pos.y * TILE_SIZE + TILE_SIZE / 4,
                TILE_SIZE / 2,
                TILE_SIZE / 2
            };
            SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255);
            SDL_RenderFillRect(renderer, & char_rect);

            if (anim_pos == anim_path_res -> path_len - 1 && anim_path_res -> path_len > 1) {
                SDL_Delay(500);
                start = end;
                end = (Point) {
                    rand() % MAZE_WIDTH, rand() % MAZE_HEIGHT
                };
                destroy_path_result(anim_path_res);
                anim_path_res = find_path_A_star(maze_walls, MAZE_WIDTH, MAZE_HEIGHT, start, end, true);
                destroy_path_result(bfs_res);
                bfs_res = find_path_bfs(maze_walls, MAZE_WIDTH, MAZE_HEIGHT, end, start);
                frame_count = 0;
            }
        }

        // 5. **DESSINER LES MURS EN DERNIER**, par-dessus tout le reste.
        draw_walls_from_bitmask(renderer, maze_walls, MAZE_HEIGHT, MAZE_WIDTH);

        SDL_RenderPresent(renderer);
        frame_count++;
        SDL_Delay(16);
    }

    // --- Nettoyage ---
    printf("Nettoyage des ressources...\n");
    free(maze_walls);
    destroy_path_result(bfs_res);
    destroy_path_result(anim_path_res);
    SDL_DestroyTexture(tileset);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}