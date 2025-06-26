#include <SDL2/SDL.h>
#include <stdbool.h>

#define WIDTH 800
#define HEIGHT 600

void fadeIn(SDL_Renderer *renderer, SDL_Texture *texture) {
    SDL_Rect dst = {0, 0, WIDTH, HEIGHT};

    for (int alpha = 0; alpha <= 255; alpha += 5) {
        SDL_SetTextureAlphaMod(texture, alpha);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &dst);
        SDL_RenderPresent(renderer);
        SDL_Delay(30);
    }
}

SDL_Texture* loadBMPTexture(SDL_Renderer *renderer, const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        SDL_Log("Erreur chargement image %s : %s", path, SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Congratulations", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *congratsTex = loadBMPTexture(renderer, "congratulations.bmp");
    if (!congratsTex) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    fadeIn(renderer, congratsTex);
    SDL_Delay(2000);

    SDL_DestroyTexture(congratsTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
