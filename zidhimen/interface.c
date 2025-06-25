#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


#define M_PI 3.14159265358979323846

// --- Constantes ---
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
#define NUM_LAYERS 6 // Nombre d'images de fond (bg0 à bg5)

// --- Structures ---
typedef enum {
    EASY,
    MEDIUM,
    HARD,
    DIFFICULTY_COUNT // Vaut 3, pratique pour les calculs
} DifficultyLevel;


// ===================================================================================
// IMPLÉMENTATION DES NOUVELLES FONCTIONS
// ===================================================================================

/**
 * @brief Initialise la SDL, la fenêtre et le renderer.
 * @param window Pointeur vers le pointeur de la fenêtre SDL.
 * @param renderer Pointeur vers le pointeur du renderer SDL.
 * @return true si l'initialisation a réussi, false sinon.
 */
bool init_sdl(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erreur d'initialisation de la SDL : %s\n", SDL_GetError());
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("Erreur d'initialisation de SDL_image : %s\n", IMG_GetError());
        return false;
    }

    *window = SDL_CreateWindow("Menu à Défilement Parallaxe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!*window) {
        printf("Erreur de création de la fenêtre : %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer) {
        printf("Erreur de création du renderer : %s\n", SDL_GetError());
        return false;
    }
    return true;
}

/**
 * @brief Charge les images de fond, initialise leurs vitesses et positions.
 */
void load_parallax_assets(SDL_Renderer* renderer, SDL_Texture* layers[], float speeds[], float x1[], float x2[]) {
    char filename[16];
    for (int i = 0; i < NUM_LAYERS; i++) {
        snprintf(filename, sizeof(filename), "bg%d.png", i);
        layers[i] = IMG_LoadTexture(renderer, filename);
        if (!layers[i]) {
            printf("Erreur: Impossible de charger '%s'.\n", filename);
        }
        speeds[i] = 0.5f + (float)i * 0.75f;
        x1[i] = 0.0f;
        x2[i] = (float)SCREEN_WIDTH;
    }
}

/**
 * @brief Gère tous les événements utilisateur (souris, fermeture de fenêtre).
 * @param running Pointeur vers le booléen de la boucle principale.
 * @param difficulty Pointeur vers le niveau de difficulté actuel.
 * @param play_rect Rectangle du bouton "Jouer".
 * @param quit_rect Rectangle du bouton "Quitter".
 * @param left_arrow Rectangle de la flèche gauche de difficulté.
 * @param right_arrow Rectangle de la flèche droite de difficulté.
 */
void handle_events(bool* running, DifficultyLevel* difficulty, const SDL_Rect* play_rect, const SDL_Rect* quit_rect, const SDL_Rect* left_arrow, const SDL_Rect* right_arrow) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = false;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            SDL_Point mouse_pos = {event.button.x, event.button.y};
            if (SDL_PointInRect(&mouse_pos, play_rect)) { printf("Lancement du jeu !\n"); }
            if (SDL_PointInRect(&mouse_pos, left_arrow)) { *difficulty = (*difficulty - 1 + DIFFICULTY_COUNT) % DIFFICULTY_COUNT; }
            if (SDL_PointInRect(&mouse_pos, right_arrow)) { *difficulty = (*difficulty + 1) % DIFFICULTY_COUNT; }
            if (SDL_PointInRect(&mouse_pos, quit_rect)) { *running = false; }
        }
    }
}

/**
 * @brief Met à jour la position des images de fond pour créer l'effet de parallaxe.
 */
void update_parallax(float speeds[], float x1[], float x2[]) {
    for (int i = 0; i < NUM_LAYERS; i++) {
        x1[i] -= speeds[i];
        x2[i] -= speeds[i];
        if (x1[i] <= -SCREEN_WIDTH) { x1[i] = x2[i] + SCREEN_WIDTH; }
        if (x2[i] <= -SCREEN_WIDTH) { x2[i] = x1[i] + SCREEN_WIDTH; }
    }
}

/**
 * @brief Dessine l'ensemble de la scène (fond et menu).
 */
