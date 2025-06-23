#include <stdio.h>
#include "maze.h"
#include "pathfinding.h"
#include "graphics.h"

// Le chemin vers votre image
const char* TILESET_PATH = "tileset.png"; // Remplacer par le nom de votre image

int main(int argc, char* argv[]) {
    // --- 1. Création d'un labyrinthe arborescent (p=0) ---
    printf("Génération du labyrinthe...\n");
    Maze* maze = create_maze(MAZE_WIDTH, MAZE_HEIGHT);
    
    // Pour un labyrinthe arborescent parfait
    // generate_maze_kruskal(maze, 0.0f); 
    
    // Pour un labyrinthe avec des cycles (non arborescent)
    generate_maze_kruskal(maze, 0.05f); // 5% de chance de créer un cycle

    // --- 2. Initialisation Graphique ---
    Graphics* g = init_graphics();
    if (!g) return 1;
    // La fonction draw_maze_tiles ne fonctionne pas avec cette image.
    // Utilisons le dessin par lignes, plus simple.
    // load_tileset(g, TILESET_PATH); 

    // --- 3. Définition départ/arrivée et recherche de chemin ---
    Point start = {0, 0};
    Point end = {MAZE_WIDTH - 1, MAZE_HEIGHT - 1};

    // --- Travail à réaliser : Promenades dans le graphe ---
    printf("\n--- Section 1: BFS et dégradé de distance ---\n");
    // On part de la destination pour colorer tout le labyrinthe
    PathResult* bfs_res = bfs_path(maze, end, start);
    
    // --- Boucle Principale ---
    SDL_Event e;
    bool quit = false;
    int current_frame = 0; // Pour l'animation
    
    // Générer un chemin avec A* pour l'animation
    PathResult* a_star_res_anim = a_star_path(maze, start, end, MANHATTAN);

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        // --- Rendu ---
        SDL_SetRenderDrawColor(g->renderer, 20, 20, 40, 255);
        SDL_RenderClear(g->renderer);
        
        // Affichage du labyrinthe avec des lignes
        draw_maze_lines(g, maze);
        
        // Affichage du dégradé de couleurs basé sur la distance (BFS)
        draw_distance_gradient(g, bfs_res);

        // Affichage des informations du chemin A* (noeuds visités, chemin)
        draw_path_info(g, a_star_res_anim, start, end);
        
        // Petite animation simple du personnage
        if (a_star_res_anim && a_star_res_anim->path_len > 0) {
            int anim_pos = (current_frame / 10) % a_star_res_anim->path_len;
            Point char_pos = a_star_res_anim->path[anim_pos];
            SDL_Rect char_rect = {char_pos.x * TILE_SIZE + TILE_SIZE/4, char_pos.y * TILE_SIZE+TILE_SIZE/4, TILE_SIZE/2, TILE_SIZE/2};
            SDL_SetRenderDrawColor(g->renderer, 255, 105, 180, 255); // Rose
            SDL_RenderFillRect(g->renderer, &char_rect);

            // A la fin, on choisit une nouvelle destination
            if(anim_pos == a_star_res_anim->path_len - 1 && a_star_res_anim->path_len > 1) {
                 SDL_Delay(500); // Pause
                 start = end;
                 end = (Point){rand() % MAZE_WIDTH, rand() % MAZE_HEIGHT};
                 destroy_path_result(a_star_res_anim);
                 a_star_res_anim = a_star_path(maze, start, end, MANHATTAN);
                 current_frame = 0;
            }
        }
        
        SDL_RenderPresent(g->renderer);
        current_frame++;
        SDL_Delay(16); // ~60 FPS
    }
    
    // --- 4. Comparaison des heuristiques A* ---
    printf("\n--- Section 2: Comparaison des heuristiques A* ---\n");
    PathResult* res_dijkstra = dijkstra_path(maze, start, end);
    PathResult* res_manhattan = a_star_path(maze, start, end, MANHATTAN);
    PathResult* res_euclidean = a_star_path(maze, start, end, EUCLIDEAN);
    PathResult* res_chebyshev = a_star_path(maze, start, end, CHEBYSHEV);
    
    printf("Nombre de noeuds visités :\n");
    printf(" - Dijkstra (A* avec h=0): %d\n", res_dijkstra ? res_dijkstra->visited_count : -1);
    printf(" - A* avec Manhattan    : %d\n", res_manhattan ? res_manhattan->visited_count : -1);
    printf(" - A* avec Euclidienne  : %d\n", res_euclidean ? res_euclidean->visited_count : -1);
    printf(" - A* avec Chebyshev    : %d\n", res_chebyshev ? res_chebyshev->visited_count : -1);

    // --- 5. Nettoyage ---
    destroy_maze(maze);
    destroy_path_result(bfs_res);
    destroy_path_result(a_star_res_anim);
    destroy_path_result(res_dijkstra);
    destroy_path_result(res_manhattan);
    destroy_path_result(res_euclidean);
    destroy_path_result(res_chebyshev);
    close_graphics(g);

    return 0;
}