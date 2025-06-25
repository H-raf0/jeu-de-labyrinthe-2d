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
#define SEUIL_DETECTION_HUNT 3      // Portée de la vue directe
#define DUREE_PISTE 10000             // La piste reste "chaude" pendant . frames
#define SEUIL_LAISSE 15 // Si le joueur est plus loin que 15 cases, le monstre se rapproche
#define RAPP_CLDWN 500
#define MEMOIRE_MAX 9999
#define VITESSE_MONSTRE 10

#define HEIGHT 20
#define WIDTH 20

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif


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

    int drnier_pos_jr_connu;

    int rapp_cooldown;
    int move_cooldown;
} Monstre;


void melanger_voisins(int* tableau, int n) {
    if (n > 1) {
        for (int i = n - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int temp = tableau[i];
            tableau[i] = tableau[j];
            tableau[j] = temp;
        }
    }
}


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



// GIDC: Graphe Inconnu, Destination Connue
int gidc(Monstre* monstre, int* murs_reels, int lignes, int colonnes, int destination) {
    int nb_cellules = lignes * colonnes;
    int depart = monstre->pos;

    // Planifier le chemin sur la carte connue
    int** graphe_connu = creer_matrice_adjacence_connue(monstre->murs_connus, lignes, colonnes);
    if (!graphe_connu) { return depart; } 

    noeud plan;
    Dijkstra_laby(graphe_connu, nb_cellules, destination, &plan);
    
    // Trouver le meilleur voisin pour se rapprocher
    int meilleur_voisin = -1;
    int min_dist_estimee = INF;
    int x_act, y_act;
    indice_vers_coord(depart, colonnes, &x_act, &y_act);
    int voisins[4] = {
        (y_act > 0) ? (y_act - 1) * colonnes + x_act : -1, (x_act < colonnes - 1) ? y_act * colonnes + (x_act + 1) : -1,
        (y_act < lignes - 1) ? (y_act + 1) * colonnes + x_act : -1, (x_act > 0) ? y_act * colonnes + (x_act - 1) : -1
    };

    for (int i = 0; i < 4; i++) {
        int v = voisins[i]; 
        if (v == -1) continue;
        
        int diff_temp = v - depart; 
        int dir_flag_temp = 0;
        if (diff_temp == -colonnes) dir_flag_temp = 1; else if (diff_temp == 1) dir_flag_temp = 2; else if (diff_temp == colonnes) dir_flag_temp = 4; else if (diff_temp == -1) dir_flag_temp = 8;
        
        if (!(monstre->murs_connus[depart] & dir_flag_temp)) {
            if (plan.distance[v] < min_dist_estimee) { 
                min_dist_estimee = plan.distance[v]; 
                meilleur_voisin = v; 
            }
        }
    }
    free_noeuds(&plan);
    liberer_matrice_adjacence(graphe_connu, nb_cellules);

    if (meilleur_voisin == -1) { 
        return depart; // Bloqué selon sa carte, il ne bouge pas
    }

    // Valider le mouvement contre la réalité
    int diff = meilleur_voisin - depart; 
    int direction_flag = 0;
    if (diff == -colonnes) direction_flag = 1; else if (diff == 1) direction_flag = 2; else if (diff == colonnes) direction_flag = 4; else if (diff == -1) direction_flag = 8;

    if (murs_reels[depart] & direction_flag) {
        // Collision ! Apprendre et ne pas bouger.
        apprendre_mur(monstre, depart, meilleur_voisin, colonnes);
        return depart;
    } else {
        // Pas de collision, le mouvement est valide.
        return meilleur_voisin;
    }
}


