#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "laby.h"
#include "labySDL.h"

// --- Définitions pour le mode de jeu à 2 ÉTATS ---
#define AI_MODE_SEARCH_ZONE 0 // Le monstre explore une zone
#define AI_MODE_HUNT 1        // Le monstre voit le joueur et le chasse

#define NOMBRE_MONSTRES 3
#define SEUIL_DETECTION_HUNT 8      // Portée de la vue directe
#define DUREE_PISTE 300             // La piste reste "chaude" pendant 300 frames
#define TAILLE_ZONE_RECHERCHE 12
#define MEMOIRE_MAX 50
#define VITESSE_MONSTRE 5

typedef struct {
    int pos;
    int mode;
    int* murs_connus; // G
    arete* memoire_murs;
    int memoire_tete, memoire_queue, memoire_taille_actuelle;
    int timer_piste;
    // Variables pour la recherche de zone
    bool* noeuds_visites_zone; // F
    int* frontier_nodes; // O
    int frontier_size;
    // Variables pour le chemin en cours
    int prochaine_position;

    int move_cooldown;
} Monstre;

// Gère l'apprentissage et l'oubli des murs
void apprendre_mur(Monstre* monstre, int u, int v, int colonnes) {
    int diff = v - u;
    int dir_flag = 0;
    if (diff == -colonnes) dir_flag = 1; else if (diff == 1) dir_flag = 2; else if (diff == colonnes) dir_flag = 4; else if (diff == -1) dir_flag = 8;
    if (monstre->murs_connus[u] & dir_flag) return;

    printf("Monstre %d apprend le mur entre %d et %d.\n", monstre->pos, u, v);
    ajouter_mur(monstre->murs_connus, colonnes, u, v);
    if (monstre->memoire_taille_actuelle >= MEMOIRE_MAX) {
        arete mur_oublie = monstre->memoire_murs[monstre->memoire_tete];
        supprimer_mur(monstre->murs_connus, colonnes, mur_oublie.u, mur_oublie.v);
        monstre->memoire_tete = (monstre->memoire_tete + 1) % MEMOIRE_MAX;
        monstre->memoire_taille_actuelle--;
    }
    monstre->memoire_murs[monstre->memoire_queue] = (arete){u, v};
    monstre->memoire_queue = (monstre->memoire_queue + 1) % MEMOIRE_MAX;
    monstre->memoire_taille_actuelle++;
}



int gidc(int* murs_reels, int* murs_connus,int lignes, int colonnes, int depart, int destination) {
    int nb_cellules = lignes * colonnes;

    if (!murs_connus) { return NULL; }

    int pos_actuelle = depart;

    bool a_transite = false;
    while (!a_transite) {
        
        int** graphe_connu = creer_matrice_adjacence_connue(murs_connus, lignes, colonnes); // ? cout=1
        if (!graphe_connu) {
            fprintf(stderr, "Erreur création graphe connu.\n");
            pos_actuelle = destination;
            continue;
        }

        noeud plan;
        // On planifie avec Dijkstra sur ce graphe frais
        Dijkstra_laby(graphe_connu, nb_cellules, destination, &plan);
        
        int meilleur_voisin = -1;
        int min_dist_estimee = INF;
        int x_act, y_act;
        indice_vers_coord(pos_actuelle, colonnes, &x_act, &y_act);

        int voisins[4] = {
            (y_act > 0) ? (y_act - 1) * colonnes + x_act : -1,
            (x_act < colonnes - 1) ? y_act * colonnes + (x_act + 1) : -1,
            (y_act < lignes - 1) ? (y_act + 1) * colonnes + x_act : -1,
            (x_act > 0) ? y_act * colonnes + (x_act - 1) : -1
        };

        for (int i = 0; i < 4; i++) {
            int v = voisins[i];
            if (v == -1) continue;
            
            int diff_temp = v - pos_actuelle;
            int direction_flag_temp = 0;
            if (diff_temp == -colonnes) direction_flag_temp = 1; else if (diff_temp == 1) direction_flag_temp = 2;
            else if (diff_temp == colonnes) direction_flag_temp = 4; else if (diff_temp == -1) direction_flag_temp = 8;
            
            if (!(murs_connus[pos_actuelle] & direction_flag_temp)) {
                if (plan.distance[v] < min_dist_estimee) {
                    min_dist_estimee = plan.distance[v];
                    meilleur_voisin = v;
                }
            }
        }

        if (meilleur_voisin == -1) {
            printf("IMPASSE CONNUE. L'agent ne peut plus bouger.\n");
            pos_actuelle = destination;
            // Nettoyer avant de continuer ---
            free_noeuds(&plan);
            liberer_matrice_adjacence(graphe_connu, nb_cellules);
            continue;
        }

        int diff = meilleur_voisin - pos_actuelle;
        int direction_flag = 0;
        if (diff == -colonnes) direction_flag = 1; else if (diff == 1) direction_flag = 2;
        else if (diff == colonnes) direction_flag = 4; else if (diff == -1) direction_flag = 8;

        if (murs_reels[pos_actuelle] & direction_flag) {
            printf("SURPRISE! Mur de %d à %d. Apprentissage...\n", pos_actuelle, meilleur_voisin);
            ajouter_mur(murs_connus, colonnes, pos_actuelle, meilleur_voisin);
        } else {
            printf("Mouvement de %d à %d.\n", pos_actuelle, meilleur_voisin);
            pos_actuelle = meilleur_voisin;
            a_transite = true;
            
        }
        
        // Le "cerveau" est détruit ici, à la fin de chaque décision ---
        free_noeuds(&plan);
        liberer_matrice_adjacence(graphe_connu, nb_cellules);

    } 
    return pos_actuelle; //nouvelle position

}


