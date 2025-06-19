#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define SCALE 3
#define FRAME_COUNT 8
#define GRAVITY 1
#define JUMP_VELOCITY -20
#define GROUND_LEVEL 810

void end_sdl(char ok, const char* msg, SDL_Window* window, SDL_Renderer* renderer) {
    if (!ok) {
        SDL_Log("%s : %s\n", msg, SDL_GetError());
    }

    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    if (window != NULL) SDL_DestroyWindow(window);

    SDL_Quit();
    if (!ok) exit(EXIT_FAILURE);
}

SDL_Texture* load_texture_from_image(char* file_image_name, SDL_Window* window, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(file_image_name);
    if (surface == NULL) end_sdl(0, "Erreur chargement image", window, renderer);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == NULL) end_sdl(0, "Erreur création texture", window, renderer);

    return texture;
}

void chargerBg(SDL_Texture* texture, SDL_Rect window_dim, SDL_Renderer* renderer, int x, int y) {
    SDL_Rect src = {0}, dst = {0};
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    dst = window_dim;
    dst.x = x;
    dst.y = y;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

void chargerChev(SDL_Texture* texture, SDL_Window* window, SDL_Renderer* renderer, int posX, int posY, int frameIndex) {
    SDL_Rect src = {0}, dst = {0}, win_dim = {0};

    SDL_GetWindowSize(window, &win_dim.w, &win_dim.h);
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    int columns = 8, rows = 1;
    float zoom = 3.f;
    int frameW = src.w / columns;
    int frameH = src.h / rows;

    src.x = frameIndex * frameW;
    src.y = 0;
    src.w = frameW;
    src.h = frameH;

    dst.w = frameW * zoom;
    dst.h = frameH * zoom;
    dst.x = posX;
    dst.y = posY;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur initialisation SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }    
    SDL_Window* window = SDL_CreateWindow("fenetre", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) end_sdl(0, "Erreur création fenêtre", NULL, NULL);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) end_sdl(0, "Erreur création renderer", window, NULL);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_Rect window_dimensions = {0};
    SDL_GetWindowSize(window, &window_dimensions.w, &window_dimensions.h);

    int running = 1, frame = 0, frameIndex = 0;
    SDL_Event event;

    SDL_Texture* bg = load_texture_from_image("Background1.png", window, renderer);
    SDL_Texture* chev = load_texture_from_image("WALK.png", window, renderer);

    int x1 = 0, x2 = window_dimensions.w;

    // Position et vitesse du chevalier
    int chevalierX = 700;
    int chevalierY = GROUND_LEVEL;
    int velocityY = 0;
    bool isJumping = false;

    const int speed = 8;

    while (running && frame <= 30000) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = 0;
                else if (event.key.keysym.sym == SDLK_SPACE && !isJumping) {
                    velocityY = JUMP_VELOCITY;
                    isJumping = true;
                }
            }
        }

        // État des touches
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_LEFT]) {
            chevalierX -= speed;
        }
        if (keystates[SDL_SCANCODE_RIGHT]) {
            chevalierX += speed;
        }

        // Application de la gravité
        chevalierY += velocityY;
        if (chevalierY < GROUND_LEVEL) {
            velocityY += GRAVITY;
        } else {
            chevalierY = GROUND_LEVEL;
            velocityY = 0;
            isJumping = false;
        }

        SDL_RenderClear(renderer);

        // Arrière-plan en boucle
        chargerBg(bg, window_dimensions, renderer, x1, 0);
        chargerBg(bg, window_dimensions, renderer, x2, 0);

        // Animation chevalier
        chargerChev(chev, window, renderer, chevalierX, chevalierY, frameIndex % FRAME_COUNT);

        if (frame % 10 == 0) frameIndex++;
        frame++;

        x1 -= 1;
        x2 -= 1;
        if (x1 <= -window_dimensions.w) x1 = window_dimensions.w;
        if (x2 <= -window_dimensions.w) x2 = window_dimensions.w;

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(chev);
    SDL_DestroyTexture(bg);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
    return 0;
}
