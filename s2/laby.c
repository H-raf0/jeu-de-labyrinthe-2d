#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "labySDL.h"
#include "laby.h"

#define MAX_COUT 10 // poids pour penetrer une cellule


// Initialise une partition de taille 'total'
void init_partition(partition* p, int total) {
    p->taille = total;
    p->parent = malloc(sizeof(int) * total);
    p->rang = malloc(sizeof(int) * total);
    if (!p->parent || !p->rang) {
        printf("Allocation mémoire impossible pour partition\n");
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
        printf("Allocation mémoire impossible pour arêtes\n");
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


void construire_arbre_imparfait(arete G[], int nb_aretes, arete *arbre, int* nb_arbre, int nb_cellules) {
    partition p;
    init_partition(&p, nb_cellules);
    *nb_arbre = 0;

    bool* est_dans_arbre = calloc(nb_aretes, sizeof(bool));

    // J'ajoute dans "arbre" les murs à supprimer pour former une arbre parfaite
    for (int i = 0; i < nb_aretes; i++) {
        if (fusion(&p, G[i].u, G[i].v)) {
            arbre[*nb_arbre] = G[i];
            (*nb_arbre)++;
            est_dans_arbre[i] = true;
        }
    }

    // J'ajoute plus d'arrêt qui seront supprimé
    for (int i = 0; i < nb_aretes; i++) {
        if (!est_dans_arbre[i]) {
            if (((float)(rand() % 100) / 100) < 0.3f) {
                arbre[*nb_arbre] = G[i];
                (*nb_arbre)++; 
            }
        }
    }

    free_partition(&p);
    free(est_dans_arbre);
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



int** creer_matrice_adjacence_cout_altr(int* murs, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int** matrice = malloc(sizeof(int*) * nb_cellules);
    if (!matrice) return NULL;
    for (int i = 0; i < nb_cellules; i++) {
        matrice[i] = calloc(nb_cellules, sizeof(int));
        if (!matrice[i]) {
            printf("erreur lors de la création de la matrice d'adjacence\n");
            return NULL; 
        }
    }

    // Remplir la matrice
    for (int u = 0; u < nb_cellules; u++) {
        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);

        // Voisin du haut
        if (y > 0 && !(murs[u] & 1) && matrice[u][(y - 1) * colonnes + x] == 0) {
            int v = (y - 1) * colonnes + x;
            int cout_arete = (rand() % MAX_COUT) + 1; // Génère un coût aléatoire pour l'arête
            matrice[u][v] = cout_arete;
            matrice[v][u] = cout_arete; // Assigne le MÊME coût au chemin du retour
        }
        // Voisin de droite
        if (x < colonnes - 1 && !(murs[u] & 2) && matrice[u][y * colonnes + (x + 1)] == 0) {
            int v = y * colonnes + (x + 1);
            int cout_arete = (rand() % MAX_COUT) + 1;
            matrice[u][v] = cout_arete;
            matrice[v][u] = cout_arete;
        }
        // Voisin du bas
        if (y < lignes - 1 && !(murs[u] & 4) && matrice[u][(y + 1) * colonnes + x] == 0) {
            int v = (y + 1) * colonnes + x;
            int cout_arete = (rand() % MAX_COUT) + 1;
            matrice[u][v] = cout_arete;
            matrice[v][u] = cout_arete;
        }
        // Voisin de gauche
        if (x > 0 && !(murs[u] & 8) && matrice[u][y * colonnes + (x - 1)] == 0) {
            int v = y * colonnes + (x - 1);
            int cout_arete = (rand() % MAX_COUT) + 1;
            matrice[u][v] = cout_arete;
            matrice[v][u] = cout_arete;
        }
    }
    return matrice;
}


int** creer_matrice_adjacence_connue(int* murs_connus, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int** matrice = malloc(sizeof(int*) * nb_cellules);
    if (!matrice) return NULL;
    for (int i = 0; i < nb_cellules; i++) {
        matrice[i] = calloc(nb_cellules, sizeof(int));
        if (!matrice[i]) {
            printf("erreur lors de la création de la matrice d'adjacence\n");
            return NULL; 
        }
    }

    // Remplir la matrice
    for (int u = 0; u < nb_cellules; u++) {
        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);
        int v;

        // Si le mur HAUT n'est pas connu, le passage est considéré comme ouvert avec un coût de 1
        if (y > 0 && !(murs_connus[u] & 1)) {
            v = (y - 1) * colonnes + x;
            matrice[u][v] = 1;
            matrice[v][u] = 1;
        }
        // Si le mur DROIT n'est pas connu...
        if (x < colonnes - 1 && !(murs_connus[u] & 2)) {
            v = y * colonnes + (x + 1);
            matrice[u][v] = 1;
            matrice[v][u] = 1;
        }
        // etc. pour les autres directions
        if (y < lignes - 1 && !(murs_connus[u] & 4)) {
            v = (y + 1) * colonnes + x;
            matrice[u][v] = 1;
            matrice[v][u] = 1;
        }
        if (x > 0 && !(murs_connus[u] & 8)) {
            v = y * colonnes + (x - 1);
            matrice[u][v] = 1;
            matrice[v][u] = 1;
        }
    }
    return matrice;
}



int** creer_matrice_couts_dynamiques(int* murs_connus, const int* penalite_map, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int** matrice = malloc(sizeof(int*) * nb_cellules);
    if (!matrice) return NULL;
    for (int i = 0; i < nb_cellules; i++) {
        matrice[i] = calloc(nb_cellules, sizeof(int));
        if (!matrice[i]) {
            printf("erreur lors de la création de la matrice d'adjacence\n");
            return NULL; 
        }
    }

    for (int u = 0; u < nb_cellules; u++) {
        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);
        int v;
        int cout_aller, cout_retour;

        // Voisin du haut
        if (y > 0 && !(murs_connus[u] & 1)) {
            v = (y - 1) * colonnes + x;
            cout_aller = 1 + penalite_map[v];
            cout_retour = 1 + penalite_map[u];
            matrice[u][v] = cout_aller;
            matrice[v][u] = cout_retour;
        }
        // Voisin de droite
        if (x < colonnes - 1 && !(murs_connus[u] & 2)) {
            v = y * colonnes + (x + 1);
            cout_aller = 1 + penalite_map[v];
            cout_retour = 1 + penalite_map[u];
            matrice[u][v] = cout_aller;
            matrice[v][u] = cout_retour;
        }
        // Voisin du bas
        if (y < lignes - 1 && !(murs_connus[u] & 4)) {
            v = (y + 1) * colonnes + x;
            cout_aller = 1 + penalite_map[v];
            cout_retour = 1 + penalite_map[u];
            matrice[u][v] = cout_aller;
            matrice[v][u] = cout_retour;
        }
        // Voisin de gauche
        if (x > 0 && !(murs_connus[u] & 8)) {
            v = y * colonnes + (x - 1);
            cout_aller = 1 + penalite_map[v];
            cout_retour = 1 + penalite_map[u];
            matrice[u][v] = cout_aller;
            matrice[v][u] = cout_retour;
        }
    }
    return matrice;
}