void render_all(SDL_Renderer* renderer, SDL_Texture* layers[], float x1[], float x2[], const SDL_Rect* play_rect, const SDL_Rect* diff_rect, const SDL_Rect* quit_rect, DifficultyLevel difficulty) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 1. Dessiner le fond en parallaxe
    for (int i = 0; i < NUM_LAYERS; i++) {
        if (layers[i]) {
            SDL_Rect dst1 = {(int)x1[i], 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, layers[i], NULL, &dst1);
            SDL_Rect dst2 = {(int)x2[i], 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, layers[i], NULL, &dst2);
        }
    }

    // 2. Dessiner les boutons
    SDL_Point mouse_pos;
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    draw_decorated_button(renderer, (SDL_Rect*)play_rect, "play", difficulty, SDL_PointInRect(&mouse_pos, play_rect));
    draw_decorated_button(renderer, (SDL_Rect*)diff_rect, "difficulty", difficulty, SDL_PointInRect(&mouse_pos, diff_rect));
    draw_decorated_button(renderer, (SDL_Rect*)quit_rect, "quit", difficulty, SDL_PointInRect(&mouse_pos, quit_rect));

    // Mettre à jour l'écran
    SDL_RenderPresent(renderer);
}

/**
 * @brief Libère toutes les ressources allouées.
 */
