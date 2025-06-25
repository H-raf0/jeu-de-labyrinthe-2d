#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "laby.h"
#include "labySDL.h"

// --- Définitions pour le mode de jeu à 2 ÉTATS ---
#define AI_MODE_IDLE 0        // Le monstre n'a aucune information et attend
#define AI_MODE_SEARCH_ZONE 1 // Le monstre a une piste et explore une zone
#define AI_MODE_HUNT 2        // Le monstre voit le joueur et le chasse

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
    int* chemin_actuel;
    int etape_chemin;
    int nb_etapes_chemin;
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

// "Cerveau" de l'IA: prend UNE décision et la prépare pour exécution
void mettre_a_jour_monstre(Monstre* monstre, int joueur_pos, int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;

    // --- A. PERCEPTION & DÉCISION DU MODE ---
    int j_x, j_y, m_x, m_y;
    indice_vers_coord(joueur_pos, colonnes, &j_x, &j_y);
    indice_vers_coord(monstre->pos, colonnes, &m_x, &m_y);
    int distance = abs(j_x - m_x) + abs(j_y - m_y);

    if (monstre->timer_piste > 0) monstre->timer_piste--;

    int old_mode = monstre->mode;
    
    // Priorité 1 : La chasse. Si le joueur est assez proche, on le chasse.
    if (distance <= SEUIL_DETECTION_HUNT) {
        monstre->mode = AI_MODE_HUNT;
        monstre->timer_piste = DUREE_PISTE; // La piste est fraîche
    } 
    // Priorité 2 : La recherche. Si on a perdu le joueur mais la piste est chaude.
    else {
        monstre->mode = AI_MODE_SEARCH_ZONE;
    }

    // Si on vient de changer de mode, on annule le plan en cours
    if (monstre->mode != old_mode) {
        monstre->nb_etapes_chemin = 0; 
        if(monstre->mode == AI_MODE_SEARCH_ZONE) {
            printf("Monstre %d -> MODE RECHERCHE DE ZONE\n", monstre->pos);
            // On définit la zone de recherche autour de la position REELLE du joueur (comme s'il entendait un bruit)
            int zone_centre_x = j_x;
            int zone_centre_y = j_y;
            monstre->frontier_size = 0;
            memset(monstre->noeuds_visites_zone, 0, sizeof(bool) * nb_cellules);
            monstre->frontier_nodes[monstre->frontier_size++] = monstre->pos;
        }
    }

    // --- B. PLANIFICATION (uniquement si le monstre n'a pas de plan) ---
    if (monstre->nb_etapes_chemin == 0) {
        noeud plan;
        switch (monstre->mode) {
            case AI_MODE_HUNT:
                BFS_laby(monstre->murs_connus, lignes, colonnes, joueur_pos, &plan);
                monstre->nb_etapes_chemin = reconstruire_chemin_inverse(&plan, monstre->pos, joueur_pos, nb_cellules, monstre->chemin_actuel);
                monstre->etape_chemin = 1;
                free_noeuds(&plan);
                break;
            case AI_MODE_SEARCH_ZONE:
            { 
                // La logique d'exploration est un "plan" qui se recalcule à chaque pas.
                // Donc on la met directement dans la section ACTION.
                // On s'assure juste de ne pas être bloqué.
                if (monstre->frontier_size == 0) {
                    monstre->timer_piste = 0; // La zone a été explorée, la piste est froide.
                }
            }
            break;
            case AI_MODE_IDLE:
                // Ne fait rien, n'a pas de plan.
                break;
        }
    }

    // --- C. ACTION ---
    if (monstre->move_cooldown > 0) { monstre->move_cooldown--; return; }
    monstre->move_cooldown = VITESSE_MONSTRE;

    int prochain_pas = -1;
    if (monstre->mode == AI_MODE_HUNT) {
        if (monstre->etape_chemin < monstre->nb_etapes_chemin) {
            prochain_pas = monstre->chemin_actuel[monstre->etape_chemin];
        }
    } else if (monstre->mode == AI_MODE_SEARCH_ZONE) {
        // Logique de recherche "pas à pas"
        // 1. Mise à jour de la frontière
        monstre->noeuds_visites_zone[monstre->pos] = true;
        // ... (Logique pour ajouter les voisins valides à la frontière)
        for (int k = 0; k < monstre->frontier_size; k++) { if(monstre->frontier_nodes[k] == monstre->pos){monstre->frontier_nodes[k] = monstre->frontier_nodes[--monstre->frontier_size]; break;}}

        // 2. Trouver la cible sur la frontière
        noeud plan_vers_frontiere;
        BFS_laby(monstre->murs_connus, lignes, colonnes, monstre->pos, &plan_vers_frontiere);
        int target_node = -1; int min_dist = INF;
        
        for (int k = 0; k < monstre->frontier_size; k++) {
        // On récupère un noeud candidat depuis la liste de la frontière
        int node_f = monstre->frontier_nodes[k];
        
            // On regarde dans notre plan fraîchement calculé quelle est la distance pour l'atteindre
            if (plan_vers_frontiere.distance[node_f] < min_dist) {
                // Si c'est la distance la plus courte trouvée jusqu'à présent,
                // ce noeud devient notre nouvelle cible prioritaire.
                min_dist = plan_vers_frontiere.distance[node_f];
                target_node = node_f;
            }
        }
        free_noeuds(&plan_vers_frontiere);

        // 3. Trouver le premier pas vers cette cible
        if (target_node != -1) {
            noeud plan_vers_cible;
            BFS_laby(monstre->murs_connus, lignes, colonnes, target_node, &plan_vers_cible);
            if (plan_vers_cible.distance[monstre->pos] != INF) prochain_pas = plan_vers_cible.parent[monstre->pos];
            free_noeuds(&plan_vers_cible);
        } else {
             monstre->timer_piste = 0; // Zone explorée
        }
    }

    if (prochain_pas != -1) {
        int diff = prochain_pas - monstre->pos;
        int dir_flag = 0;
        if (diff == -colonnes) dir_flag = 1; else if (diff == 1) dir_flag = 2; else if (diff == colonnes) dir_flag = 4; else if (diff == -1) dir_flag = 8;
        
        if (murs_reels[monstre->pos] & dir_flag) {
            apprendre_mur(monstre, monstre->pos, prochain_pas, colonnes);
            monstre->nb_etapes_chemin = 0; // Invalider le plan
        } else {
            monstre->pos = prochain_pas;
            if(monstre->mode == AI_MODE_HUNT) monstre->etape_chemin++;
            if(monstre->etape_chemin >= monstre->nb_etapes_chemin) monstre->nb_etapes_chemin = 0;
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
        m->mode = AI_MODE_IDLE;
        m->timer_piste = 0;
        m->murs_connus = calloc(nb_cellules, sizeof(int));
        m->memoire_murs = malloc(sizeof(arete) * MEMOIRE_MAX);
        m->memoire_tete = 0; m->memoire_queue = 0; m->memoire_taille_actuelle = 0;
        
        m->noeuds_visites_zone = calloc(nb_cellules, sizeof(bool));
        m->frontier_nodes = malloc(sizeof(int) * nb_cellules);
        m->frontier_size = 0;

        m->chemin_actuel = malloc(sizeof(int) * nb_cellules);
        m->nb_etapes_chemin = 0;
        m->etape_chemin = 0;
        m->move_cooldown = i * 2;
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