int** creer_matrice_couts_connus(int* murs_connus, int* passages_counts, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int** matrice = malloc(sizeof(int*) * nb_cellules);
    if (!matrice) return NULL;
    for (int i = 0; i < nb_cellules; i++) {
        matrice[i] = calloc(nb_cellules, sizeof(int));
        if (!matrice[i]) {
            printf("erreur lors de la création de la matrice d'adjacence\n");
            return NULL; 
        }
    }

    for (int u = 0; u < nb_cellules; u++) {
        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);
        int v;

        // Voisin du haut
        if (y > 0 && !(murs_connus[u] & 1)) {
            v = (y - 1) * colonnes + x;
            int cout = 1 + passages_counts[v]; // Coût pour entrer dans la cellule v
            matrice[u][v] = cout;
            matrice[v][u] = 1 + passages_counts[u]; // Coût pour revenir en u
        }
        // Voisin de droite
        if (x < colonnes - 1 && !(murs_connus[u] & 2)) {
            v = y * colonnes + (x + 1);
            int cout = 1 + passages_counts[v];
            matrice[u][v] = cout;
            matrice[v][u] = 1 + passages_counts[u];
        }
        // Voisin du bas
        if (y < lignes - 1 && !(murs_connus[u] & 4)) {
            v = (y + 1) * colonnes + x;
            int cout = 1 + passages_counts[v];
            matrice[u][v] = cout;
            matrice[v][u] = 1 + passages_counts[u];
        }
        // Voisin de gauche
        if (x > 0 && !(murs_connus[u] & 8)) {
            v = y * colonnes + (x - 1);
            int cout = 1 + passages_counts[v];
            matrice[u][v] = cout;
            matrice[v][u] = 1 + passages_counts[u];
        }
    }
    return matrice;
}


