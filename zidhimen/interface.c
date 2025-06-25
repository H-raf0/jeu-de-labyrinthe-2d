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
// IMPLÉMENTATION DES FONCTIONS DE DESSIN
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
// FONCTION PRINCIPALE
// ===================================================================================
int main(int argc, char* argv[]) {
    // Marquer les paramètres comme non utilisés pour éviter les avertissements du compilateur
    (void)argc;
    (void)argv;

    // --- Initialisation de SDL ---
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Menu à Défilement Parallaxe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    // MODIF 1 : Tableau pour "traduire" l'enum en chaîne de caractères pour l'affichage.
    // L'ordre doit correspondre à l'enum : EASY=0, MEDIUM=1, HARD=2.
    const char* difficulty_names[] = {"Facile", "Moyen", "Difficile"};

    // --- Gestion du Parallaxe (avec correction du bug de découpage) ---
    SDL_Texture* layers[NUM_LAYERS];
    float speeds[NUM_LAYERS];
    float x1[NUM_LAYERS], x2[NUM_LAYERS];
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

    // --- Variables du Menu ---
    DifficultyLevel current_difficulty = EASY;
    int button_width = 280, button_height = 70;
    int center_x = (SCREEN_WIDTH - button_width) / 2;
    SDL_Rect play_rect = {center_x, 150, button_width, button_height};
    SDL_Rect diff_rect = {center_x, 250, button_width, button_height};
    SDL_Rect quit_rect = {center_x, 350, button_width, button_height};
    SDL_Rect diff_left_arrow_rect = {diff_rect.x, diff_rect.y, diff_rect.w / 3, diff_rect.h};
    SDL_Rect diff_right_arrow_rect = {diff_rect.x + (2 * diff_rect.w / 3), diff_rect.y, diff_rect.w / 3, diff_rect.h};

    // --- Boucle Principale ---
    bool running = true;
    SDL_Event event;
    while (running) {
        // MODIF 2 : Mémoriser la difficulté AVANT de traiter les événements.
        DifficultyLevel old_difficulty = current_difficulty;

        // --- Gestion des Événements ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point mouse_pos = {event.button.x, event.button.y};
                
                // MODIF 3 : Utiliser le tableau `difficulty_names` pour afficher le bon niveau.
                if (SDL_PointInRect(&mouse_pos, &play_rect)) { 
                    printf("Lancement du jeu niveau %s !\n", difficulty_names[current_difficulty]); 
                }
                
                if (SDL_PointInRect(&mouse_pos, &diff_left_arrow_rect)) { current_difficulty = (current_difficulty - 1 + DIFFICULTY_COUNT) % DIFFICULTY_COUNT; }
                if (SDL_PointInRect(&mouse_pos, &diff_right_arrow_rect)) { current_difficulty = (current_difficulty + 1) % DIFFICULTY_COUNT; }
                if (SDL_PointInRect(&mouse_pos, &quit_rect)) { running = false; }
            }
        }
        
        // MODIF 4 : Vérifier si la difficulté a changé APRES avoir traité les événements.
        if (old_difficulty != current_difficulty) {
            printf("Changement de difficulté -> Niveau %s\n", difficulty_names[current_difficulty]);
        }


        // --- Rendu Graphique ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // 1. DESSINER LE FOND EN PARALLAXE AUTOMATIQUE (CORRIGÉ)
        for (int i = 0; i < NUM_LAYERS; i++) {
            if (layers[i]) {
                x1[i] -= speeds[i];
                x2[i] -= speeds[i];
                if (x1[i] <= -SCREEN_WIDTH) { x1[i] = x2[i] + SCREEN_WIDTH; }
                if (x2[i] <= -SCREEN_WIDTH) { x2[i] = x1[i] + SCREEN_WIDTH; }
                SDL_Rect dst1 = {(int)x1[i], 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_RenderCopy(renderer, layers[i], NULL, &dst1);
                SDL_Rect dst2 = {(int)x2[i], 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_RenderCopy(renderer, layers[i], NULL, &dst2);
            }
        }
        
        // 2. DESSINER LES BOUTONS PAR-DESSUS
        SDL_Point mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        draw_decorated_button(renderer, &play_rect, "play", current_difficulty, SDL_PointInRect(&mouse_pos, &play_rect));
        draw_decorated_button(renderer, &diff_rect, "difficulty", current_difficulty, SDL_PointInRect(&mouse_pos, &diff_rect));
        draw_decorated_button(renderer, &quit_rect, "quit", current_difficulty, SDL_PointInRect(&mouse_pos, &quit_rect));

        // Mettre à jour l'écran
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // --- Libération des Ressources ---
    for (int i = 0; i < NUM_LAYERS; i++) {
        if (layers[i]) SDL_DestroyTexture(layers[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}