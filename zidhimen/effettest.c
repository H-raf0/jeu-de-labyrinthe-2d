#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float re, im;
} Complex;

Complex translation_inverse(Complex z_inv, int dx, int dy) {
    Complex z;
    z.re = z_inv.re - dx;
    z.im = z_inv.im - dy;
    return z;
}

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

void loadImage(int** img, int w, int h) {
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            img[i][j] = (i + j) % 9;  // Motif simple pour voir la translation
        }
    }
}

void afficheImage(int** img, int w, int h) {
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            printf("%2d ", img[i][j]);
        }
        printf("\n");
    }
}

void free2D(int **tab, int w) {
    for (int i = 0; i < w; i++) {
        free(tab[i]);
    }
    free(tab);
}

int** applique_translation(int** originale, int org_w, int org_h, int dx, int dy, int couleur_par_defaut) {
    int des_w = org_w;
    int des_h = org_h;
    int** destination = createImage(des_w, des_h);
    
    for(int j = 0; j < des_h; j++) {
        for(int i = 0; i < des_w; i++) {
            // Calcul des coordonnées originales
            int x = i - dx;
            int y = j - dy;
            
            if (x >= 0 && x < org_w && y >= 0 && y < org_h) {
                destination[i][j] = originale[x][y];
            } else {
                destination[i][j] = couleur_par_defaut;
            }
        }
    }
    return destination;
}

int main() {
    int w = 10, h = 10;
    int** img_org = createImage(w, h);
    loadImage(img_org, w, h);

    printf("Image originale:\n");
    afficheImage(img_org, w, h);

    int dx = 2, dy = 1;  // Décalage de 2 pixels vers la droite et 1 vers le bas
    printf("\nImage après translation (dx=%d, dy=%d):\n", dx, dy);
    int** img_des = applique_translation(img_org, w, h, dx, dy, -1);
    afficheImage(img_des, w, h);
    free2D(img_org, w);
    free2D(img_des, w);

    return 0;
}