// Libère la mémoire allouée pour la matrice d'adjacence.
void liberer_matrice_adjacence(int** matrice, int nb_cellules) {
    if (!matrice) return;
    for (int i = 0; i < nb_cellules; i++) {
        free(matrice[i]);
    }
    free(matrice);
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


void ajouter_mur(int *murs, int colonnes, int u, int v) {
    int x1, y1, x2, y2;
    indice_vers_coord(u, colonnes, &x1, &y1);
    indice_vers_coord(v, colonnes, &x2, &y2);
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx == 1) { // v à droite de u
        murs[u] |= 2; // ajouter mur à droite de u
        murs[v] |= 8; // ajouter mur à gauche de v
    } else if (dx == -1) { // v à gauche de u
        murs[u] |= 8;
        murs[v] |= 2;
    } else if (dy == 1) { // v en bas de u
        murs[u] |= 4; // bas
        murs[v] |= 1; // haut
    } else if (dy == -1) { // v en haut de u
        murs[u] |= 1;
        murs[v] |= 4;
    }
}






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


// BFS pour naviguer dans le labyrinthe en utilisant le tableau de murs
void BFS_laby(int *murs, int lignes, int colonnes, int origine, noeud* n) {
    int nb_cellules = lignes * colonnes;
    file f;
    initialiser_file(&f, nb_cellules);
    initialiser_noeuds(n, origine, nb_cellules);

    n->visite[origine] = 1;
    enfiler(&f, origine);

    while (!filevide(&f)) {
        int u = defiler(&f);

        int x, y;
        indice_vers_coord(u, colonnes, &x, &y);

        // Voisin du haut (y-1)
        if (y > 0 && !(murs[u] & 1)) { // S'il n'y a pas de mur en haut
            int v = (y - 1) * colonnes + x;
            if (!n->visite[v]) {
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->parent[v] = u;
                enfiler(&f, v);
            }
        }
        // Voisin de droite (x+1)
        if (x < colonnes - 1 && !(murs[u] & 2)) { // S'il n'y a pas de mur à droite
            int v = y * colonnes + (x + 1);
            if (!n->visite[v]) {
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->parent[v] = u;
                enfiler(&f, v);
            }
        }
        // Voisin du bas (y+1)
        if (y < lignes - 1 && !(murs[u] & 4)) { // S'il n'y a pas de mur en bas
            int v = (y + 1) * colonnes + x;
            if (!n->visite[v]) {
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->parent[v] = u;
                enfiler(&f, v);
            }
        }
        // Voisin de gauche (x-1)
        if (x > 0 && !(murs[u] & 8)) { // S'il n'y a pas de mur à gauche
            int v = y * colonnes + (x - 1);
            if (!n->visite[v]) {
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->parent[v] = u;
                enfiler(&f, v);
            }
        }
    }
    free_file(&f);
}

// Dijkstra fonctionnant sur une matrice d'adjacence.
void Dijkstra_laby(int** graphe, int nb_cellules, int destination, noeud* n) {
    initialiser_noeuds(n, destination, nb_cellules);
    
    tas t;
    t.tab = malloc(sizeof(int) * nb_cellules);
    t.positions = malloc(sizeof(int) * nb_cellules); // Allouer le tableau des positions
    if (!t.tab || !t.positions) {
        printf("Erreur d'allocation pour le tas dans Dijkstra\n");
        if(t.tab) free(t.tab);
        if(t.positions) free(t.positions);
        return;
    }
    t.taille = 0;

    // Initialiser les positions à -1 (non présent dans le tas)
    for (int i = 0; i < nb_cellules; i++) {
        t.positions[i] = -1;
    }

    inserer(&t, destination, n);

    while (t.taille > 0) {
        int u = extraire_min(&t, n);
        n->visite[u] = 1;

        for (int v = 0; v < nb_cellules; v++) {
            if (graphe[v][u] > 0) { // Si arête (v, u) existe
                int cout_du_pas = graphe[v][u];

                if (n->distance[u] + cout_du_pas < n->distance[v]) {
                    n->distance[v] = n->distance[u] + cout_du_pas;
                    n->parent[v] = u;
                    
                    
                    if (t.positions[v] == -1) {
                        inserer(&t, v, n);
                    } else {
                        mettre_a_jour_priorite(&t, v, n);
                    }
                }
            }
        }
    }
    
    // Libérer la mémoire
    free(t.tab);
    free(t.positions);
}