int gidi(/* ? */){

    /* */
}



// "Cerveau" de l'IA: prend UNE décision et la prépare pour exécution
void mettre_a_jour_monstre(Monstre* monstre, int joueur_pos, int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;

    // --- PERCEPTION & DÉCISION DU MODE ---
    int j_x, j_y, m_x, m_y;
    indice_vers_coord(joueur_pos, colonnes, &j_x, &j_y);
    indice_vers_coord(monstre->pos, colonnes, &m_x, &m_y);
    int distance = abs(j_x - m_x) + abs(j_y - m_y);

    if (monstre->timer_piste > 0) monstre->timer_piste--;

    int old_mode = monstre->mode;
    
    // La chasse. Si le joueur est assez proche ou sa fait pas longtemps qu'il s'est s'échapé, on le chasse.
    if (distance <= SEUIL_DETECTION_HUNT || monstre->timer_piste > 0) {
        monstre->mode = AI_MODE_HUNT;
        if (distance <= SEUIL_DETECTION_HUNT) monstre->timer_piste = DUREE_PISTE; 
    } 
    // La recherche. Si on a perdu le joueur..
    else {
        monstre->mode = AI_MODE_SEARCH_ZONE;
    }

    // Si on vient de changer de mode, on annule le plan en cours
    if (monstre->mode != old_mode) {
        if(monstre->mode == AI_MODE_SEARCH_ZONE) {
            printf("Monstre %d -> MODE RECHERCHE DE ZONE\n", monstre->pos);
            // On définit la zone de recherche autour de la position REELLE du joueur (comme s'il entendait un bruit)
            int zone_centre_x = j_x;
            int zone_centre_y = j_y;
            
            // On réinitialise O et F
            monstre->frontier_size = 0;
            memset(monstre->noeuds_visites_zone, 0, sizeof(bool) * nb_cellules);
            monstre->frontier_nodes[monstre->frontier_size++] = monstre->pos;
            
        }
    }



    // ---  PLANIFICATION  ---
    
    switch (monstre->mode) {
        case AI_MODE_HUNT:
            monstre->prochaine_position = gidc(murs_reels, monstre->murs_connus, lignes, colonnes, monstre->pos, joueur_pos);
            break;
        case AI_MODE_SEARCH_ZONE:
        { 
            //?
        
        }
    }
    

    // --- C. ACTION ---
    if (monstre->move_cooldown > 0) { monstre->move_cooldown--; return; }
    monstre->move_cooldown = VITESSE_MONSTRE;

    int prochain_pas = -1;
    if (monstre->mode == AI_MODE_HUNT) {
        if (monstre->pos != joueur_pos) {
            prochain_pas = monstre->prochaine_position;
        }
    } else if (monstre->mode == AI_MODE_SEARCH_ZONE) {
        
    }

    if (prochain_pas != -1) {
        int diff = prochain_pas - monstre->pos;
        int dir_flag = 0;
        if (diff == -colonnes) dir_flag = 1; else if (diff == 1) dir_flag = 2; else if (diff == colonnes) dir_flag = 4; else if (diff == -1) dir_flag = 8;
        
        if (murs_reels[monstre->pos] & dir_flag) {
            apprendre_mur(monstre, monstre->pos, prochain_pas, colonnes);
        } else {
            monstre->pos = prochain_pas;
            // ?
        }
    }
}