int gidi(Monstre* monstre, int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;

    // --- 1. Observer les environs et mettre à jour la frontière ---
    // (Cette partie reste identique)
    monstre->noeuds_visites_zone[monstre->pos] = true;
    for (int k = 0; k < monstre->frontier_size; k++) {
        if (monstre->frontier_nodes[k] == monstre->pos) {
            monstre->frontier_nodes[k] = monstre->frontier_nodes[--monstre->frontier_size];
            break;
        }
    }
    int x_act, y_act;
    indice_vers_coord(monstre->pos, colonnes, &x_act, &y_act);
    int voisins[4] = {
        (y_act > 0) ? (y_act - 1) * colonnes + x_act : -1, (x_act < colonnes - 1) ? y_act * colonnes + (x_act + 1) : -1,
        (y_act < lignes - 1) ? (y_act + 1) * colonnes + x_act : -1, (x_act > 0) ? y_act * colonnes + (x_act - 1) : -1
    };
    melanger_voisins(voisins, 4);
    for (int i = 0; i < 4; i++) {
        int v = voisins[i];
        if (v == -1) continue;
        int diff = v - monstre->pos;
        int dir_flag = 0;
        if (diff == -colonnes) dir_flag = 1; else if (diff == 1) dir_flag = 2; else if (diff == colonnes) dir_flag = 4; else if (diff == -1) dir_flag = 8;
        if (murs_reels[monstre->pos] & dir_flag) {
            apprendre_mur(monstre, monstre->pos, v, colonnes);
        } else {
            if (!monstre->noeuds_visites_zone[v]) {
                bool deja_dans_frontiere = false;
                for (int j = 0; j < monstre->frontier_size; j++) { if (monstre->frontier_nodes[j] == v) { deja_dans_frontiere = true; break; } }
                if (!deja_dans_frontiere) { monstre->frontier_nodes[monstre->frontier_size++] = v; }
            }
        }
    }

    if (monstre->frontier_size == 0) return monstre->pos; // Exploration finie, on ne bouge pas.

    // --- 2. Trouver la cible la plus proche sur la frontière ---
    // (Cette partie reste identique)
    noeud plan_vers_frontiere;
    BFS_laby(monstre->murs_connus, lignes, colonnes, monstre->pos, &plan_vers_frontiere);
    int target_node = -1;
    int min_dist = INF;
    for (int i = 0; i < monstre->frontier_size; i++) {
        int node_f = monstre->frontier_nodes[i];
        if (plan_vers_frontiere.distance[node_f] < min_dist) {
            min_dist = plan_vers_frontiere.distance[node_f];
            target_node = node_f;
        }
    }
    free_noeuds(&plan_vers_frontiere);
    if (target_node == -1) return monstre->pos; // Bloqué, on ne bouge pas.

    // --- 3. Calculer le premier pas vers cette cible ---
    // (Cette partie reste identique)
    noeud plan_vers_cible;
    BFS_laby(monstre->murs_connus, lignes, colonnes, target_node, &plan_vers_cible);
    int prochain_pas_planifie = -1;
    if (plan_vers_cible.distance[monstre->pos] != INF) {
        prochain_pas_planifie = plan_vers_cible.parent[monstre->pos];
    }
    free_noeuds(&plan_vers_cible);

    if (prochain_pas_planifie == -1 || prochain_pas_planifie == monstre->pos) {
        return monstre->pos; // Pas de chemin trouvé ou déjà sur place, on ne bouge pas.
    }
    
    // --- 4. NOUVELLE PARTIE : VÉRIFIER LE MOUVEMENT CONTRE LA RÉALITÉ ---
    int diff = prochain_pas_planifie - monstre->pos;
    int direction_flag = 0;
    if (diff == -colonnes) direction_flag = 1; else if (diff == 1) direction_flag = 2; else if (diff == colonnes) direction_flag = 4; else if (diff == -1) direction_flag = 8;

    if (murs_reels[monstre->pos] & direction_flag) {
        // Le chemin planifié est bloqué par un mur réel inconnu !
        apprendre_mur(monstre, monstre->pos, prochain_pas_planifie, colonnes);
        return monstre->pos; // On ne bouge pas
    } else {
        // Le chemin est libre.
        return prochain_pas_planifie; // On retourne le pas valide.
    }
}