// Reconstruit le chemin depuis la destination vers le départ en utilisant le tableau parent.
// Le chemin est inversé pour être dans le bon sens (départ -> destination).
// Renvoie le nombre d'étapes dans le chemin.
int reconstruire_chemin(noeud* n, int depart, int destination, int* chemin_buffer) {
    if (n->distance[depart] == INF) {
        printf("Avertissement : Pas de chemin trouvé entre %d et %d\n", depart, destination);
        return 0; // Pas de chemin
    }

    int etapes = 0;
    int courant = depart;
    
    // On suit les parents depuis le départ jusqu'à atteindre la destination.
    // Le chemin est construit directement dans le bon ordre.
    while (courant != destination) {
        chemin_buffer[etapes++] = courant;
        courant = n->parent[courant];
        
        // éviter les boucles infinies, impossible pour le moment
        if (courant == -1) { 
            printf("Erreur: Chemin cassé lors de la reconstruction.\n");
            return 0;
        }
    }
    // ajouter la destination à la fin du chemin.
    chemin_buffer[etapes++] = destination;
    
    return etapes;
}

int reconstruire_chemin_inverse(noeud* n, int depart, int destination, int nb_cellules, int* chemin_buffer) {
    // Pour A*, la recherche part de "depart", donc on vérifie si "destination" a été atteinte.
    if (n->parent[destination] == -1 && depart != destination) {
        printf("Avertissement : Pas de chemin trouvé entre %d et %d\n", depart, destination);
        return 0; // Pas de chemin
    }

    int* chemin_temp = malloc(sizeof(int) * nb_cellules);
    if (!chemin_temp) return 0;

    int etapes = 0;
    int courant = destination;

    // On suit les parents depuis la destination jusqu'à atteindre le départ.
    while (courant != -1) {
        chemin_temp[etapes++] = courant;
        if (courant == depart) break; // On a atteint le début, on s'arrête.
        courant = n->parent[courant];
        
        
        if (etapes >= nb_cellules) {
            printf("Erreur dans la reconstruction du chemin\n");
            free(chemin_temp);
            return 0;
        }
    }
    
    // Si la boucle s'est terminée sans trouver le départ, il y a un problème.
    if (courant != depart) {
        printf("Erreur dans la reconstruction du chemin\n");
        free(chemin_temp);
        return 0;
    }


    // on inverse l'odre du chemin
    for (int i = 0; i < etapes; i++) {
        chemin_buffer[i] = chemin_temp[etapes - 1 - i];
    }

    free(chemin_temp);
    return etapes;
}

