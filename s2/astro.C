/*
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define WIDTH 640
#define HEIGHT 480
#define FRAME_WIDTH 16
#define FRAME_HEIGHT 16
#define SCALE 4
#define FRAME_COUNT 6

typedef enum {
    NONE,
    DOWN,
    LEFT,
    RIGHT,
    UP
} Direction;

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("Erreur chargement image: %s", IMG_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Robot Animation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* spriteSheet = loadTexture(renderer, "CosmicLilac_AnimatedSpriteSheet.png");
    if (!spriteSheet) {
        SDL_Quit();
        return 1;
    }

    int x = WIDTH / 2;
    int y = HEIGHT / 2;
    int frame = 0;
    Uint32 lastFrameTime = 0;
    Direction dir = NONE;

    bool running = true;
    SDL_Event event;

    while (running) {
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        Direction newDir = NONE;

        if (keystate[SDL_SCANCODE_DOWN]) {
            y += 2;
            newDir = DOWN;
        } else if (keystate[SDL_SCANCODE_UP]) {
            y -= 2;
            newDir = UP;
        } else if (keystate[SDL_SCANCODE_RIGHT]) {
            x += 2;
            newDir = RIGHT;
        } else if (keystate[SDL_SCANCODE_LEFT]) {
            x -= 2;
            newDir = LEFT;
        }

        if (newDir != NONE) {
            dir = newDir;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Animation frame avancée
        Uint32 now = SDL_GetTicks();
        if (now - lastFrameTime > 120) {
            frame = (frame + 1) % FRAME_COUNT;
            lastFrameTime = now;
        }

        int row = 0;
        SDL_RendererFlip flip = SDL_FLIP_NONE;

        switch (dir) {
            case DOWN:  row = 2; break;
            case RIGHT: row = 3; flip = SDL_FLIP_NONE; break;
            case LEFT:  row = 3; flip = SDL_FLIP_HORIZONTAL; break;
            case UP:    row = 4; break;
            default:    frame = 0; break;
        }

        SDL_Rect src = {
            .x = frame * FRAME_WIDTH,
            .y = row * FRAME_HEIGHT,
            .w = FRAME_WIDTH,
            .h = FRAME_HEIGHT
        };

        SDL_Rect dst = {
            .x = x,
            .y = y,
            .w = FRAME_WIDTH * SCALE,
            .h = FRAME_HEIGHT * SCALE
        };

        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopyEx(renderer, spriteSheet, &src, &dst, 0, NULL, flip);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(spriteSheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define WIDTH 640
#define HEIGHT 480
#define FRAME_WIDTH 16
#define FRAME_HEIGHT 16
#define SCALE 4
#define FRAME_COUNT 6

typedef enum {
    NONE,
    DOWN,
    LEFT,
    RIGHT,
    UP
} Direction;

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("Erreur chargement image: %s", IMG_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Robot Animation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* spriteSheet = loadTexture(renderer, "CosmicLilac_AnimatedSpriteSheet.png");
    if (!spriteSheet) {
        SDL_Quit();
        return 1;
    }

    int x = WIDTH / 2;
    int y = HEIGHT / 2;
    int frame = 0;
    Uint32 lastFrameTime = 0;
    Direction dir = NONE;

    bool running = true;
    SDL_Event event;

    while (running) {
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        Direction newDir = NONE;

        if (keystate[SDL_SCANCODE_DOWN]) {
            y += 2;
            newDir = DOWN;
        } else if (keystate[SDL_SCANCODE_UP]) {
            y -= 2;
            newDir = UP;
        } else if (keystate[SDL_SCANCODE_RIGHT]) {
            x += 2;
            newDir = RIGHT;
        } else if (keystate[SDL_SCANCODE_LEFT]) {
            x -= 2;
            newDir = LEFT;
        }

        if (newDir != NONE) {
            dir = newDir;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        Uint32 now = SDL_GetTicks();

        int row = 0;
        SDL_RendererFlip flip = SDL_FLIP_NONE;

        if (newDir == NONE) {
            // Rester sur ligne 2, frame 0 (idle)
            row = 1; // ligne 2 = index 1
            frame = 0;
        } else {
            // Animation en mouvement
            if (now - lastFrameTime > 120) {
                frame = (frame + 1) % FRAME_COUNT;
                lastFrameTime = now;
            }

            switch (dir) {
                case DOWN:  row = 2; break;
                case RIGHT: row = 3; flip = SDL_FLIP_NONE; break;
                case LEFT:  row = 3; flip = SDL_FLIP_HORIZONTAL; break;
                case UP:    row = 4; break;
                default:    row = 1; frame = 0; break;
            }
        }

        SDL_Rect src = {
            .x = frame * FRAME_WIDTH,
            .y = row * FRAME_HEIGHT,
            .w = FRAME_WIDTH,
            .h = FRAME_HEIGHT
        };

        SDL_Rect dst = {
            .x = x,
            .y = y,
            .w = FRAME_WIDTH * SCALE,
            .h = FRAME_HEIGHT * SCALE
        };

        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopyEx(renderer, spriteSheet, &src, &dst, 0, NULL, flip);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(spriteSheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define WIDTH 640
#define HEIGHT 480
#define FRAME_WIDTH 16
#define FRAME_HEIGHT 16
#define SCALE 4
#define FRAME_COUNT 6

typedef enum {
    NONE,
    DOWN,
    LEFT,
    RIGHT,
    UP
} Direction;

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("Erreur chargement image: %s", IMG_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Robot Animation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* spriteSheet = loadTexture(renderer, "CosmicLilac_AnimatedSpriteSheet.png");
    if (!spriteSheet) {
        SDL_Quit();
        return 1;
    }

    int x = WIDTH / 2;
    int y = HEIGHT / 2;
    int frame = 0;
    Uint32 lastFrameTime = 0;
    Direction dir = DOWN;
    bool isMoving = false;

    bool running = true;
    SDL_Event event;

    while (running) {
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        Direction newDir = NONE;
        isMoving = false;

        if (keystate[SDL_SCANCODE_DOWN]) {
            y += 2;
            newDir = DOWN;
            isMoving = true;
        } else if (keystate[SDL_SCANCODE_UP]) {
            y -= 2;
            newDir = UP;
            isMoving = true;
        } else if (keystate[SDL_SCANCODE_RIGHT]) {
            x += 2;
            newDir = RIGHT;
            isMoving = true;
        } else if (keystate[SDL_SCANCODE_LEFT]) {
            x -= 2;
            newDir = LEFT;
            isMoving = true;
        }

        if (isMoving && newDir != NONE) {
            dir = newDir;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        Uint32 now = SDL_GetTicks();

        // Avancer animation si en mouvement
        if (isMoving) {
            if (now - lastFrameTime > 120) {
                frame = (frame + 1) % FRAME_COUNT;
                lastFrameTime = now;
            }
        } else {
            frame = 0; // idle frame
        }

        int row = 1; // ligne idle par défaut
        SDL_RendererFlip flip = SDL_FLIP_NONE;

        if (isMoving) {
            switch (dir) {
                case DOWN:  row = 2; break;
                case RIGHT: row = 3; flip = SDL_FLIP_NONE; break;
                case LEFT:  row = 3; flip = SDL_FLIP_HORIZONTAL; break;
                case UP:    row = 4; break;
                default:    row = 1; break;
            }
        }

        SDL_Rect src = {
            .x = frame * FRAME_WIDTH,
            .y = row * FRAME_HEIGHT,
            .w = FRAME_WIDTH,
            .h = FRAME_HEIGHT
        };

        SDL_Rect dst = {
            .x = x,
            .y = y,
            .w = FRAME_WIDTH * SCALE,
            .h = FRAME_HEIGHT * SCALE
        };

        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopyEx(renderer, spriteSheet, &src, &dst, 0, NULL, flip);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(spriteSheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define WIDTH 640
#define HEIGHT 480
#define FRAME_WIDTH 16
#define FRAME_HEIGHT 16
#define SCALE 4
#define FRAME_COUNT 6

typedef enum {
    NONE,
    DOWN,
    LEFT,
    RIGHT,
    UP
} Direction;

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("Erreur chargement image: %s", IMG_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Robot Animation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* spriteSheet = loadTexture(renderer, "CosmicLilac_AnimatedSpriteSheet.png");
    if (!spriteSheet) {
        SDL_Quit();
        return 1;
    }

    int x = WIDTH / 2;
    int y = HEIGHT / 2;
    int frame = 0;
    Uint32 lastFrameTime = 0;
    Direction dir = DOWN;
    bool isMoving = false;

    bool running = true;
    SDL_Event event;

    while (running) {
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        Direction newDir = NONE;
        isMoving = false;

        if (keystate[SDL_SCANCODE_DOWN]) {
            y += 2;
            newDir = DOWN;
            isMoving = true;
        } else if (keystate[SDL_SCANCODE_UP]) {
            y -= 2;
            newDir = UP;
            isMoving = true;
        } else if (keystate[SDL_SCANCODE_RIGHT]) {
            x += 2;
            newDir = RIGHT;
            isMoving = true;
        } else if (keystate[SDL_SCANCODE_LEFT]) {
            x -= 2;
            newDir = LEFT;
            isMoving = true;
        }

        if (isMoving && newDir != NONE) {
            dir = newDir;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        int row;
        SDL_RendererFlip flip = SDL_FLIP_NONE;

        if (isMoving) {
            // Animer si déplacement
            Uint32 now = SDL_GetTicks();
            if (now - lastFrameTime > 120) {
                frame = (frame + 1) % FRAME_COUNT;
                lastFrameTime = now;
            }

            switch (dir) {
                case DOWN:  row = 2; break;
                case RIGHT: row = 3; flip = SDL_FLIP_NONE; break;
                case LEFT:  row = 3; flip = SDL_FLIP_HORIZONTAL; break;
                case UP:    row = 4; break;
                default:    row = 2; break;
            }
        } else {
            // Pas de mouvement → image fixe (ligne 2, frame 0)
            row = 1;   // Ligne d'index 1 (2e ligne du sprite)
            frame = 0; // Frame fixe
        }

        SDL_Rect src = {
            .x = frame * FRAME_WIDTH,
            .y = row * FRAME_HEIGHT,
            .w = FRAME_WIDTH,
            .h = FRAME_HEIGHT
        };

        SDL_Rect dst = {
            .x = x,
            .y = y,
            .w = FRAME_WIDTH * SCALE,
            .h = FRAME_HEIGHT * SCALE
        };

        SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopyEx(renderer, spriteSheet, &src, &dst, 0, NULL, flip);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(spriteSheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
