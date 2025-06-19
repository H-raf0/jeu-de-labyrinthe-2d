#include "bSDL.h"
#include "effetSDL.h"
#include <SDL2/SDL_rect.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>



int main(int argc, char * argv[]) {
    // ================================ Initialization ===================================
    char img_loc[50];
    if(argc < 2) {
        printf("Usage: ./exec \"img.png\"\n");
        return 0;
    } else {
        strcpy(img_loc, argv[1]);
    }

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect window_dimensions = {0};
    InitialisationSDL(&window, &renderer, &window_dimensions);
    SDL_GetWindowSize(window, &window_dimensions.w, &window_dimensions.h);
    
    SDL_Surface *original_surf = IMG_Load(img_loc);
    if (!original_surf) {
        printf("Image loading error: %s\n", IMG_GetError());
        return 0;
    }

    // Create a working copy of the original surface
    SDL_Surface *current_surf = SDL_ConvertSurface(original_surf, original_surf->format, 0);
    if (!current_surf) {
        printf("Surface copy error: %s\n", SDL_GetError());
        SDL_FreeSurface(original_surf);
        return 0;
    }

    SDL_Rect src = {0, 0, current_surf->w, current_surf->h};
    SDL_Rect dst = window_dimensions;

    float angle = 0.0f, alpha = 1.0f;
    //float angle_max = 60.0f;
    float d0 = 200, d_max = 600, d1 = 1000;
    float angle_step = 1.0f, alpha_step = 0.05f;
    float x_t = 0.0f, y_t = 0.0f, t_step = 5.0f;
    Complex z_0 = {current_surf->w / 2.0f, current_surf->h / 2.0f}; // center

    int running = 1;
    SDL_Event event;
    int transformed = 0; // 1 updated à no

    // Initial texture from current_surf
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, current_surf);
    if (!texture) {
        printf("Texture creation error: %s\n", SDL_GetError());
        SDL_FreeSurface(original_surf);
        SDL_FreeSurface(current_surf);
        return 1;
    }

    // ================================== Main Loop ========================================
    while (running) {
        transformed = 0; // Reset each frame

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    z_0.re = event.button.x;
                    z_0.im = event.button.y;
                    printf("New transformation center: %.1f, %.1f\n", z_0.re, z_0.im);
                    break;
                case SDL_KEYDOWN: {
                    SDL_Surface* new_surf = NULL;
                    
                    switch (event.key.keysym.sym) {
                        case SDLK_d:
                            printf("Right translation\n");
                            new_surf = apply_trans(current_surf, t_step, 0);
                            x_t += t_step;
                            break;
                        case SDLK_q:
                            printf("Left translation\n");
                            new_surf = apply_trans(current_surf, -t_step, 0);
                            x_t -= t_step;
                            break;
                        case SDLK_z:
                            printf("Upward translation\n");
                            new_surf = apply_trans(current_surf, 0, -t_step);
                            y_t -= t_step;
                            break;
                        case SDLK_s:
                            printf("Downward translation\n");
                            new_surf = apply_trans(current_surf, 0, t_step);
                            y_t += t_step;
                            break;
                        case SDLK_RIGHT:
                            printf("Counter-clockwise rotation\n");
                            new_surf = apply_rotation_d(current_surf, angle_step, d0, d1, d_max, z_0);
                            angle += angle_step;
                            break;
                        case SDLK_LEFT:
                            printf("Clockwise rotation\n");
                            new_surf = apply_rotation_d(current_surf, -angle_step, d0, d1, d_max, z_0);
                            angle -= angle_step;
                            break;
                        case SDLK_UP:
                            printf("Zoom in\n");
                            new_surf = apply_zoom(current_surf, 1.0f + alpha_step, z_0);
                            alpha *= (1.0f + alpha_step);
                            break;
                        case SDLK_DOWN:
                            printf("Zoom out\n");
                            new_surf = apply_zoom(current_surf, 1.0f / (1.0f + alpha_step), z_0);
                            alpha /= (1.0f + alpha_step);
                            break;
                        case SDLK_SPACE:
                            printf("Reset transformations\n");
                            // Reset parameters
                            x_t = 0;
                            y_t = 0;
                            angle = 0;
                            alpha = 1.0f;
                            z_0.re = original_surf->w / 2.0f;
                            z_0.im = original_surf->h / 2.0f;
                            // Restore original image
                            SDL_FreeSurface(current_surf);
                            current_surf = SDL_ConvertSurface(original_surf, original_surf->format, 0);
                            transformed = 1;
                            break;
                    }

                    // Update current surface if transformation occurred
                    if (new_surf) {
                        SDL_FreeSurface(current_surf);
                        current_surf = new_surf;
                        transformed = 1;
                    }
                    break;
                }
            }
        }

        // Update texture if surface was modified
        if (transformed) {
            SDL_DestroyTexture(texture);
            texture = SDL_CreateTextureFromSurface(renderer, current_surf);
            if (!texture) {
                printf("Texture update error: %s\n", SDL_GetError());
                running = 0;
            }
        }

        // Rendering
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, &src, &dst);
        SDL_RenderPresent(renderer);

        SDL_Delay(5);
    }

    // Cleanup
    SDL_FreeSurface(original_surf);
    SDL_FreeSurface(current_surf);
    if (texture) SDL_DestroyTexture(texture);
    destroyAndQuit(&window, &renderer);

    return 0;
}