int A_etoile_laby(int *murs, int lignes, int colonnes, int depart, int destination, noeud* n, int type_heuristique) {
    int nb_cellules = lignes * colonnes;
    int *g_costs = malloc(sizeof(int) * nb_cellules); // Tableau pour les coûts g(n)

    if (!g_costs) {
        printf("Allocation échouée pour g_costs dans A*\n");
        return -1;
    }

    // Initialisation
    initialiser_noeuds(n, depart, nb_cellules); // n->distance stockera f(n)
    for (int i = 0; i < nb_cellules; i++) {
        g_costs[i] = INF;
    }

    g_costs[depart] = 0;
    
    int x_dest, y_dest;
    indice_vers_coord(destination, colonnes, &x_dest, &y_dest);
    int x_depart, y_depart;
    indice_vers_coord(depart, colonnes, &x_depart, &y_depart);
    
    // Valeur de f du départ est juste l'heuristique (car g=0)
    n->distance[depart] = estimation(x_depart, x_dest, y_depart, y_dest, type_heuristique);

    // Initialisation du tas (file de priorité)
    tas t;
    t.tab = malloc(sizeof(int) * nb_cellules);
    t.positions = malloc(sizeof(int) * nb_cellules);
    t.taille = 0;

    for (int i = 0; i < nb_cellules; i++) {
        t.positions[i] = -1;
    }

    // Initialiser les positions à -1, signifie "pas dans le tas"
    inserer(&t, depart, n);

    int noeuds_visites = 0;

    while (t.taille > 0) {
        int u = extraire_min(&t, n);
        noeuds_visites++;

        if (u == destination) {
            free(g_costs);
            free(t.tab);
            free(t.positions);
            return noeuds_visites; // Chemin trouvé
        }

        // n->visite = 1 si déjà visité
        n->visite[u] = 1;

        int x_u, y_u;
        indice_vers_coord(u, colonnes, &x_u, &y_u);

        // Explorer les voisins
        int voisins[4];
        int nb_voisins = 0;
        if (y_u > 0 && !(murs[u] & 1)) voisins[nb_voisins++] = (y_u - 1) * colonnes + x_u; // Haut
        if (x_u < colonnes - 1 && !(murs[u] & 2)) voisins[nb_voisins++] = y_u * colonnes + (x_u + 1); // Droite
        if (y_u < lignes - 1 && !(murs[u] & 4)) voisins[nb_voisins++] = (y_u + 1) * colonnes + x_u; // Bas
        if (x_u > 0 && !(murs[u] & 8)) voisins[nb_voisins++] = y_u * colonnes + (x_u - 1); // Gauche

        for (int i = 0; i < nb_voisins; i++) {
            int v = voisins[i];
            
            if (n->visite[v]) continue; // Existe déjà

            // Le coût pour se déplacer à un voisin est 1 içi
            int tentative_g = g_costs[u] + 1;

            if (tentative_g < g_costs[v]) {
                // Ce chemin vers v est mieux que le précédent
                n->parent[v] = u;
                g_costs[v] = tentative_g;

                int x_v, y_v;
                indice_vers_coord(v, colonnes, &x_v, &y_v);
                int h_cost = estimation(x_v, x_dest, y_v, y_dest, type_heuristique);
                
                n->distance[v] = g_costs[v] + h_cost; // n->distance est notre f(n)
                
                // On vérifie si v est DÉJÀ dans le tas t
                if (t.positions[v] == -1) {
                    // S'il n'y est pas, on l'insère.
                    inserer(&t, v, n);
                } else {
                    // S'il y est déjà, on met simplement sa priorité à jour.
                    mettre_a_jour_priorite(&t, v, n);
                }
            }
        }
    }

    // Pas de chemin trouvé
    free(g_costs);
    free(t.tab);
    free(t.positions);
    return noeuds_visites;
}

// Fonction pour comparer les heuristiques
void comparer_heuristiques_A_etoile(int* murs, int lignes, int colonnes, int depart, int destination) {
    noeud n;
    //int nb_cellules = lignes * colonnes;
    int noeuds_visites;

    printf("\n--- Comparaison des Heuristiques A* pour le trajet %d -> %d ---\n", depart, destination);

    // Distance de Manhattan
    noeuds_visites = A_etoile_laby(murs, lignes, colonnes, depart, destination, &n, HEURISTIC_MANHATTAN);
    printf("Heuristique MANHATTAN : %d noeuds visités.\n", noeuds_visites);
    free_noeuds(&n);

    // Distance Euclidienne
    noeuds_visites = A_etoile_laby(murs, lignes, colonnes, depart, destination, &n, HEURISTIC_EUCLIDEAN);
    printf("Heuristique EUCLIDIENNE : %d noeuds visités.\n", noeuds_visites);
    free_noeuds(&n);

    // Distance de Tchebychev
    noeuds_visites = A_etoile_laby(murs, lignes, colonnes, depart, destination, &n, HEURISTIC_TCHEBYCHEV);
    printf("Heuristique TCHEBYCHEV: %d noeuds visités.\n", noeuds_visites);
    free_noeuds(&n);
    printf("-----------------------------------------------------------------\n\n");
}