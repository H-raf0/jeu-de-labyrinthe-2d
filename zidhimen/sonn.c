#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdlib.h> // Pour rand() et srand()
#include <time.h>   // Pour srand()
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define M_PI 3.14159265358979323846

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// --- MODIFICATION : On utilise maintenant 6 couches ---
#define NUM_LAYERS 6

typedef enum {
    EASY,
    MEDIUM,
    HARD,
    DIFFICULTY_COUNT
} DifficultyLevel;

// ... (Toutes vos fonctions de dessin "draw_..." restent identiques) ...
void draw_power_icon(SDL_Renderer* renderer, SDL_Rect* button_rect, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    int cx = button_rect->x + button_rect->w / 2;
    int cy = button_rect->y + button_rect->h / 2;
    int radius = 16;
    
    // Corps de l'icône
    for (int i = -1; i <= 1; i++) {
        SDL_RenderDrawLine(renderer, cx + i, cy - radius + 2, cx + i, cy);
    }
    
    // Cercle externe
    float start_angle = 45.0f * M_PI / 180.0f;
    float end_angle = 315.0f * M_PI / 180.0f;
    float step = M_PI / 180.0f;
    
    for (int offset = -1; offset <= 1; offset++) {
        float prev_x = cx + (radius + offset) * cos(start_angle);
        float prev_y = cy + (radius + offset) * sin(start_angle);
        
        for (float angle = start_angle + step; angle <= end_angle; angle += step) {
            float x = cx + (radius + offset) * cos(angle);
            float y = cy + (radius + offset) * sin(angle);
            SDL_RenderDrawLine(renderer, (int)prev_x, (int)prev_y, (int)x, (int)y);
            prev_x = x;
            prev_y = y;
        }
    }
}

void draw_difficulty_control(SDL_Renderer* renderer, SDL_Rect* rect, DifficultyLevel difficulty, SDL_Color color) {
    int padding = 25;
    int arrow_width = 30;
    int arrow_cy = rect->y + rect->h / 2;
    
    // Flèche gauche
    SDL_Point left_arrow[3] = {
        {rect->x + padding + 10, arrow_cy - 10},
        {rect->x + padding, arrow_cy},
        {rect->x + padding + 10, arrow_cy + 10}
    };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLines(renderer, left_arrow, 3);
    
    // Flèche droite
    SDL_Point right_arrow[3] = {
        {rect->x + rect->w - padding - 10, arrow_cy - 10},
        {rect->x + rect->w - padding, arrow_cy},
        {rect->x + rect->w - padding - 10, arrow_cy + 10}
    };
    SDL_RenderDrawLines(renderer, right_arrow, 3);
    
    // Barres de difficulté
    int bar_padding = 5;
    int total_bar_width = rect->w - 2 * (padding + arrow_width);
    int bar_width = (total_bar_width - (DIFFICULTY_COUNT - 1) * bar_padding) / DIFFICULTY_COUNT;
    int bar_height = 25;
    int start_x = rect->x + padding + arrow_width;
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        SDL_Rect bar_rect = {
            start_x + i * (bar_width + bar_padding),
            rect->y + (rect->h - bar_height) / 2,
            bar_width,
            bar_height
        };
        
        if (i <= (int)difficulty) {
            SDL_RenderFillRect(renderer, &bar_rect);
        } else {
            SDL_RenderDrawRect(renderer, &bar_rect);
        }
    }
}

