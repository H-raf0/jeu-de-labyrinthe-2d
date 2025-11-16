#ifndef BSDL_H
#define BSDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define COUCHES_NB 5

void end_sdl(char ok, const char* msg, SDL_Window* window, SDL_Renderer* renderer);
SDL_Texture* load_texture_from_image(char* file_image_name, SDL_Window* window, SDL_Renderer* renderer);
void ShowLayer(SDL_Texture* texture, SDL_Rect window_dim, SDL_Renderer* renderer, int x, int y);
void CreateDragon(SDL_Texture* texture, SDL_Window* window, SDL_Renderer* renderer, int posX, int posY, int frameIndex);
void InitialisationSDL(SDL_Window** window, SDL_Renderer** renderer, SDL_Rect* window_dimensions);
SDL_Texture** chargerCouche(SDL_Window* window, SDL_Renderer* renderer);
SDL_Texture* chargerDragon(SDL_Window* window, SDL_Renderer* renderer);
void destroyLayersAndDragon(SDL_Texture** layers, SDL_Texture** dragonTex);
void destroyAndQuit(SDL_Window** window, SDL_Renderer** renderer);

#endif