/*
int main(){

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect window_dimensions = {0};
    InitialisationSDL(&window, &renderer, &window_dimensions);
    SDL_Rect source = {0}, destination = {0};
    SDL_GetWindowSize(window, &window_dimensions.w, &window_dimensions.h);
    destination = window_dimensions;



    SDL_Surface *surf = IMG_Load("img1.png");
    if (!surf) {
        printf("Erreur chargement image : %s\n", IMG_GetError());
        return 0;
    }
    
    // exemple de zoom totale
    float alpha = 2.0f;
    Complex z_0;
    z_0.re = surf->w/2.0f;
    z_0.im = surf->h/2.0f;  // centre
    SDL_Surface *dest = apply_zoom(surf, alpha, z_0);
    if (!surf) {
        printf("Erreur chargement text : %s\n", IMG_GetError());
        return 0;
    }
    
    // zoom partiel
    SDL_Rect z_zp = {100, 100, 500, 500};
    //apply_zoom_sur_zone(surf, alpha, z_zp);
    
    //exemple translation
    //SDL_Surface *dest = apply_trans(surf, 100.0, 50.0);
    //exemple rotation
    //SDL_Surface *dest = apply_rotation(surf, 50, z_0);
    float Angle_max = 45;
    float d0 = 0, dmax = 100, d1 = 400;

    //SDL_Surface *dest = apply_rotation_d(surf, Angle_max, d0, d1, dmax, z_0);

    //SDL_Surface *dest = apply_rotation_sur_zone(surf, Angle_max, z_zp);

    SDL_Texture *text_dest = SDL_CreateTextureFromSurface(renderer, dest);
    SDL_FreeSurface(dest);


    SDL_RenderClear(renderer);
    SDL_QueryTexture(text_dest, NULL, NULL, &source.w, &source.h); 
    SDL_RenderCopy(renderer, text_dest, &source, &destination); //SDL_RenderCopy(renderer, text_dest, &source, &destination); //plein ecran
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
    
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(text_dest);
    destroyAndQuit(&window, &renderer); 

    return 0;
}
*/
/*
int main(){
    // ================================les Initialisations===================================
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect window_dimensions = {0};
    InitialisationSDL(&window, &renderer, &window_dimensions);
    SDL_GetWindowSize(window, &window_dimensions.w, &window_dimensions.h);
    SDL_Surface *surf = IMG_Load("img1.png");
    if (!surf) {
        printf("Erreur chargement image : %s\n", IMG_GetError());
        return 0;
    }
    // ==================================      end    ========================================

    SDL_Rect src = {0, 0, surf->w, surf->h};
    SDL_Rect dst = window_dimensions;

    float angle = 0.0f, alpha =1.0f;
    float angle_max = 60.0f;
    float d0 = 200, d_max = 600, d1 = 1000;
    float angle_step = 1.0f, alpha_step=.05f; // combien d’angle/zoom ajouter par frame
    float x_t=0.0f, x_t_step=5.0f;
    Complex z_0; // centre
    z_0.re = surf->w/2.0f;
    z_0.im = surf->h/2.0f; 

    SDL_Rect zone = {100, 100, 500, 500};

    Uint32 last_time = SDL_GetTicks(); //milli sec since start
    int running = 1;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        Uint32 now = SDL_GetTicks();
        if (now - last_time >= 33) { //1000/30 =~33fps
            last_time = now;

            SDL_Surface *current;
            if(now/1000 <= 1){ //3sec passed
                current = apply_zoom(surf, alpha, z_0);
                alpha += alpha_step;
            }else if (now/1000 <= 2){
                current = apply_zoom_sur_zone(surf, alpha, zone);
                alpha += alpha_step;
            }else if (now/1000 <= 3){
                current = apply_rotation_sur_zone(surf, angle, zone);
                angle += angle_step;
            }else if (now/1000 <= 4){
                current = apply_rotation(surf, angle, z_0);
                angle += angle_step;
            }else if (now/1000 <= 5){
                current = apply_trans(surf, x_t, 0);
                x_t+=x_t_step;
            }else if (now/1000 <= 6){
                current = apply_rotation_d(surf, angle, d0, d1, d_max, z_0);
                angle += angle_step;
            }else{
                running = 0;
            }
            // appliquer l’effet avec l’angle courant
            
            if (!current) break;

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, current);
            SDL_FreeSurface(current);
            if (!texture) break;

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, &src, &dst);
            SDL_RenderPresent(renderer);

            SDL_DestroyTexture(texture);

            
        }
    }
    return 0;

}*/