// Fonction de jeu principale
void lancer_jeu(int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* fenetre = SDL_CreateWindow("Jeu IA Focalisée", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");
    SDL_Texture* monstre_texture = IMG_LoadTexture(rendu, "monstre.png");

    int joueur_pos = 0;
    Monstre monstres[NOMBRE_MONSTRES];
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        Monstre* m = &monstres[i];
        m->pos = nb_cellules - 1 - i * 2;
        m->mode = AI_MODE_SEARCH_ZONE;
        m->timer_piste = 0;
        m->murs_connus = calloc(nb_cellules, sizeof(int));
        m->memoire_murs = malloc(sizeof(arete) * MEMOIRE_MAX);
        m->memoire_tete = 0; m->memoire_queue = 0; m->memoire_taille_actuelle = 0;
        
        m->noeuds_visites_zone = calloc(nb_cellules, sizeof(bool));
        m->frontier_nodes = malloc(sizeof(int) * nb_cellules);
        m->frontier_size = 0;

        m->prochaine_position = nb_cellules - 1 - i * 2;

        m->move_cooldown = 5;//i * 2;
    }

    bool quitter = false;
    SDL_Event e;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitter = true;
            }
            if (e.type == SDL_KEYDOWN) {
                int nouvelle_pos = joueur_pos;
                int direction_flag = 0;

                switch (e.key.keysym.sym) {
                    case SDLK_UP:    nouvelle_pos -= colonnes; direction_flag = 1; break;
                    case SDLK_DOWN:  nouvelle_pos += colonnes; direction_flag = 4; break;
                    case SDLK_LEFT:  nouvelle_pos -= 1;        direction_flag = 8; break;
                    case SDLK_RIGHT: nouvelle_pos += 1;        direction_flag = 2; break;
                }
                // Vérifier si le mouvement est valide (pas de mur)
                if (!(murs_reels[joueur_pos] & direction_flag)) {
                    joueur_pos = nouvelle_pos;
                }
            }
        }


        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            mettre_a_jour_monstre(&monstres[i], joueur_pos, murs_reels, lignes, colonnes);
            if (monstres[i].pos == joueur_pos) { printf("GAME OVER !\n"); quitter = true; }
        }

        // --- DESSIN ---
        SDL_SetRenderDrawColor(rendu, 20, 0, 30, 255);
        SDL_RenderClear(rendu);
        SDL_SetRenderDrawColor(rendu, 100, 80, 200, 255);
        for (int i = 0; i < nb_cellules; i++) dessiner_murs(rendu, i % colonnes, i / colonnes, murs_reels, colonnes);
        dessiner_personnage(rendu, perso_texture, (joueur_pos % colonnes + 0.5f) * TAILLE_CELLULE, (joueur_pos / colonnes + 0.5f) * TAILLE_CELLULE);
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            if(monstres[i].mode == AI_MODE_HUNT) SDL_SetTextureColorMod(monstre_texture, 255, 100, 100); // Rouge
            else if (monstres[i].mode == AI_MODE_SEARCH_ZONE) SDL_SetTextureColorMod(monstre_texture, 255, 255, 100); // Jaune
            else SDL_SetTextureColorMod(monstre_texture, 100, 255, 100); // Vert
            dessiner_personnage(rendu, monstre_texture, (monstres[i].pos % colonnes + 0.5f) * TAILLE_CELLULE, (monstres[i].pos / colonnes + 0.5f) * TAILLE_CELLULE);
        }
        SDL_RenderPresent(rendu);
    }

    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        free(monstres[i].murs_connus);
        free(monstres[i].memoire_murs);
        free(monstres[i].noeuds_visites_zone);
        free(monstres[i].frontier_nodes);
        free(monstres[i].chemin_actuel);
    }
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyTexture(monstre_texture);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre); SDL_Quit();
}

int main() {
    srand(time(NULL));
    int lignes = 20;
    int colonnes = 30;
    int nb_cellules = lignes * colonnes;
    
    arete *toutes_aretes;
    int nb_total_aretes = generation_grille_vide(&toutes_aretes, lignes, colonnes);
    fisher_yates(toutes_aretes, nb_total_aretes);
    arete *arbre = malloc(sizeof(arete) * (nb_cellules - 1));
    int nb_aretes_arbre;
    construire_arbre_couvrant(toutes_aretes, nb_total_aretes, arbre, &nb_aretes_arbre, nb_cellules);
    free(toutes_aretes);
    
    int *murs_reels = malloc(sizeof(int) * nb_cellules);
    for (int i = 0; i < nb_cellules; i++) murs_reels[i] = 1 | 2 | 4 | 8;
    for (int i = 0; i < nb_aretes_arbre; i++) supprimer_mur(murs_reels, colonnes, arbre[i].u, arbre[i].v);
    free(arbre);
    lancer_jeu(murs_reels, lignes, colonnes);
    free(murs_reels);
    printf("Programme terminé.\n");
    return 0;
}