#include <stdio.h>
#include <stdbool.h>
#include "drawing.h"
#include "audio.h"

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    // --- Initialisation SDL ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image initialization failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    if (!init_audio_system()) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // --- Création Fenêtre et Rendu ---
    SDL_Window* window = SDL_CreateWindow("Menu Parallaxe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) { /* ... gestion erreur ... */ return 1; }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { /* ... gestion erreur ... */ return 1; }

    // --- Chargement des ressources ---
    AudioData audio_data;
    load_sounds(&audio_data);

    ParallaxBackground background;
    init_parallax(renderer, &background);

    // --- Variables de jeu ---
    const char* difficulty_names[] = {"Facile", "Moyen", "Difficile"};
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

    // --- Boucle Principale ---
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point mouse_pos = {event.button.x, event.button.y};
                bool clicked_on_button = false;

                if (SDL_PointInRect(&mouse_pos, &play_rect)) {
                    printf("Lancement du jeu niveau %s !\n", difficulty_names[current_difficulty]);
                    clicked_on_button = true;
                } else if (SDL_PointInRect(&mouse_pos, &diff_left_arrow_rect)) {
                    current_difficulty = (current_difficulty - 1 + DIFFICULTY_COUNT) % DIFFICULTY_COUNT;
                    printf("Changement de difficulté -> Niveau %s\n", difficulty_names[current_difficulty]);
                    clicked_on_button = true;
                } else if (SDL_PointInRect(&mouse_pos, &diff_right_arrow_rect)) {
                    current_difficulty = (current_difficulty + 1) % DIFFICULTY_COUNT;
                    printf("Changement de difficulté -> Niveau %s\n", difficulty_names[current_difficulty]);
                    clicked_on_button = true;
                } else if (SDL_PointInRect(&mouse_pos, &quit_rect)) {
                    running = false;
                    clicked_on_button = true;
                } else if (SDL_PointInRect(&mouse_pos, &sound_rect)) {
                    sound_on = !sound_on;
                    toggle_music(sound_on);
                    clicked_on_button = true;
                }
                
                if (sound_on && clicked_on_button) {
                    play_click_sound(&audio_data);
                }
            }
        }

        // --- Rendu ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_parallax(renderer, &background);

        SDL_Point mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        bool is_hovering_play = SDL_PointInRect(&mouse_pos, &play_rect);
        bool is_hovering_diff = SDL_PointInRect(&mouse_pos, &diff_rect);
        bool is_hovering_quit = SDL_PointInRect(&mouse_pos, &quit_rect);
        bool is_hovering_sound = SDL_PointInRect(&mouse_pos, &sound_rect);
        
        draw_decorated_button(renderer, &play_rect, "play", current_difficulty, is_hovering_play);
        draw_decorated_button(renderer, &diff_rect, "difficulty", current_difficulty, is_hovering_diff);
        draw_decorated_button(renderer, &quit_rect, "quit", current_difficulty, is_hovering_quit);
        draw_sound_icon(renderer, &sound_rect, sound_on, is_hovering_sound);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // --- Nettoyage ---
    cleanup_drawing(&background);
    cleanup_audio(&audio_data);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}