void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* layers[]) {
    for (int i = 0; i < NUM_LAYERS; i++) {
        if (layers[i]) SDL_DestroyTexture(layers[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}


// ===================================================================================
// IMPLÉMENTATION DES FONCTIONS DE DESSIN (INCHANGÉES)
// ===================================================================================

/*
   Dessine une icône "Power" (alimentation) centrée dans un rectangle.
*/
void draw_power_icon(SDL_Renderer* renderer, SDL_Rect* button_rect, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    int cx = button_rect->x + button_rect->w / 2;
    int cy = button_rect->y + button_rect->h / 2;
    int radius = 16;
    // Trait vertical (épais)
    for (int i = -1; i <= 1; i++) {
        SDL_RenderDrawLine(renderer, cx + i, cy - radius + 2, cx + i, cy);
    }
    // Arc de cercle (épais)
    float start_angle = 45.0f * M_PI / 180.0f, end_angle = 315.0f * M_PI / 180.0f, step = M_PI / 180.0f;
    for (int i = -1; i <= 1; i++) {
        float prev_x = cx + (radius + i) * cos(start_angle), prev_y = cy + (radius + i) * sin(start_angle);
        for (float angle = start_angle + step; angle <= end_angle; angle += step) {
            float x = cx + (radius + i) * cos(angle), y = cy + (radius + i) * sin(angle);
            SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);
            prev_x = x; prev_y = y;
        }
    }
}

/*
Dessine le contrôle de difficulté avec des flèches et une jauge.
*/
void draw_difficulty_control(SDL_Renderer* renderer, SDL_Rect* rect, DifficultyLevel difficulty, SDL_Color color) {
    int padding = 25, arrow_width = 30, arrow_cy = rect->y + rect->h / 2;
    // Flèche gauche
    SDL_Vertex left_arrow[] = {{{rect->x + padding + 10, arrow_cy - 10}, color, {0,0}}, {{rect->x + padding, arrow_cy}, color, {0,0}}, {{rect->x + padding + 10, arrow_cy + 10}, color, {0,0}}};
    SDL_RenderGeometry(renderer, NULL, left_arrow, 3, NULL, 0);
    // Flèche droite
    SDL_Vertex right_arrow[] = {{{rect->x + rect->w - padding - 10, arrow_cy - 10}, color, {0,0}}, {{rect->x + rect->w - padding, arrow_cy}, color, {0,0}}, {{rect->x + rect->w - padding - 10, arrow_cy + 10}, color, {0,0}}};
    SDL_RenderGeometry(renderer, NULL, right_arrow, 3, NULL, 0);
    // Jauge de difficulté
    int bar_padding = 5, total_bar_width = rect->w - 2 * (padding + arrow_width), bar_width = (total_bar_width - (DIFFICULTY_COUNT - 1) * bar_padding) / DIFFICULTY_COUNT;
    int bar_height = 25, start_x = rect->x + padding + arrow_width;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        SDL_Rect bar_rect = {start_x + i * (bar_width + bar_padding), rect->y + (rect->h - bar_height) / 2, bar_width, bar_height};
        if (i <= (int)difficulty) { SDL_RenderFillRect(renderer, &bar_rect); } else { SDL_RenderDrawRect(renderer, &bar_rect); }
    }
}

/*
Dessine un bouton décoré avec un fond semi-transparent, une bordure et une icône.
*/
void draw_decorated_button(SDL_Renderer* renderer, SDL_Rect* rect, const char* icon_type, DifficultyLevel difficulty, bool is_hovered) {
    // 1. Fond semi-transparent
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_RenderFillRect(renderer, rect);
    
    // 2. Bordure et couleur de l'icône (jaune si survolé, blanc sinon)
    SDL_Color icon_color;
    if (is_hovered) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        icon_color = (SDL_Color){255, 255, 0, 255};
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        icon_color = (SDL_Color){255, 255, 255, 255};
    }
    SDL_RenderDrawRect(renderer, rect);

    // 3. Dessin de l'icône appropriée
    if (strcmp(icon_type, "play") == 0) {
        int cx = rect->x + rect->w / 2, cy = rect->y + rect->h / 2;
        SDL_Vertex vertices[] = {{{cx - 10, cy - 15}, icon_color, {0,0}}, {{cx + 15, cy}, icon_color, {0,0}}, {{cx - 10, cy + 15}, icon_color, {0,0}}};
        SDL_RenderGeometry(renderer, NULL, vertices, 3, NULL, 0);
    } else if (strcmp(icon_type, "quit") == 0) {
        draw_power_icon(renderer, rect, icon_color);
    } else if (strcmp(icon_type, "difficulty") == 0) {
        draw_difficulty_control(renderer, rect, difficulty, icon_color);
    }
    
    // Rétablir le mode de rendu par défaut
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
// ===================================================================================
// FONCTION PRINCIPALE (SIMPLIFIÉE)
// ===================================================================================
int main(int argc, char* argv[]) {
    (void)argc; (void)argv; // Marquer comme non utilisés

    // --- Variables ---
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* layers[NUM_LAYERS];
    float speeds[NUM_LAYERS], x1[NUM_LAYERS], x2[NUM_LAYERS];
    DifficultyLevel current_difficulty = EASY;

    // --- Initialisation ---
    if (!init_sdl(&window, &renderer)) {
        return 1; // Quitte si l'initialisation échoue
    }
    load_parallax_assets(renderer, layers, speeds, x1, x2);

    // --- Définition des éléments de l'interface (UI) ---
    int button_width = 280, button_height = 70;
    int center_x = (SCREEN_WIDTH - button_width) / 2;
    SDL_Rect play_rect = {center_x, 150, button_width, button_height};
    SDL_Rect diff_rect = {center_x, 250, button_width, button_height};
    SDL_Rect quit_rect = {center_x, 350, button_width, button_height};
    SDL_Rect diff_left_arrow_rect = {diff_rect.x, diff_rect.y, diff_rect.w / 3, diff_rect.h};
    SDL_Rect diff_right_arrow_rect = {diff_rect.x + (2 * diff_rect.w / 3), diff_rect.y, diff_rect.w / 3, diff_rect.h};

    // --- Boucle Principale ---
    bool running = true;
    while (running) {
        // 1. Gérer les entrées utilisateur
        handle_events(&running, ¤t_difficulty, &play_rect, &quit_rect, &diff_left_arrow_rect, &diff_right_arrow_rect);

        // 2. Mettre à jour l'état du jeu (ici, juste le fond)
        update_parallax(speeds, x1, x2);
        
        // 3. Dessiner tout à l'écran
        render_all(renderer, layers, x1, x2, &play_rect, &diff_rect, &quit_rect, current_difficulty);
        
        // Viser ~60 FPS
        SDL_Delay(16);
    }

    // --- Libération des Ressources ---
    cleanup(window, renderer, layers);
    return 0;
}