void mettre_a_jour_monstre(Monstre* monstre, int joueur_pos, int* murs_reels, int lignes, int colonnes) {
    // --- PERCEPTION & DÉCISION DU MODE ---
    int j_x, j_y, m_x, m_y;
    indice_vers_coord(joueur_pos, colonnes, &j_x, &j_y);
    indice_vers_coord(monstre->pos, colonnes, &m_x, &m_y);
    int distance = abs(j_x - m_x) + abs(j_y - m_y);

    if (monstre->timer_piste > 0) monstre->timer_piste--;
    

    int old_mode = monstre->mode;
    
    
    if (distance <= SEUIL_DETECTION_HUNT) {
        monstre->mode = AI_MODE_HUNT;
        if (old_mode != AI_MODE_HUNT) printf("Monstre %d -> MODE CHASSE (Détection directe)\n", monstre->pos);
        monstre->drnier_pos_jr_connu = joueur_pos;
        monstre->timer_piste = DUREE_PISTE;
    } 
    else if (monstre->timer_piste > 0) {
        monstre->mode = AI_MODE_HUNT;
        if (old_mode != AI_MODE_HUNT) printf("Monstre %d -> MODE CHASSE (Poursuite de la piste)\n", monstre->pos);
    } 
    else {
        monstre->mode = AI_MODE_SEARCH_ZONE;
        if (old_mode != AI_MODE_SEARCH_ZONE){
            printf("Monstre %d -> MODE RECHERCHE (Piste perdue)\n", monstre->pos);
            // On s'assure que la cible de patrouille est la position actuelle pour commencer par explorer localement
            monstre->drnier_pos_jr_connu = monstre->pos; 
            monstre->frontier_size = 0;
            memset(monstre->noeuds_visites_zone, 0, sizeof(bool) * (lignes*colonnes));
            monstre->frontier_nodes[monstre->frontier_size++] = monstre->pos;
        }
    }

    
    // --- ACTION ---
    if (monstre->move_cooldown > 0) {
        monstre->move_cooldown--;
        return; // Pas encore l'heure de bouger
    }
    monstre->move_cooldown = VITESSE_MONSTRE;

    // --- PLANIFICATION ET MISE À JOUR DE la `prochaine_position` ---
    int prochaine_position_planifiee = monstre->pos;
    
    switch (monstre->mode) {
        case AI_MODE_HUNT:
            // GIDC: Calcule le prochain pas optimal vers le joueur
            prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, joueur_pos);
            break;
        case AI_MODE_SEARCH_ZONE:
            // On vérifie si les conditions de repositionnement sont remplies
            
            // 2. Phase de déplacement ou d'exploration
            if (monstre->pos != monstre->drnier_pos_jr_connu && monstre->rapp_cooldown == 0) {
                // Si on n'est pas encore à notre point de patrouille, on s'y dirige.
                prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, monstre->drnier_pos_jr_connu);
                
                // Si on est bloqué en route, on explore pour trouver une issue
                if (prochaine_position_planifiee == monstre->pos) {
                    printf("Monstre %d : Bloqué en route vers la cible de patrouille, exploration GIDI en secours.\n", monstre->pos);
                    prochaine_position_planifiee = gidi(monstre, murs_reels, lignes, colonnes);
                }
            } else {
                // On est arrivé à notre point de patrouille, on explore localement avec GIDI.
                printf("Monstre %d : Cible de patrouille atteinte, exploration locale (GIDI).\n", monstre->pos);
                prochaine_position_planifiee = gidi(monstre, murs_reels, lignes, colonnes);
                if (monstre->rapp_cooldown > 0){
                    monstre->rapp_cooldown--;
                }else {
                    monstre->rapp_cooldown = RAPP_CLDWN;
                    monstre->drnier_pos_jr_connu = joueur_pos;
                }
            }
            
            // 3. Dernier recours si GIDI ne trouve rien à explorer
            if (prochaine_position_planifiee == monstre->pos) { 
                printf("Monstre %d a fini sa recherche locale, choisit une destination de patrouille aléatoire.\n", monstre->pos);
                monstre->drnier_pos_jr_connu = joueur_pos; // La nouvelle cible est aléatoire
                prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, monstre->drnier_pos_jr_connu);
            }
            break;
    }

    // --- EXÉCUTION DU MOUVEMENT ---  
    monstre->pos = prochaine_position_planifiee;
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
        m->drnier_pos_jr_connu = 0; // = pos de depart du jr 
        m->rapp_cooldown = RAPP_CLDWN;
        m->move_cooldown = 1;//i * 2;
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
    }
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyTexture(monstre_texture);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre); SDL_Quit();
}

int main() {
    srand(time(NULL));
    int lignes = HEIGHT;
    int colonnes = WIDTH;
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