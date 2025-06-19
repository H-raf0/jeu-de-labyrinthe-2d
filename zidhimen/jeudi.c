#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define WIDTH 30
#define HEIGHT 80
#define TILE_SIZE 20
#define SCREEN_WIDTH (WIDTH * TILE_SIZE)
#define SCREEN_HEIGHT (HEIGHT * TILE_SIZE)

// Direction
enum { UP, DOWN, LEFT, RIGHT };

// Structure de chaque cellule
typedef struct {
    int x, y;
    int walls[4]; // UP, DOWN, LEFT, RIGHT
} Cell;

// Structure pour une arête entre deux cellules
typedef struct {
    int x1, y1, x2, y2;
} Edge;

Cell maze[HEIGHT][WIDTH];
Edge *edges = NULL;
int edge_count = 0;

int parent[WIDTH * HEIGHT];

// Initialisation Union-Find
int find(int x) {
    if (parent[x] != x)
        parent[x] = find(parent[x]);
    return parent[x];
}

void unite(int x, int y) {
    parent[find(x)] = find(y);
}

// Mélange aléatoire des arêtes (Fisher-Yates)
void shuffle_edges() {
    for (int i = edge_count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Edge tmp = edges[i];
        edges[i] = edges[j];
        edges[j] = tmp;
    }
}

void init_maze() {
    edge_count = 0;
    edges = malloc(2 * WIDTH * HEIGHT * sizeof(Edge));

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            maze[y][x].x = x;
            maze[y][x].y = y;
            for (int i = 0; i < 4; i++) maze[y][x].walls[i] = 1;
            int id = y * WIDTH + x;
            parent[id] = id;

            if (x < WIDTH - 1) {
                edges[edge_count++] = (Edge){x, y, x + 1, y};
            }
            if (y < HEIGHT - 1) {
                edges[edge_count++] = (Edge){x, y, x, y + 1};
            }
        }
    }
}

void kruskal() {
    shuffle_edges();
    for (int i = 0; i < edge_count; i++) {
        Edge e = edges[i];
        int id1 = e.y1 * WIDTH + e.x1;
        int id2 = e.y2 * WIDTH + e.x2;

        if (find(id1) != find(id2)) {
            unite(id1, id2);
            if (e.x1 == e.x2) {
                // Vertical
                if (e.y1 < e.y2) {
                    maze[e.y1][e.x1].walls[DOWN] = 0;
                    maze[e.y2][e.x2].walls[UP] = 0;
                } else {
                    maze[e.y2][e.x2].walls[DOWN] = 0;
                    maze[e.y1][e.x1].walls[UP] = 0;
                }
            } else {
                // Horizontal
                if (e.x1 < e.x2) {
                    maze[e.y1][e.x1].walls[RIGHT] = 0;
                    maze[e.y2][e.x2].walls[LEFT] = 0;
                } else {
                    maze[e.y2][e.x2].walls[RIGHT] = 0;
                    maze[e.y1][e.x1].walls[LEFT] = 0;
                }
            }
        }
    }
}

void draw_maze(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int x0 = x * TILE_SIZE;
            int y0 = y * TILE_SIZE;
            if (maze[y][x].walls[UP])
                SDL_RenderDrawLine(renderer, x0, y0, x0 + TILE_SIZE, y0);
            if (maze[y][x].walls[DOWN])
                SDL_RenderDrawLine(renderer, x0, y0 + TILE_SIZE, x0 + TILE_SIZE, y0 + TILE_SIZE);
            if (maze[y][x].walls[LEFT])
                SDL_RenderDrawLine(renderer, x0, y0, x0, y0 + TILE_SIZE);
            if (maze[y][x].walls[RIGHT])
                SDL_RenderDrawLine(renderer, x0 + TILE_SIZE, y0, x0 + TILE_SIZE, y0 + TILE_SIZE);
        }
    }

    SDL_RenderPresent(renderer);
}

int main() {
    srand(time(NULL));
    init_maze();
    kruskal();

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Labyrinthe avec Kruskal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    draw_maze(renderer);

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = 1;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(edges);
    return 0;
}