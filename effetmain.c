#include "effetSDL.h"
#include "bSDL.h"
#include <SDL2/SDL_rect.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>





int** createImage(int w, int h) {
    int** img = (int**) malloc(sizeof(int*) * w);
    for (int i = 0; i < w; i++) {
        img[i] = (int*) malloc(sizeof(int) * h);
    }
    return img;
}

void loadImage(int** img, int w, int h){
    for(int j = 0; j<h; j++){
        for(int i = 0; i<w; i++){
            //img[i][j] = i%(h/10) + j%(w/10);
            img[i][j] = (i+j)%9;
        }
    }
}

void afficheImage(int** img, int w, int h){
    for(int j = 0; j<h; j++){
        for(int i = 0; i<w; i++){
            printf("%d ", img[i][j]);
        }
        printf("\n");
    }
}

void free2D(int **tab, int w) {
    for (int i = 0; i < w; i++) {
        free(tab[i]);
    }
}



int** applique_zoom(int** originale, int org_w, int org_h, float alpha, Complex z_0, int couleur_par_defaut){
    Complex zp, z;
    int x,y,couleur;
    int** destination = createImage(org_w, org_h);
    for(int j = 0; j<org_h; j++){
        for(int i = 0; i<org_w; i++){
            zp = coordonnee_image_vers_complexe(i, j);
            z = zoom_inverse(zp, z_0, alpha);
            complexe_vers_coordonnee_image(z, &x, &y);
            if (x >= 0 && x < org_w && y >= 0 && y < org_h){
                couleur = originale[x][y];
            }else{
                couleur = couleur_par_defaut;
            }
            destination[i][j] = couleur;
        }
    }
    return destination;
}

int** applique_zoom_sur_zone(int** originale, int org_w, int org_h, Complex zone_z, int zone_w, int zone_h, float alpha, int couleur_par_defaut){
    Complex zp, z, z_0;

    //centre de zoom
    z_0.re = zone_z.re + zone_w/2.0;
    z_0.im = zone_z.im + zone_h/2.0;
    
    int x,y,couleur;
    int** destination = createImage(zone_w, zone_h);
    for(int j = 0; j<zone_h; j++){
        for(int i = 0; i<zone_w; i++){
            zp = coordonnee_image_vers_complexe((zone_w + zone_z.re - 1)+i, (zone_h + zone_z.im - 1)+j);
            z = zoom_inverse(zp, z_0, alpha);
            complexe_vers_coordonnee_image(z, &x, &y);
            if (x >= 0 && x < org_w && y >= 0 && y < org_h){
                couleur = originale[x][y];
            }else{
                couleur = couleur_par_defaut;
            }
            destination[i][j] = couleur;
        }
    }
    return destination;
}


int** applique_trans(int** originale, int org_w, int org_h, int x_t, int y_t, int couleur_par_defaut){
    Complex zp, z;
    int x,y,couleur;
    int** destination = createImage(org_w, org_h);
    for(int j = 0; j<org_h; j++){
        for(int i = 0; i<org_w; i++){
            zp = coordonnee_image_vers_complexe(i, j); //dest
            z = translation_inverse(zp, x_t, y_t);
            complexe_vers_coordonnee_image(z, &x, &y);
            if (x >= 0 && x < org_w && y >= 0 && y < org_h){
                couleur = originale[x][y];
            }else{
                couleur = couleur_par_defaut;
                printf("%d %d\n", i, j);
            }
            destination[i][j] = couleur;
        }
    }
    return destination;
}

int** applique_rotation(int** originale, int org_w, int org_h, Complex z_0, float angle, int couleur_par_defaut){

    Complex zp, z;
    int x,y,couleur;
    int** destination = createImage(org_w, org_h);
    for(int j = 0; j<org_h; j++){
        for(int i = 0; i<org_w; i++){
            zp = coordonnee_image_vers_complexe(i, j);
            z = rotation_img(zp, z_0, -angle);
            complexe_vers_coordonnee_image(z, &x, &y);
            if (x >= 0 && x < org_w && y >= 0 && y < org_h){
                couleur = originale[x][y];
            }else{
                couleur = couleur_par_defaut;
            }
            destination[i][j] = couleur;
        }
    }
    return destination;

}




