#include "drawing.h"
#include <string.h>
#include <math.h>

// Définition de PI si non présent dans math.h
#define M_PI 3.14159265358979323846


// --- Fonctions internes (non déclarées dans le .h) ---

void draw_power_icon(SDL_Renderer* renderer, SDL_Rect* button_rect, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    int cx = button_rect->x + button_rect->w / 2;
    int cy = button_rect->y + button_rect->h / 2;
    int radius = 16;
    
    for (int i = -1; i <= 1; i++) {
        SDL_RenderDrawLine(renderer, cx + i, cy - radius + 2, cx + i, cy);
    }
    
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
    
    SDL_Point left_arrow[3] = { {rect->x + padding + 10, arrow_cy - 10}, {rect->x + padding, arrow_cy}, {rect->x + padding + 10, arrow_cy + 10} };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLines(renderer, left_arrow, 3);
    
    SDL_Point right_arrow[3] = { {rect->x + rect->w - padding - 10, arrow_cy - 10}, {rect->x + rect->w - padding, arrow_cy}, {rect->x + rect->w - padding - 10, arrow_cy + 10} };
    SDL_RenderDrawLines(renderer, right_arrow, 3);
    
    int bar_padding = 5;
    int total_bar_width = rect->w - 2 * (padding + arrow_width);
    int bar_width = (total_bar_width - (DIFFICULTY_COUNT - 1) * bar_padding) / DIFFICULTY_COUNT;
    int bar_height = 25;
    int start_x = rect->x + padding + arrow_width;
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        SDL_Rect bar_rect = { start_x + i * (bar_width + bar_padding), rect->y + (rect->h - bar_height) / 2, bar_width, bar_height };
        if (i <= (int)difficulty) {
            SDL_RenderFillRect(renderer, &bar_rect);
        } else {
            SDL_RenderDrawRect(renderer, &bar_rect);
        }
    }
}

// --- Fonctions publiques (déclarées dans le .h) ---

void init_parallax(SDL_Renderer* renderer, ParallaxBackground* bg) {
    for (int i = 0; i < NUM_LAYERS; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "bgg%d.png", i);
        bg->layers[i] = IMG_LoadTexture(renderer, filename);
        if (!bg->layers[i]) {
            printf("Failed to load %s: %s\n", filename, IMG_GetError());
        }
        bg->speeds[i] = 0.5f + (float)i * 0.75f;
        bg->x_pos[i][0] = 0.0f;
        bg->x_pos[i][1] = (float)SCREEN_WIDTH;
    }
}

void draw_parallax(SDL_Renderer* renderer, ParallaxBackground* bg) {
    for (int i = 0; i < NUM_LAYERS; i++) {
        if (bg->layers[i]) {
            bg->x_pos[i][0] -= bg->speeds[i];
            bg->x_pos[i][1] -= bg->speeds[i];
            
            if (bg->x_pos[i][0] <= -SCREEN_WIDTH) bg->x_pos[i][0] = bg->x_pos[i][1] + SCREEN_WIDTH;
            if (bg->x_pos[i][1] <= -SCREEN_WIDTH) bg->x_pos[i][1] = bg->x_pos[i][0] + SCREEN_WIDTH;
            
            SDL_Rect dst1 = {(int)bg->x_pos[i][0], 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, bg->layers[i], NULL, &dst1);
            
            SDL_Rect dst2 = {(int)bg->x_pos[i][1], 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, bg->layers[i], NULL, &dst2);
        }
    }
}

void draw_decorated_button(SDL_Renderer* renderer, SDL_Rect* rect, const char* icon_type, DifficultyLevel difficulty, bool is_hovered) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_RenderFillRect(renderer, rect);
    
    SDL_Color icon_color = is_hovered ? (SDL_Color){255, 255, 0, 255} : (SDL_Color){255, 255, 255, 255};
    
    SDL_SetRenderDrawColor(renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
    SDL_RenderDrawRect(renderer, rect);

    if (strcmp(icon_type, "play") == 0) {
        int cx = rect->x + rect->w / 2;
        int cy = rect->y + rect->h / 2;
        SDL_Point vertices[3] = { {cx - 10, cy - 15}, {cx + 15, cy}, {cx - 10, cy + 15} };
        SDL_RenderDrawLines(renderer, vertices, 3);
    } else if (strcmp(icon_type, "quit") == 0) {
        draw_power_icon(renderer, rect, icon_color);
    } else if (strcmp(icon_type, "difficulty") == 0) {
        draw_difficulty_control(renderer, rect, difficulty, icon_color);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void draw_sound_icon_shape(SDL_Renderer* renderer, SDL_Rect* rect, bool sound_on, SDL_Color color) {
    int cx = rect->x + rect->w / 2;
    int cy = rect->y + rect->h / 2;
    int size = 15; // Ajusté pour mieux rentrer dans le bouton

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Base du haut-parleur
    SDL_Rect body = {cx - size / 2, cy - size / 4, size / 2, size / 2};
    SDL_RenderFillRect(renderer, &body);
    
    // Cône du haut-parleur
    SDL_Point triangle[4] = {
        {cx - size/2, cy - size/2}, {cx, cy - size},
        {cx, cy + size}, {cx - size/2, cy + size/2}
    };
    SDL_RenderDrawLines(renderer, triangle, 4);

    if (sound_on) {
        // Ondes sonores
        for (int i = 0; i < 2; i++) {
            int r_start = size + i * 4;
            SDL_RenderDrawLine(renderer, cx + r_start, cy - r_start, cx + r_start, cy + r_start);
        }
    } else {
        // Croix (barre de mute)
        SDL_RenderDrawLine(renderer, rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
        SDL_RenderDrawLine(renderer, rect->x, rect->y + rect->h, rect->x + rect->w, rect->y);
    }
}

void draw_sound_icon(SDL_Renderer* renderer, SDL_Rect* rect, bool sound_on, bool is_hovered) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_RenderFillRect(renderer, rect);
    
    SDL_Color icon_color = is_hovered ? (SDL_Color){255, 255, 0, 255} : (SDL_Color){255, 255, 255, 255};
    
    SDL_SetRenderDrawColor(renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
    SDL_RenderDrawRect(renderer, rect);
    
    draw_sound_icon_shape(renderer, rect, sound_on, icon_color);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}


void cleanup_drawing(ParallaxBackground* bg) {
    for (int i = 0; i < NUM_LAYERS; i++) {
        if (bg->layers[i]) {
            SDL_DestroyTexture(bg->layers[i]);
        }
    }
}