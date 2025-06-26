#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Constantes ---
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SPEED = 3;
const int ANIMATION_FPS = 12; // Vitesse de l'animation
const int ANIMATION_FRAMES = 6; // Chaque animation a 6 images

// Enumération pour les directions
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

// Structure pour notre personnage (Player)
typedef struct {
    SDL_Rect screen_pos;
    Direction direction;
    int current_frame;
    Uint32 last_frame_time;
    bool is_moving;
} Player;

// Fonction de fin de SDL
void end_sdl(char ok, const char* msg, SDL_Window* window, SDL_Renderer* renderer) {
    if (!ok) {
        SDL_Log("%s : %s\n", msg, SDL_GetError());
    }
    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    if (window != NULL) SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    if (!ok) exit(EXIT_FAILURE);
}

// Chargement de texture
SDL_Texture* load_texture_from_image(const char* file_image_name, SDL_Window* window, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(file_image_name);
    if (surface == NULL) end_sdl(0, "Erreur chargement image", window, renderer);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == NULL) end_sdl(0, "Erreur création texture", window, renderer);
    return texture;
}

// Mise à jour du personnage
void update_player(Player* player) {
    if (player->is_moving) {
        // 1. Mettre à jour la position
        switch (player->direction) {
            case UP:    player->screen_pos.y -= PLAYER_SPEED; break;
            case DOWN:  player->screen_pos.y += PLAYER_SPEED; break;
            case LEFT:  player->screen_pos.x -= PLAYER_SPEED; break;
            case RIGHT: player->screen_pos.x += PLAYER_SPEED; break;
        }

        // 2. Mettre à jour l'animation (toujours 6 frames)
        if (SDL_GetTicks() - player->last_frame_time > (1000 / ANIMATION_FPS)) {
            player->current_frame++;
            if (player->current_frame >= ANIMATION_FRAMES) {
                player->current_frame = 0;
            }
            player->last_frame_time = SDL_GetTicks();
        }
    } else {
        player->current_frame = 0; // Personnage à l'arrêt, on affiche la 1ère frame
    }
}

// Dessin du personnage
void render_player(SDL_Renderer* renderer, Player* player, SDL_Texture* textures[], int frame_w, int frame_h) {
    // Choisir la bonne texture en fonction de la direction
    SDL_Texture* current_texture = textures[player->direction];
    
    // Découper la bonne frame dans la texture choisie
    SDL_Rect src = {
        .x = player->current_frame * frame_w,
        .y = 0, // Chaque animation est sur une seule ligne
        .w = frame_w,
        .h = frame_h
    };

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (player->direction == LEFT) {
        flip = SDL_FLIP_HORIZONTAL; // On retourne l'image pour aller à gauche
    }
    
    SDL_RenderCopyEx(renderer, current_texture, &src, &player->screen_pos, 0, NULL, flip);
}

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow("Nouveau Personnage", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // MODIFICATION MAJEURE : On charge 3 textures dans un tableau
    SDL_Texture* animation_textures[4]; // UP, DOWN, LEFT, RIGHT
    animation_textures[DOWN]  = load_texture_from_image("Carry_Run_Down-Sheet.png", window, renderer);
    animation_textures[RIGHT] = load_texture_from_image("Carry_Run_Side-Sheet.png", window, renderer);
    animation_textures[UP]    = load_texture_from_image("Carry_Run_Up-Sheet.png", window, renderer);
    animation_textures[LEFT]  = animation_textures[RIGHT]; // Pour la gauche, on réutilise la texture de droite

    // Calculer la taille d'une seule frame (on suppose que toutes les images ont la même taille de frame)
    int texture_w, texture_h;
    SDL_QueryTexture(animation_textures[DOWN], NULL, NULL, &texture_w, &texture_h);
    int frame_w = texture_w / ANIMATION_FRAMES;
    int frame_h = texture_h;

    // Initialisation du joueur
    Player player;
    player.screen_pos.x = (SCREEN_WIDTH - frame_w * 2) / 2;
    player.screen_pos.y = (SCREEN_HEIGHT - frame_h * 2) / 2;
    player.screen_pos.w = frame_w * 2.0; // On peut zoomer un peu
    player.screen_pos.h = frame_h * 2.0;
    player.direction = DOWN;
    player.current_frame = 0;
    player.last_frame_time = 0;
    player.is_moving = false;

    // Boucle de jeu
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;
        }

        const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
        Direction old_direction = player.direction;

        player.is_moving = false;
        if (keyboard_state[SDL_SCANCODE_RIGHT]) {
            player.direction = RIGHT;
            player.is_moving = true;
        } else if (keyboard_state[SDL_SCANCODE_LEFT]) {
            player.direction = LEFT;
            player.is_moving = true;
        } else if (keyboard_state[SDL_SCANCODE_DOWN]) {
            player.direction = DOWN;
            player.is_moving = true;
        } else if (keyboard_state[SDL_SCANCODE_UP]) {
            player.direction = UP;
            player.is_moving = true;
        }
        
        if (player.direction != old_direction) {
            player.current_frame = 0; // Réinitialise l'animation si la direction change
        }

        update_player(&player);

        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255); // Fond sombre
        SDL_RenderClear(renderer);
        render_player(renderer, &player, animation_textures, frame_w, frame_h);
        SDL_RenderPresent(renderer);
    }

    // Nettoyage
    SDL_DestroyTexture(animation_textures[DOWN]);
    SDL_DestroyTexture(animation_textures[RIGHT]); // Détruire RIGHT détruit aussi LEFT car ils pointent sur la même chose
    SDL_DestroyTexture(animation_textures[UP]);
    end_sdl(1, "Fin normale", window, renderer);
    return EXIT_SUCCESS;
}