int mainSDL(){

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
    float alpha = 1.6f;
    Complex z_0;
    z_0.re = surf->w/2.0f;
    z_0.im = surf->h/2.0f;  // centre
    //SDL_Surface *dest = apply_zoom(surf, alpha, z_0);
    // zoom partiel
    SDL_Rect z_zp = {100, 100, 500, 500};
    //SDL_Surface *dest = apply_zoom_sur_zone(surf, alpha, z_zp);
    
    //exemple translation
    //SDL_Surface *dest = apply_trans(surf, 100.0, 50.0);
    //exemple rotation
    //SDL_Surface *dest = apply_rotation(surf, 50, z_0);
    float Angle_max = 45;
    float d0 = 0, dmax = 100, d1 = 400;

    //SDL_Surface *dest = apply_rotation_d(surf, Angle_max, d0, d1, dmax, z_0);

    SDL_Surface *dest = apply_rotation_sur_zone(surf, Angle_max, z_zp);
    if (!dest) {
        printf("Erreur chargement image : %s\n", IMG_GetError());
        return 0;
    }
    SDL_Texture *text_dest = SDL_CreateTextureFromSurface(renderer, dest); //surf to text
    if (!text_dest) {
        printf("Erreur chargement texture : %s\n", IMG_GetError());
        return 0;
    }

    SDL_RenderClear(renderer);
    SDL_QueryTexture(text_dest, NULL, NULL, &source.w, &source.h); 
    SDL_RenderCopy(renderer, text_dest, &source, &source); //SDL_RenderCopy(renderer, text_dest, &source, &destination); //plein ecran
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
    
    SDL_FreeSurface(dest);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(text_dest);
    destroyAndQuit(&window, &renderer); 

    return 0;
}

/*
int mainSDL(){
    // ================================les Initialisations===================================
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
    // ==================================      end    ========================================

    float angle = 0.0f;
    float angle_max = 60.0f;
    float d0 = 200, d_max = 600, d1 = 1000;
    float angle_step = 1.0f; // combien d’angle ajouter par frame

    Uint32 last_time = SDL_GetTicks();
    int running = 1;

    while (running && angle <= angle_max) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        Uint32 now = SDL_GetTicks();
        if (now - last_time >= 33) { // ~30 fps
            last_time = now;

            // appliquer l’effet avec l’angle courant
            SDL_Surface* current = apply_rotation_d(surf, angle, d0, d1, d_max, z_0);
            if (!current) break;

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, current);
            SDL_FreeSurface(current);
            if (!texture) break;

            SDL_RenderClear(renderer);
            SDL_Rect src = {0, 0, surf->w, surf->h};
            SDL_Rect dst = window_dimensions;
            SDL_RenderCopy(renderer, texture, &src, &dst);
            SDL_RenderPresent(renderer);

            SDL_DestroyTexture(texture);

            // augmenter l’angle
            angle += angle_step;
        }
    }

}
*/


int main() {
    
    // example utilisation zoom sur le terminal
    int w=10,h=10;
    float alpha = 2;
    //Complex z_0 = {w/2,h/2};
    int** img_org = createImage(w, h);
    loadImage(img_org, w, h);

    /*
    // zoom ------------------
    int** img_des = applique_zoom(img_org, w, h, alpha, z_0, -1);
    afficheImage(img_org, w, h);
    printf("\n\n\n\n");
    afficheImage(img_des, (int) w, (int) h);
    free2D(img_des, w);
    
    // zoom paritel---------------
    Complex zone_z = {6, 6};
    int zone_w = 4;
    int zone_h = 4;
    z_0.re = (zone_z.re + 1 + zone_w)/2;
    z_0.im = (zone_z.im + 1 + zone_h)/2;
    int** img_des = applique_zoom_sur_zone(img_org, w, h, zone_z, zone_w, zone_h, alpha, -1);

    afficheImage(img_org, w, h);
    printf("\n\n\n\n");
    afficheImage(img_des, (int) zone_w, (int) zone_h);
    free2D(img_des, zone_w);
    
    // translate ---------------------------

    int tx=2, ty=2;
    int** img_des = applique_trans(img_org, w, h, tx, ty, -1);
    afficheImage(img_org, w, h);
    printf("\n\n\n\n");
    afficheImage(img_des, (int) w, (int) h);
    free2D(img_des, w);
    * /
    // rotation -----------------------
    Complex z_0 = {w/2,h/2};
    int** img_des = applique_rotation(img_org, w, h, z_0, 45, -1);
    afficheImage(img_org, w, h);
    printf("\n\n\n\n");
    afficheImage(img_des, (int) w, (int) h);
    free2D(img_des, w);
    
    free2D(img_org, w);
    free(img_org);
    free(img_des);
    */
    
    mainSDL();
    
}