void draw_decorated_button(SDL_Renderer* renderer, SDL_Rect* rect, const char* icon_type, DifficultyLevel difficulty, bool is_hovered) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_RenderFillRect(renderer, rect);
    
    SDL_Color icon_color;
    if (is_hovered) {
        icon_color = (SDL_Color){255, 255, 0, 255};
    } else {
        icon_color = (SDL_Color){255, 255, 255, 255};
    }
    
    SDL_SetRenderDrawColor(renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
    SDL_RenderDrawRect(renderer, rect);

    if (strcmp(icon_type, "play") == 0) {
        int cx = rect->x + rect->w / 2;
        int cy = rect->y + rect->h / 2;
        SDL_Point vertices[3] = {
            {cx - 10, cy - 15},
            {cx + 15, cy},
            {cx - 10, cy + 15}
        };
        SDL_RenderDrawLines(renderer, vertices, 3);
    } else if (strcmp(icon_type, "quit") == 0) {
        draw_power_icon(renderer, rect, icon_color);
    } else if (strcmp(icon_type, "difficulty") == 0) {
        draw_difficulty_control(renderer, rect, difficulty, icon_color);
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void draw_sound_icon(SDL_Renderer* renderer, SDL_Rect* rect, bool sound_on, SDL_Color color) {
    int cx = rect->x + rect->w / 2;
    int cy = rect->y + rect->h / 2;
    int size = 20;
    
    // Corps du haut-parleur
    SDL_Rect body = {cx - size/3, cy - size/2, size/3, size};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &body);
    
    // Triangle
    SDL_Point triangle[3] = {
        {cx, cy - size/2},
        {cx + size/2, cy},
        {cx, cy + size/2}
    };
    SDL_RenderDrawLines(renderer, triangle, 3);
    
    if (sound_on) {
        // Ondes sonores
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (int r = size; r <= size * 1.5; r += 3) {
            for (float angle = 0; angle < M_PI/4; angle += 0.1f) {
                float x1 = cx + r * cos(angle);
                float y1 = cy - r * sin(angle);
                float x2 = cx + r * cos(angle + 0.1f);
                float y2 = cy - r * sin(angle + 0.1f);
                SDL_RenderDrawLine(renderer, (int)x1, (int)y1, (int)x2, (int)y2);
            }
        }
    } else {
        // Croix de mute
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLine(renderer, 
            rect->x, rect->y, 
            rect->x + rect->w, rect->y + rect->h);
        SDL_RenderDrawLine(renderer, 
            rect->x, rect->y + rect->h, 
            rect->x + rect->w, rect->y);
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    srand(time(NULL)); // Initialise le générateur de nombres aléatoires

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { /*...*/ return 1; }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) { /*...*/ return 1; }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { /*...*/ return 1; }

    SDL_Window* window = SDL_CreateWindow("Menu Parallaxe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) { /*...*/ return 1; }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { /*...*/ return 1; }

    // Charger les sons (aucune modification ici)
    Mix_Chunk* clickSound = Mix_LoadWAV("click.wav");
    Mix_Chunk* hoverSound = Mix_LoadWAV("hover.wav");
    Mix_Music *backgroundMusic = Mix_LoadMUS("musique.mp3");
    if (backgroundMusic) { Mix_PlayMusic(backgroundMusic, -1); }

    const char* difficulty_names[] = {"Facile", "Moyen", "Difficile"};

    // --- MODIFICATION : Chargement intelligent des 6 couches ---
    SDL_Texture* layers[NUM_LAYERS] = {NULL};
    float speeds[NUM_LAYERS];
    float x1[NUM_LAYERS], x2[NUM_LAYERS];
    int layer_w[NUM_LAYERS], layer_h[NUM_LAYERS]; // Pour stocker la taille originale de chaque image
    int layer_y[NUM_LAYERS]; // Pour la position verticale aléatoire des particules

    // On définit l'ordre de chargement pour un rendu logique (du fond vers l'avant)
    const char* filenames[NUM_LAYERS] = {
        "bgg5.png", // Couche 0: Étoiles (le plus loin)
        "bgg4.png", // Couche 1: Nébuleuse
        "bgg2.png", // Couche 2: Particule 1
        "bgg3.png", // Couche 3: Particule 2
        "bgg0.png", // Couche 4: Particule 3
        "bgg1.png"  // Couche 5: Particule 4 (le plus proche)
    };
    
    for (int i = 0; i < NUM_LAYERS; i++) {
        layers[i] = IMG_LoadTexture(renderer, filenames[i]);
        if (!layers[i]) {
            printf("Échec du chargement de %s: %s\n", filenames[i], IMG_GetError());
        } else {
            // On récupère la taille de l'image pour ne pas la déformer
            SDL_QueryTexture(layers[i], NULL, NULL, &layer_w[i], &layer_h[i]);
        }
        
        // Vitesse progressive : les premières couches (fond) sont plus lentes
        speeds[i] = 0.4f + (float)i * (float)i * 0.2f;
        
        x1[i] = 0.0f;
        // Positionne la deuxième copie correctement, même pour les petites images
        x2[i] = (float)layer_w[i];
        
        // Position verticale aléatoire pour les particules
        layer_y[i] = rand() % (SCREEN_HEIGHT - layer_h[i]);
    }

    // Configuration initiale de l'UI (aucune modification ici)
    DifficultyLevel current_difficulty = EASY;
    bool sound_on = true;
    int button_width = 280, button_height = 70;
    int center_x = (SCREEN_WIDTH - button_width) / 2;
    SDL_Rect play_rect = {center_x, 150, button_width, button_height};
    SDL_Rect diff_rect = {center_x, 250, button_width, button_height};
    SDL_Rect quit_rect = {center_x, 350, button_width, button_height};
    SDL_Rect sound_rect = {SCREEN_WIDTH - 60, 20, 40, 40};
    SDL_Rect diff_left_arrow_rect = {diff_rect.x, diff_rect.y, diff_rect.w / 3, diff_rect.h};
    SDL_Rect diff_right_arrow_rect = {diff_rect.x + (2 * diff_rect.w / 3), diff_rect.y, diff_rect.w / 3, diff_rect.h};
    bool was_hovering_play = false, was_hovering_diff = false, was_hovering_quit = false, was_hovering_sound = false;

    // Boucle principale
    bool running = true;
    SDL_Event event;
    while (running) {
        // La gestion des événements reste la même
        DifficultyLevel old_difficulty = current_difficulty;
        while (SDL_PollEvent(&event)) { /* ... */ } // Code de gestion des événements inchangé

        // --- MODIFICATION : Logique de mise à jour et de rendu ---
        
        // Effacer l'écran
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Rendu des 6 couches de fond
        for (int i = 0; i < NUM_LAYERS; i++) {
            if (layers[i]) {
                // Mise à jour des positions
                x1[i] -= speeds[i];
                x2[i] -= speeds[i];

                int current_w = layer_w[i];
                
                // Si une image sort de l'écran par la gauche, on la replace à droite
                if (x1[i] <= -current_w) {
                    x1[i] = x2[i] + current_w;
                    // Change la position Y des particules quand elles réapparaissent
                    if (i >= 2) layer_y[i] = rand() % (SCREEN_HEIGHT - layer_h[i]);
                }
                if (x2[i] <= -current_w) {
                    x2[i] = x1[i] + current_w;
                    if (i >= 2) layer_y[i] = rand() % (SCREEN_HEIGHT - layer_h[i]);
                }

                // Déterminer la taille de rendu
                int render_w, render_h, render_y;
                if (i < 2) { // Pour les couches 0 (étoiles) et 1 (nébuleuse)
                    render_w = SCREEN_WIDTH + 5; // On étire pour remplir l'écran (+5 pour éviter les trous)
                    render_h = SCREEN_HEIGHT;
                    render_y = 0;
                } else { // Pour les couches de particules
                    render_w = layer_w[i]; // On garde la taille originale
                    render_h = layer_h[i];
                    render_y = layer_y[i]; // On utilise la position Y stockée
                }
                
                // Rendu des deux copies de l'image pour un défilement continu
                SDL_Rect dst1 = {(int)x1[i], render_y, render_w, render_h};
                SDL_RenderCopy(renderer, layers[i], NULL, &dst1);
                
                SDL_Rect dst2 = {(int)x2[i], render_y, render_w, render_h};
                SDL_RenderCopy(renderer, layers[i], NULL, &dst2);
            }
        }

        // Le rendu de l'interface utilisateur reste le même
        SDL_Point mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        bool is_hovering_play = SDL_PointInRect(&mouse_pos, &play_rect);
        bool is_hovering_diff = SDL_PointInRect(&mouse_pos, &diff_rect);
        bool is_hovering_quit = SDL_PointInRect(&mouse_pos, &quit_rect);
        bool is_hovering_sound = SDL_PointInRect(&mouse_pos, &sound_rect);
        
        // ... (gestion du survol et dessin des boutons inchangés) ...


        // Présenter le rendu final
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Vise ~60 FPS
    }

    // Nettoyage
    for (int i = 0; i < NUM_LAYERS; i++) {
        if (layers[i]) SDL_DestroyTexture(layers[i]);
    }
    if (clickSound) Mix_FreeChunk(clickSound);
    if (hoverSound) Mix_FreeChunk(hoverSound);
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    return 0;
}