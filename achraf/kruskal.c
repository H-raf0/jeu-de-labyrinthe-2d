#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>


#define TUILE_TAILLE 16 // Taille des tuiles dans l'image PNG (par exemple 16x16)

#define TAILLE_CELLULE 50 // Taille graphique d'une cellule dans SDL

// Structure représentant une partition (ensemble disjoint)
typedef struct {
    int *parent;
    int *rang;
    int taille;
} partition;

// Structure représentant une arête entre deux cellules
typedef struct {
    int u, v; // les deux cellules connectées
} arete;


SDL_Rect src_murs[16] = {
    {1 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 0 : rien
    {1 * TUILE_TAILLE, 0 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 1 : haut
    {2 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 2 : droite
    {2 * TUILE_TAILLE, 0 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 3 : haut + droite
    {1 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 4 : bas
    {7 * TUILE_TAILLE, 4 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 5 : haut + bas
    {2 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 6 : droite + bas
    {8 * TUILE_TAILLE, 4 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 7 : haut + droite + bas
    {0 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 8 : gauche
    {0 * TUILE_TAILLE, 0 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 9 : haut + gauche
    {8 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 10 : droite + gauche
    {8 * TUILE_TAILLE, 1 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 11 : haut + droite + gauche
    {0 * TUILE_TAILLE, 2 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 12 : bas + gauche
    {6 * TUILE_TAILLE, 4 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 13 : haut + bas + gauche
    {8 * TUILE_TAILLE, 3 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}, // 14 : droite + bas + gauche
    {7 * TUILE_TAILLE, 3 * TUILE_TAILLE, TUILE_TAILLE, TUILE_TAILLE}  // 15 : tous les murs
};


// Initialise une partition de taille 'total'
void init_partition(partition* p, int total) {
    p->taille = total;
    p->parent = malloc(sizeof(int) * total);
    p->rang = malloc(sizeof(int) * total);
    if (!p->parent || !p->rang) {
        fprintf(stderr, "Allocation mémoire impossible pour partition\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < total; i++) {
        p->parent[i] = i;
        p->rang[i] = 0;
    }
}

// Libère la mémoire d'une partition
void free_partition(partition* p) {
    free(p->parent);
    free(p->rang);
}

// Renvoie l'identifiant de la classe de l'élément i (avec compression de chemin)
int recuperer_classe(partition* p, int i) {
    if (p->parent[i] != i)
        p->parent[i] = recuperer_classe(p, p->parent[i]);
    return p->parent[i];
}

// Fusionne les classes des éléments i et j
int fusion(partition* p, int i, int j) {
    int ri = recuperer_classe(p, i);
    int rj = recuperer_classe(p, j);
    if (ri != rj) {
        if (p->rang[ri] < p->rang[rj]) {
            p->parent[ri] = rj;
        } else if (p->rang[ri] > p->rang[rj]) {
            p->parent[rj] = ri;
        } else {
            p->parent[rj] = ri;
            p->rang[ri]++;
        }
        return 1;
    }
    return 0;
}

// Mélange les arêtes aléatoirement (Fisher-Yates)
void fisher_yates(arete G[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        arete tmp = G[i];
        G[i] = G[j];
        G[j] = tmp;
    }
}

// Génère une grille vide avec toutes les arêtes possibles verticales et horizontales
int generation_grille_vide(arete **G_ptr, int lignes, int colonnes) {
    int max_aretes = 2 * lignes * colonnes - lignes - colonnes; // (n-1) * m + (m-1) * n = 2 * n * m - m - n
    arete *G = malloc(sizeof(arete) * max_aretes); // tab de tous les aretes possibles
    if (!G) {
        fprintf(stderr, "Allocation mémoire impossible pour arêtes\n");
        exit(EXIT_FAILURE);
    }
    int compteur = 0;
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int i = y * colonnes + x;  // indice du noeud de coord x,y
            if (y + 1 < lignes) G[compteur++] = (arete){i, (y + 1) * colonnes + x}; // si != dernier ligne alors ajoute l'arete vers le bas
            if (x + 1 < colonnes) G[compteur++] = (arete){i, y * colonnes + (x + 1)}; // si != dernier colonne alors ajoute l'arete vers le droite
        }
    }
    *G_ptr = G;
    return compteur; // nombre des aretes
}

// Construit un arbre couvrant minimal à partir des arêtes (kruskal)
void construire_arbre_couvrant(arete G[], int nb_aretes, arete *arbre, int* nb_arbre, int nb_cellules) {
    partition p;
    init_partition(&p, nb_cellules);
    *nb_arbre = 0;
    for (int i = 0; i < nb_aretes; i++) {
        if (fusion(&p, G[i].u, G[i].v)) { //s il ne sont pas deja dans la meme classe
            arbre[*nb_arbre] = G[i];
            (*nb_arbre)++;
            if (*nb_arbre >= nb_cellules - 1) break;
        }
    }
    free_partition(&p);
}

// Génère un fichier DOT pour visualisation de graphe
void generer_dot(const char* nom, arete aretes[], int nb) {
    FILE* f = fopen(nom, "w");
    if (!f) {
        perror("Erreur ouverture fichier DOT");
        return;
    }
    fprintf(f, "graph G {\n  node [shape=circle];\n");
    for (int i = 0; i < nb; i++) {
        fprintf(f, "  %d -- %d;\n", aretes[i].u, aretes[i].v);
    }
    fprintf(f, "}\n");
    fclose(f);
}

// Convertit un indice en coordonnées x, y
void indice_vers_coord(int indice, int colonnes, int* x, int* y) {
    *y = indice / colonnes;
    *x = indice % colonnes;
}

// la fonction supprime un mur entre deux cellules adjacentes
//=============================================================================================================================//
// 15 -> 1111     murs de tous les cotes                                                                                       //
//  8 -> 1000     mur à gauche                                                                                                 //
//  4 -> 0100     mur en bas                                                                                                   //
//  2 -> 0010     mur à droite                                                                                                 //
//  1 -> 0001     mur en haut                                                                                                  //
// chaque bit represente la presence d'un mur dans une position                                                                //
// si par exemple on a 4 murs autour de u, donc murs[u] sera égale à 1|2|3|4 = 0001|0010|0100|1000 = 1111 = 15                 //
// et pour supprimer un mur, on doit d'abord inverser le nombre qui represente le murs par '~'par exemple pour le mur droite   //
// droite ~2 = 1101 et si on fait murs[u]&~2, le seule bit qui va changer est le troisieme et deviendre 0                      //
//=============================================================================================================================//
void supprimer_mur(int *murs, int colonnes, int u, int v) {
    int x1, y1, x2, y2; //coord des cellules 
    indice_vers_coord(u, colonnes, &x1, &y1);
    indice_vers_coord(v, colonnes, &x2, &y2);
    //distance entre les 2 cellures pour determiner la position d'une par rapport à l'autre (haut, bas, droit,gauche)
    int dx = x2 - x1; 
    int dy = y2 - y1;
    int idx1 = y1 * colonnes + x1; // = u, juste pour quelle se voit
    int idx2 = y2 * colonnes + x2; // = v, juste pour quelle se voit
    if (dx == 1) { // v à droite de u
        murs[idx1] &= ~2; // supprimer mur à droite de u 
        murs[idx2] &= ~8; // supprimer mur à gauche de v 
    } else if (dx == -1) { // v à gauche de u
        murs[idx1] &= ~8;
        murs[idx2] &= ~2;
    } else if (dy == 1) { // v en bas de u
        murs[idx1] &= ~4; // bas
        murs[idx2] &= ~1; // haut
    } else if (dy == -1) { // v en haut de u
        murs[idx1] &= ~1;
        murs[idx2] &= ~4;
    }
}



//============================== SDL =================================================================

// Dessine les murs d'une cellule donnée avec SDL
void dessiner_murs(SDL_Renderer* rendu, int x, int y, int *murs, int colonnes) {
    int px = x * TAILLE_CELLULE;
    int py = y * TAILLE_CELLULE;
    int val = murs[y * colonnes + x];
    if (val & 1) SDL_RenderDrawLine(rendu, px, py, px + TAILLE_CELLULE, py); // haut
    if (val & 2) SDL_RenderDrawLine(rendu, px + TAILLE_CELLULE, py, px + TAILLE_CELLULE, py + TAILLE_CELLULE); // droite
    if (val & 4) SDL_RenderDrawLine(rendu, px, py + TAILLE_CELLULE, px + TAILLE_CELLULE, py + TAILLE_CELLULE); // bas
    if (val & 8) SDL_RenderDrawLine(rendu, px, py, px, py + TAILLE_CELLULE); // gauche
}

// Affiche le labyrinthe avec SDL
void afficher_labyrinthe_sdl(arete arbre[], int nb_aretes, int lignes, int colonnes) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }
    int largeur = colonnes * TAILLE_CELLULE;
    int hauteur = lignes * TAILLE_CELLULE;
    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        largeur+1, hauteur+1, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
    SDL_RenderClear(rendu);
    int total = lignes * colonnes;
    int *murs = malloc(sizeof(int) * total);
    if (!murs) {
        fprintf(stderr, "Allocation mémoire impossible pour murs\n");
        SDL_DestroyRenderer(rendu);
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        return;
    }
    for (int i = 0; i < total; i++) murs[i] = 1|2|4|8; // = 15
    for (int i = 0; i < nb_aretes; i++) supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);
    SDL_SetRenderDrawColor(rendu, 0, 255, 0, 255);
    for (int y = 0; y < lignes; y++)
        for (int x = 0; x < colonnes; x++)
            dessiner_murs(rendu, x, y, murs, colonnes);
    SDL_RenderPresent(rendu);
    SDL_Event e;
    int quitter = 0;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quitter = 1;
        }
        SDL_Delay(10);
    }
    free(murs);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}


// Ajoute les bordures hautes et gauches dans la fonction dessiner_tuile
void dessiner_tuile(SDL_Renderer* rendu, SDL_Texture* tileset, int* murs, int x, int y, int colonnes) {
    SDL_Rect dst;
    SDL_Rect src;
    int val_murs = murs[y * colonnes + x];
    src = src_murs[val_murs];

    dst.x = x * TAILLE_CELLULE;
    dst.y = y * TAILLE_CELLULE;
    dst.w = TAILLE_CELLULE;
    dst.h = TAILLE_CELLULE;
    
    SDL_RenderCopy(rendu, tileset, &src, &dst);
}



void afficher_labyrinthe_sdl_tuiles(int *murs, int lignes, int colonnes) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return;
    }

    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe Tuiles",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    

    // Charger l'image comme surface
    SDL_Surface* tileset_surface = IMG_Load("tileset.png");
    if (!tileset_surface) {
        fprintf(stderr, "Erreur chargement tileset.png : %s\n", IMG_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Créer une texture depuis la surface
    SDL_Texture* tileset = SDL_CreateTextureFromSurface(rendu, tileset_surface);
    if (!tileset) {
        fprintf(stderr, "Erreur création texture : %s\n", SDL_GetError());
        SDL_FreeSurface(tileset_surface);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Libérer la surface car plus nécessaire
    SDL_FreeSurface(tileset_surface);

    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
    SDL_RenderClear(rendu);


    for (int y = 0; y < lignes ; y++) {
        for (int x = 0; x < colonnes; x++) {
            dessiner_tuile(rendu, tileset, murs, x, y, colonnes);
        }
    }


    SDL_RenderPresent(rendu);
    SDL_Event e;
    int quitter = 0;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quitter = 1;
        }
        SDL_Delay(10);
    }

    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}

// ======================================================= fin Sdl ========================================


// Affiche le labyrinthe en mode texte avec Unicode
void afficher_labyrinthe_unicode(int *murs, int lignes, int colonnes) {
    // bordure haut
    printf("┌");
    for (int x = 0; x < colonnes; x++) {
        printf("──");
        printf(x < colonnes - 1 ? "┬" : "┐");
    }
    printf("\n");
    for (int y = 0; y < lignes; y++) {
        printf("│"); // bordure gauche
        for (int x = 0; x < colonnes; x++) {
            printf("  ");
            int val = murs[y * colonnes + x];
            printf(val & 2 ? "│" : " ");
        }
        printf("\n");
        if (y < lignes - 1) {
            printf("├");
            for (int x = 0; x < colonnes; x++) {
                int val = murs[y * colonnes + x];
                printf(val & 4 ? "──" : "  ");
                printf(x < colonnes - 1 ? "┼" : "┤");
            }
            printf("\n");
        }
    }
    // bordure bas
    printf("└");
    for (int x = 0; x < colonnes; x++) {
        printf("──");
        printf(x < colonnes - 1 ? "┴" : "┘");
    }
    printf("\n");
}

// Fonction principale
int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <lignes> <colonnes> <SDL-0/1>\n", argv[0]);
        return 1;
    }
    int lignes = atoi(argv[1]);
    int colonnes = atoi(argv[2]);
    int utiliser_sdl = atoi(argv[3]); // console ou SDL
    if (lignes <= 0 || colonnes <= 0) {
        fprintf(stderr, "Dimensions invalides : lignes et colonnes doivent être > 0\n");
        return 1;
    }

    
    srand(0);
    int total_cellules = lignes * colonnes;
    arete *graphe;
    int nb_aretes = generation_grille_vide(&graphe, lignes, colonnes);
    fisher_yates(graphe, nb_aretes);

    arete *arbre = malloc(sizeof(arete) * total_cellules);
    if (!arbre) {
        fprintf(stderr, "Allocation mémoire impossible pour arbre\n");
        free(graphe);
        return 1;
    }
    int nb_arbre;
    construire_arbre_couvrant(graphe, nb_aretes, arbre, &nb_arbre, total_cellules);
    generer_dot("graphe.dot", graphe, nb_aretes);
    generer_dot("arbre.dot", arbre, nb_arbre);
    printf("Fichiers DOT générés : graphe.dot et arbre.dot\n");

    int *murs = malloc(sizeof(int) * total_cellules);
    if (!murs) {
        fprintf(stderr, "Allocation mémoire impossible pour murs\n");
        free(graphe);
        free(arbre);
        return 1;
    }
    for (int i = 0; i < total_cellules; i++) murs[i] = 1|2|4|8;
    for (int i = 0; i < nb_arbre; i++) {
        supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);
    }

    if (utiliser_sdl) {
        afficher_labyrinthe_sdl(arbre, nb_arbre, lignes, colonnes);
        afficher_labyrinthe_sdl_tuiles(murs, lignes, colonnes);
    } else {
        afficher_labyrinthe_unicode(murs, lignes, colonnes);
    }

    free(graphe);
    free(arbre);
    free(murs);
    return 0;
}
