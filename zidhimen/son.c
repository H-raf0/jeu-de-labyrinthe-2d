#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>


#define M_PI 3.14159265358979323846


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
#define NUM_LAYERS 2

typedef enum {
    EASY,
    MEDIUM,
    HARD,
    DIFFICULTY_COUNT
} DifficultyLevel;

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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image initialization failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer initialization failed: %s\n", Mix_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Menu à Défilement Parallaxe", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        0
    );
    
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        Mix_CloseAudio();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Charger les sons
    Mix_Chunk* clickSound = Mix_LoadWAV("click.wav");
    if (!clickSound) {
        printf("Failed to load click sound: %s\n", Mix_GetError());
    }
    
    Mix_Chunk* hoverSound = Mix_LoadWAV("hover.wav");
    if (!hoverSound) {
        printf("Failed to load hover sound: %s\n", Mix_GetError());
    }

    Mix_Music *backgroundMusic = Mix_LoadMUS("musique.mp3");
    if (backgroundMusic) {
        if (Mix_PlayMusic(backgroundMusic, -1) == -1) {
            printf("Failed to play music: %s\n", Mix_GetError());
        }
    } else {
        printf("Failed to load music: %s\n", Mix_GetError());
    }

    const char* difficulty_names[] = {"Facile", "Moyen", "Difficile"};

    // Charger les couches de fond
    SDL_Texture* layers[NUM_LAYERS] = {NULL};
    float speeds[NUM_LAYERS];
    float x1[NUM_LAYERS], x2[NUM_LAYERS];
    
    for (int i = 0; i < NUM_LAYERS; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "bg%d.png", i);
        layers[i] = IMG_LoadTexture(renderer, filename);
        if (!layers[i]) {
            printf("Failed to load %s: %s\n", filename, IMG_GetError());
        }
        speeds[i] = 0.5f + (float)i * 0.75f;
        x1[i] = 0.0f;
        x2[i] = (float)SCREEN_WIDTH;
    }

    // Configuration initiale
    DifficultyLevel current_difficulty = EASY;
    bool sound_on = true;
    int button_width = 280, button_height = 70;
    int center_x = (SCREEN_WIDTH - button_width) / 2;
    
    SDL_Rect play_rect = {center_x, 150, button_width, button_height};
    SDL_Rect diff_rect = {center_x, 250, button_width, button_height};
    SDL_Rect quit_rect = {center_x, 350, button_width, button_height};
    SDL_Rect sound_rect = {SCREEN_WIDTH - 60, 20, 40, 40};
    
    // Zones cliquables pour les flèches de difficulté
    SDL_Rect diff_left_arrow_rect = {
        diff_rect.x, 
        diff_rect.y, 
        diff_rect.w / 3, 
        diff_rect.h
    };
    
    SDL_Rect diff_right_arrow_rect = {
        diff_rect.x + (2 * diff_rect.w / 3), 
        diff_rect.y, 
        diff_rect.w / 3, 
        diff_rect.h
    };

    bool was_hovering_play = false;
    bool was_hovering_diff = false;
    bool was_hovering_quit = false;
    bool was_hovering_sound = false;

    // Boucle principale
    bool running = true;
    SDL_Event event;
    while (running) {
        DifficultyLevel old_difficulty = current_difficulty;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point mouse_pos = {event.button.x, event.button.y};

                // Jouer le son du clic si activé
                if (sound_on && clickSound && (
                    SDL_PointInRect(&mouse_pos, &play_rect) ||
                    SDL_PointInRect(&mouse_pos, &diff_rect) ||
                    SDL_PointInRect(&mouse_pos, &quit_rect) ||
                    SDL_PointInRect(&mouse_pos, &sound_rect)
                )) {
                    Mix_PlayChannel(-1, clickSound, 0);
                }

                // Gestion des clics
                if (SDL_PointInRect(&mouse_pos, &play_rect)) {
                    printf("Lancement du jeu niveau %s !\n", difficulty_names[current_difficulty]);
                }
                else if (SDL_PointInRect(&mouse_pos, &diff_left_arrow_rect)) { 
                    current_difficulty = (current_difficulty - 1 + DIFFICULTY_COUNT) % DIFFICULTY_COUNT; 
                }
                else if (SDL_PointInRect(&mouse_pos, &diff_right_arrow_rect)) { 
                    current_difficulty = (current_difficulty + 1) % DIFFICULTY_COUNT; 
                }
                else if (SDL_PointInRect(&mouse_pos, &quit_rect)) { 
                    running = false; 
                }
                else if (SDL_PointInRect(&mouse_pos, &sound_rect)) {
                    sound_on = !sound_on;
                    if (sound_on) {
                        Mix_ResumeMusic();
                        Mix_Volume(-1, MIX_MAX_VOLUME);
                    } else {
                        Mix_PauseMusic();
                        Mix_Volume(-1, 0);
                    }
                }
            }
        }

        if (old_difficulty != current_difficulty) {
            printf("Changement de difficulté -> Niveau %s\n", difficulty_names[current_difficulty]);
        }

        // Rendu des couches de fond
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < NUM_LAYERS; i++) {
            if (layers[i]) {
                x1[i] -= speeds[i];
                x2[i] -= speeds[i];
                
                if (x1[i] <= -SCREEN_WIDTH) x1[i] = x2[i] + SCREEN_WIDTH;
                if (x2[i] <= -SCREEN_WIDTH) x2[i] = x1[i] + SCREEN_WIDTH;
                
                SDL_Rect dst1 = {(int)x1[i], 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_RenderCopy(renderer, layers[i], NULL, &dst1);
                
                SDL_Rect dst2 = {(int)x2[i], 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_RenderCopy(renderer, layers[i], NULL, &dst2);
            }
        }

        // Gestion du survol des boutons
        SDL_Point mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        
        bool is_hovering_play = SDL_PointInRect(&mouse_pos, &play_rect);
        bool is_hovering_diff = SDL_PointInRect(&mouse_pos, &diff_rect);
        bool is_hovering_quit = SDL_PointInRect(&mouse_pos, &quit_rect);
        bool is_hovering_sound = SDL_PointInRect(&mouse_pos, &sound_rect);
        
        // Jouer les sons de survol
        if (sound_on && hoverSound) {
            if (is_hovering_play && !was_hovering_play) {
                Mix_PlayChannel(-1, hoverSound, 0);
            }
            if (is_hovering_diff && !was_hovering_diff) {
                Mix_PlayChannel(-1, hoverSound, 0);
            }
            if (is_hovering_quit && !was_hovering_quit) {
                Mix_PlayChannel(-1, hoverSound, 0);
            }
            if (is_hovering_sound && !was_hovering_sound) {
                Mix_PlayChannel(-1, hoverSound, 0);
            }
        }
        
        was_hovering_play = is_hovering_play;
        was_hovering_diff = is_hovering_diff;
        was_hovering_quit = is_hovering_quit;
        was_hovering_sound = is_hovering_sound;

        // Dessin des éléments d'interface
        draw_decorated_button(renderer, &play_rect, "play", current_difficulty, is_hovering_play);
        draw_decorated_button(renderer, &diff_rect, "difficulty", current_difficulty, is_hovering_diff);
        draw_decorated_button(renderer, &quit_rect, "quit", current_difficulty, is_hovering_quit);

        // Bouton son
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_RenderFillRect(renderer, &sound_rect);
        
        SDL_Color sound_color = is_hovering_sound ? 
            (SDL_Color){255, 255, 0, 255} : 
            (SDL_Color){255, 255, 255, 255};
        
        SDL_SetRenderDrawColor(renderer, sound_color.r, sound_color.g, sound_color.b, sound_color.a);
        SDL_RenderDrawRect(renderer, &sound_rect);
        
        draw_sound_icon(renderer, &sound_rect, sound_on, sound_color);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
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