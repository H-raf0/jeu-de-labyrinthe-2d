#include "effetSDL.h"
#include "bSDL.h"
#include <stdio.h>
#include <stdlib.h>





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

int** applique_zoom_sur_zone(int** originale, int org_w, int org_h, Complex zone_z, int zone_w, int zone_h, float alpha, Complex z_0, int couleur_par_defaut){
    Complex zp, z;
    int x,y,couleur;
    int** destination = createImage(org_w, org_h);
    for(int j = 0; j<org_h; j++){
        for(int i = 0; i<org_w; i++){
            zp = coordonnee_image_vers_complexe(i, j);
            if(i>= zone_z.re && i <= zone_z.re+zone_w && j>= zone_z.im && j <= zone_z.im+zone_h){
                z = zoom_inverse(zp, z_0, alpha);
            } 
            else{
                z.re = zp.re;
                z.im = zp.im;
            }
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


    float alpha = 2;
    SDL_Surface *surf = IMG_Load("img.png");
    Complex z_0;
    // exemple de zoom totale
    z_0.re = surf->w/2.0f;
    z_0.im = surf->h/2.0f;  // centre
    SDL_Surface *dest = apply_zoom(surf, alpha, z_0);
    SDL_Texture *text_dest = SDL_CreateTextureFromSurface(renderer, dest); //surf to text


    SDL_QueryTexture(text_dest, NULL, NULL, &source.w, &source.h); 
    SDL_RenderCopy(renderer, text_dest, &source, &destination);
    SDL_RenderPresent(renderer);
    SDL_Delay(2000);
    
    SDL_FreeSurface(dest);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(text_dest);
    destroyAndQuit(&window, &renderer); 

    return 0;
}

int main() {
    /*  
    // example utilisation zoom sur le terminal
    int w=10,h=10;
    float alpha = 2;
    Complex z_0 = {2.5,2.5}, zone_z = {2, 2};
    int** img_org = createImage(w, h);
    loadImage(img_org, w, h);

    //int** img_des = applique_zoom(img_org, w, h, alpha, z_0, -1);
    int** img_des = applique_zoom_sur_zone(img_org, w, h, zone_z, 3, 3, alpha, z_0, -1);

    afficheImage(img_org, w, h);
    printf("\n\n\n\n");
    afficheImage(img_des, (int) w, (int) h);

    free2D(img_org, w);
    free(img_org);
    free2D(img_des, w);
    free(img_des);

    */
    mainSDL();
    
}

