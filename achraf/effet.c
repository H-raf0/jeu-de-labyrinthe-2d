#include "bSDL.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>

#define ZOOM_FACTOR 2.0f  // facteur a dans f(z) = a*z

typedef struct {
    float re, im;
} Complex;


Complex coordonnee_image_vers_complexe(int x, int y) {
    Complex z = {
        .re = x,
        .im = y
    };
    return z;
}

void complexe_vers_coordonnee_image(Complex z, int* x, int* y) {
    *x = (int) round(z.re);
    *y = (int) round(z.im);
}

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

Complex zoom_inverse(Complex z_prime, Complex z_0, float a) {
    Complex z = {((z_prime.re - z_0.re) / a) + z_0.re, ((z_prime.im - z_0.im) / a) + z_0.im};
    return z;
}

int** applique_zoom(int** originale, int org_w, int org_h, float alpha, Complex z_0, int couleur_par_defaut){
    Complex zp, z;
    int x,y,couleur;
    int des_w = (int) org_w*alpha;
    int des_h = (int) org_h*alpha;
    int** destination = createImage(des_w, des_h);
    for(int j = 0; j<des_h; j++){
        for(int i = 0; i<des_w; i++){
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



int main() {
    int w=10,h=10;
    float alpha = 0.8;
    Complex z_0 = {0,0};
    int** img_org = createImage(w, h);
    loadImage(img_org, w, h);

    int** img_des = applique_zoom(img_org, w, h, alpha, z_0, -1);

    afficheImage(img_org, w, h);
    printf("\n\n\n\n");
    afficheImage(img_des, (int) w*alpha, (int) h*alpha);



    free2D(img_org, w);
    free(img_org);
    free2D(img_des, w*alpha);
    free(img_des);

    return 0;
}
