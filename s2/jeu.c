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
#define SEUIL_DETECTION_HUNT 6      // Portée de la vue directe
#define DUREE_PISTE 500             // La piste reste "chaude" pendant . frames
#define RAPP_CLDWN 100
#define MEMOIRE_MAX 99999
#define VITESSE_MONSTRE 20


#define MONSTRE_PENALITE_RAYON 4 // How far the penalty spreads from a monster.
#define MONSTRE_PENALITE_COUT 20 // The base cost added to a cell near another monster.


#define SAUT_COOLDOWN 0

#define NOMBRE_PIECES 3 // Le joueur doit collecter . pièces pour gagner

#define HEIGHT 20
#define WIDTH 20


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



typedef struct {
    int pos;
    int direction; // La direction vers laquelle le joueur fait face (1:Haut, 2:Droite, 4:Bas, 8:Gauche)
    int saut_cooldown;
} Joueur;




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
int gidc(Monstre* monstre, int* murs_reels, int lignes, int colonnes, int destination, const int* penalite_map) {
    int nb_cellules = lignes * colonnes;
    int depart = monstre->pos;

    // Planifier le chemin sur la carte connue
    int** graphe_connu = creer_matrice_couts_dynamiques(monstre->murs_connus, penalite_map, lignes, colonnes);
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



int gidi(Monstre* monstre, int* murs_reels, int lignes, int colonnes, const int* penalite_map) {
    int nb_cellules = lignes * colonnes;

    // --- 1. Observer les environs et mettre à jour la frontière ---
    // Cette partie reste inchangée, car elle ne fait pas de pathfinding.
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

    if (monstre->frontier_size == 0) return monstre->pos;

    // --- 2. Trouver la cible la plus proche sur la frontière avec DIJKSTRA ---
    noeud plan_vers_frontiere;
    

    //BFS_laby(monstre->murs_connus, lignes, colonnes, monstre->pos, &plan_vers_frontiere);

    // On crée une matrice d'adjacence à partir de la connaissance du monstre
    int** graphe_connu_1 = creer_matrice_couts_dynamiques(monstre->murs_connus, penalite_map, lignes, colonnes);
    if (!graphe_connu_1) { return monstre->pos; } // Sécurité
    // On lance Dijkstra depuis la position actuelle
    Dijkstra_laby(graphe_connu_1, nb_cellules, monstre->pos, &plan_vers_frontiere);
    // On libère la mémoire de la matrice
    liberer_matrice_adjacence(graphe_connu_1, nb_cellules);

    int target_node = -1;
    int min_dist = INF;
    for (int i = 0; i < monstre->frontier_size; i++) {
        int node_f = monstre->frontier_nodes[i];
        if (plan_vers_frontiere.distance[node_f] < min_dist) {
            min_dist = plan_vers_frontiere.distance[node_f];
            target_node = node_f;
        }
    }
    free_noeuds(&plan_vers_frontiere); // On libère le plan qui n'est plus utile
    if (target_node == -1) return monstre->pos;

    // --- 3. Calculer le premier pas vers cette cible avec DIJKSTRA ---
    noeud plan_vers_cible;

    // On recrée une matrice d'adjacence (nécessaire car la première a été libérée)
    int** graphe_connu_2 = creer_matrice_couts_dynamiques(monstre->murs_connus, penalite_map, lignes, colonnes);
    if (!graphe_connu_2) { return monstre->pos; } // Sécurité
    // On lance Dijkstra depuis la cible pour trouver le chemin retour
    Dijkstra_laby(graphe_connu_2, nb_cellules, target_node, &plan_vers_cible);
    // On libère la mémoire
    liberer_matrice_adjacence(graphe_connu_2, nb_cellules);

    int prochain_pas_planifie = -1;
    if (plan_vers_cible.distance[monstre->pos] != INF) {
        prochain_pas_planifie = plan_vers_cible.parent[monstre->pos];
    }
    free_noeuds(&plan_vers_cible);

    if (prochain_pas_planifie == -1 || prochain_pas_planifie == monstre->pos) {
        return monstre->pos;
    }
    
    // --- 4. Vérifier le mouvement contre la réalité ---
    int diff = prochain_pas_planifie - monstre->pos;
    int direction_flag = 0;
    if (diff == -colonnes) direction_flag = 1; else if (diff == 1) direction_flag = 2; else if (diff == colonnes) direction_flag = 4; else if (diff == -1) direction_flag = 8;

    if (murs_reels[monstre->pos] & direction_flag) {
        apprendre_mur(monstre, monstre->pos, prochain_pas_planifie, colonnes);
        return monstre->pos;
    } else {
        return prochain_pas_planifie;
    }
}


void mettre_a_jour_monstre(Monstre* monstres, int monstre_index, int joueur_pos, int* murs_reels, int lignes, int colonnes) {
    Monstre* monstre = &monstres[monstre_index];
    int nb_cellules = lignes * colonnes;
    
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


    int* penalite_map = calloc(nb_cellules, sizeof(int));
    if (!penalite_map) return; // Sécurité

    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        if (i == monstre_index) continue; // Ne pas se pénaliser soi-même

        int autre_monstre_pos = monstres[i].pos;
        int am_x, am_y;
        indice_vers_coord(autre_monstre_pos, colonnes, &am_x, &am_y);

        // Appliquer une pénalité dans un rayon autour de l'autre monstre
        for (int y_p = am_y - MONSTRE_PENALITE_RAYON; y_p <= am_y + MONSTRE_PENALITE_RAYON; y_p++) {
            for (int x_p = am_x - MONSTRE_PENALITE_RAYON; x_p <= am_x + MONSTRE_PENALITE_RAYON; x_p++) {
                if (x_p >= 0 && x_p < colonnes && y_p >= 0 && y_p < lignes) {
                    int dist = abs(x_p - am_x) + abs(y_p - am_y);
                    if (dist <= MONSTRE_PENALITE_RAYON) {
                        int penalite = MONSTRE_PENALITE_COUT * (MONSTRE_PENALITE_RAYON - dist) / MONSTRE_PENALITE_RAYON;
                        int cell_idx = y_p * colonnes + x_p;
                        penalite_map[cell_idx] += penalite; // On ajoute, au cas où plusieurs monstres sont proches
                    }
                }
            }
        }
    }

    // --- PLANIFICATION ET MISE À JOUR DE la `prochaine_position` ---
    int prochaine_position_planifiee = monstre->pos;
    
    switch (monstre->mode) {
        case AI_MODE_HUNT:
            // GIDC: Calcule le prochain pas optimal vers le joueur
            prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, joueur_pos, penalite_map);
            monstre->rapp_cooldown = 0;
            break;
        case AI_MODE_SEARCH_ZONE:
            // On vérifie si les conditions de repositionnement sont remplies
            
            // 2. Phase de déplacement ou d'exploration
            if (monstre->pos != monstre->drnier_pos_jr_connu && monstre->rapp_cooldown == 0) {
                // Si on n'est pas encore à notre point de patrouille, on s'y dirige.
                prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, monstre->drnier_pos_jr_connu, penalite_map);
                
                /*
                // Si on est bloqué en route, on explore pour trouver une issue
                if (prochaine_position_planifiee == monstre->pos) {
                    printf("Monstre %d : Bloqué en route vers la cible de patrouille, exploration GIDI en secours.\n", monstre->pos);
                    prochaine_position_planifiee = gidi(monstre, murs_reels, lignes, colonnes);
                }*/

                if (prochaine_position_planifiee == monstre->drnier_pos_jr_connu) {
                    monstre->rapp_cooldown = RAPP_CLDWN;
                }
            } else {
                // On est arrivé à notre point de patrouille, on explore localement avec GIDI.
                //printf("Monstre %d : Cible de patrouille atteinte, exploration locale (GIDI).\n", monstre->pos);
                prochaine_position_planifiee = gidi(monstre, murs_reels, lignes, colonnes, penalite_map);
                if (monstre->rapp_cooldown >= 1){
                    printf("\ncooldown :%d\n", monstre->rapp_cooldown);
                    monstre->rapp_cooldown--;
                }
                if (monstre->rapp_cooldown == 1){
                    //monstre->rapp_cooldown = RAPP_CLDWN;
                    monstre->drnier_pos_jr_connu = joueur_pos;
                }
            }
            /*
            // 3. Dernier recours si GIDI ne trouve rien à explorer
            if (prochaine_position_planifiee == monstre->pos) { 
                printf("Monstre %d a fini sa recherche locale, choisit une destination de patrouille aléatoire.\n", monstre->pos);
                monstre->drnier_pos_jr_connu = joueur_pos; // La nouvelle cible est aléatoire
                prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, monstre->drnier_pos_jr_connu);
            }*/
            break;
    }

    // Libérer la mémoire de la carte de pénalités
    free(penalite_map);

    // --- EXÉCUTION DU MOUVEMENT ---  
    monstre->pos = prochaine_position_planifiee;
}




// Fonction de jeu principale
void lancer_jeu(int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;

    // --- Initialisation SDL ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image initialization failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    if (!init_audio_system()) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Obtenir les dimensions de l'écran
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }

    // Remplir les dimensions de la fenêtre dans notre config
    g_config.window_w = dm.w;
    g_config.window_h = dm.h;

    // 2. Créer une fenêtre en plein écran
    SDL_Window* fenetre = SDL_CreateWindow("Le Jeu", 
                                           SDL_WINDOWPOS_CENTERED, 
                                           SDL_WINDOWPOS_CENTERED, 
                                           g_config.window_w + 1, 
                                           g_config.window_h, 
                                           SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!fenetre) {
        printf("erreur lors de la creation du fenêtre\n");
        return;
    }
    
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    
    // Calculer les dimensions proportionnelles
    // Calcul de la taille de cellule pour que tout le labyrinthe rentre
    int cell_w = g_config.window_w / colonnes;
    int cell_h = g_config.window_h / lignes;
    g_config.cell_size = (cell_w < cell_h) ? cell_w : cell_h; // On prend la plus petite pour garder les proportions

    // L'épaisseur des murs est une fraction de la taille de la cellule
    // On s'assure qu'elle est d'au moins 1 pixel.
    g_config.wall_thickness = (int)(g_config.cell_size / 16.0f);
    if (g_config.wall_thickness < 1) g_config.wall_thickness = 1;

    // Calcul des marges pour centrer le labyrinthe dans la fenêtre
    int laby_pixel_width = colonnes * g_config.cell_size;
    int laby_pixel_height = lignes * g_config.cell_size;
    g_config.offset_x = (g_config.window_w - laby_pixel_width) / 2;
    g_config.offset_y = (g_config.window_h - laby_pixel_height) / 2;

    
    
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");
    SDL_Texture* monstre_texture = IMG_LoadTexture(rendu, "monstre.png");
    SDL_Texture* piece_texture = IMG_LoadTexture(rendu, "piece.png");


    // Initialisation du Joueur
    Joueur joueur;
    joueur.pos = 0;
    joueur.direction = 4; // Commence en regardant vers le bas
    joueur.saut_cooldown = 0;

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
        m->rapp_cooldown = 0;
        m->move_cooldown = 1;//i * 2;
    }

    
    //generation des pieces
    int* pieces_pos = malloc(sizeof(int) * NOMBRE_PIECES);
    int pieces_collectees = 0;
    printf("Génération de %d pièces...\n", NOMBRE_PIECES);
    for (int i = 0; i < NOMBRE_PIECES; i++) {
        int pos;
        bool position_valide;
        do {
            position_valide = true;
            pos = rand() % nb_cellules; // Position aléatoire

            // Vérifier que la position n'est pas celle du joueur
            if (pos == joueur.pos) { position_valide = false; continue; }

            // Vérifier que la position n'est pas celle d'un monstre
            for(int j = 0; j < NOMBRE_MONSTRES; j++) {
                if (pos == monstres[j].pos) { position_valide = false; break; }
            }
            if (!position_valide) continue;

            // Vérifier que la position n'est pas déjà prise par une autre pièce
            for (int k = 0; k < i; k++) {
                if (pos == pieces_pos[k]) { position_valide = false; break; }
            }
        } while (!position_valide);
        pieces_pos[i] = pos;
    }


    bool quitter = false;
    SDL_Event e;
    while (!quitter) {
        // Décrémenter le cooldown du saut du joueur à chaque frame
        if (joueur.saut_cooldown > 0) {
            joueur.saut_cooldown--;
        }
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitter = true;
            }
            if (e.type == SDL_KEYDOWN) {
                int nouvelle_pos = joueur.pos;
                int direction_flag = 0;

                switch (e.key.keysym.sym) {
                    case SDLK_UP:    nouvelle_pos -= colonnes; direction_flag = 1; break;
                    case SDLK_DOWN:  nouvelle_pos += colonnes; direction_flag = 4; break;
                    case SDLK_LEFT:  nouvelle_pos -= 1;        direction_flag = 8; break;
                    case SDLK_RIGHT: nouvelle_pos += 1;        direction_flag = 2; break;
                    case SDLK_SPACE: 
                        // On ne peut sauter que si le cooldown est terminé
                        if (joueur.saut_cooldown == 0) {
                            int pos_apres_saut = -1;
                            // On vérifie s'il y a bien un mur dans la direction où le joueur regarde et qu'il ne sort pas du labyrinthe
                            if (murs_reels[joueur.pos] & joueur.direction) {
                                // Calculer la position de l'autre côté du mur
                                if (joueur.direction == 1) pos_apres_saut = joueur.pos - colonnes; // HAUT
                                else if (joueur.direction == 4) pos_apres_saut = joueur.pos + colonnes; // BAS
                                else if (joueur.direction == 8) pos_apres_saut = joueur.pos - 1;       // GAUCHE
                                else if (joueur.direction == 2) pos_apres_saut = joueur.pos + 1;       // DROITE
                                
                                // Vérifier que la case d'atterrissage est valide (dans le labyrinthe)
                                int x, y;
                                indice_vers_coord(pos_apres_saut, colonnes, &x, &y);
                                if (x >= 0 && x < colonnes && y >= 0 && y < lignes) {
                                    if(((joueur.direction == 2 || joueur.direction == 8) && joueur.pos/colonnes == y) || joueur.direction==1 || joueur.direction==4) {
                                    printf("Le joueur a sauté par-dessus un mur !\n");
                                    joueur.pos = pos_apres_saut;
                                    joueur.saut_cooldown = SAUT_COOLDOWN; // Activer le cooldown
                                    }
                                }
                            }
                        }
                        // On met le flag à 0 pour ne pas entrer dans la logique de marche normale
                        direction_flag = 0;
                        break;
                }
                // Si une touche de direction a été pressée
                if (direction_flag != 0) {
                    // On met à jour la direction dans laquelle le joueur regarde
                    joueur.direction = direction_flag;
                    // On vérifie si le mouvement est valide (pas de mur)
                    if (!(murs_reels[joueur.pos] & direction_flag)) {
                        joueur.pos = nouvelle_pos;
                    }
                }
            }
        }

        // --- LOGIQUE DE COLLECTE DES PIÈCES ---
        for (int i = 0; i < NOMBRE_PIECES; i++) {
            // Si la pièce existe encore et que le joueur est dessus
            if (pieces_pos[i] != -1 && joueur.pos == pieces_pos[i]) {
                pieces_pos[i] = -1; // "Supprimer" la pièce
                pieces_collectees++;
                printf("Pièce collectée ! (%d / %d)\n", pieces_collectees, NOMBRE_PIECES);
            }
        }
        
        // --- CONDITION DE VICTOIRE ---
        if (pieces_collectees == NOMBRE_PIECES) {
            printf("VICTOIRE ! Toutes les pièces ont été collectées !\n");
            quitter = true; // Termine la boucle de jeu
        }

        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            mettre_a_jour_monstre(monstres, i, joueur.pos, murs_reels, lignes, colonnes);
            if (monstres[i].pos == joueur.pos) { printf("GAME OVER !\n"); quitter = true; }
        }

        // --- DESSIN ---
        SDL_SetRenderDrawColor(rendu, 20, 0, 30, 255);
        SDL_RenderClear(rendu);

        //dessin de bg
        dessiner_bg(rendu, lignes, colonnes);

        for (int i = 0; i < NOMBRE_PIECES; i++) {
            if (pieces_pos[i] != -1) {
                // Calculer les coordonnées en pixels de la pièce en utilisant g_config
                float piece_px = g_config.offset_x + (pieces_pos[i] % colonnes + 0.5f) * g_config.cell_size;
                float piece_py = g_config.offset_y + (pieces_pos[i] / colonnes + 0.5f) * g_config.cell_size;
                
                // On réutilise la fonction dessiner_personnage, car elle dessine un sprite à une position
                dessiner_personnage(rendu, piece_texture, piece_px, piece_py);
            }
        }


        SDL_SetRenderDrawColor(rendu, 100, 80, 200, 255);
        

        for (int i = 0; i < nb_cellules; i++) dessiner_murs_connus(rendu, i % colonnes, i / colonnes, murs_reels, colonnes);

        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            if (monstres[i].mode == AI_MODE_HUNT) {
                dessiner_rayon_detection(rendu, monstres[i].pos, SEUIL_DETECTION_HUNT, lignes, colonnes);
            }
        }

        
        float joueur_px = g_config.offset_x + (joueur.pos % colonnes + 0.5f) * g_config.cell_size;
        float joueur_py = g_config.offset_y + (joueur.pos / colonnes + 0.5f) * g_config.cell_size;
        dessiner_personnage(rendu, perso_texture, joueur_px, joueur_py);

        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            if(monstres[i].mode == AI_MODE_HUNT) SDL_SetTextureColorMod(monstre_texture, 255, 100, 100); // Rouge
            else if (monstres[i].mode == AI_MODE_SEARCH_ZONE) SDL_SetTextureColorMod(monstre_texture, 100, 255, 100); // Jaune
            float monstre_px = g_config.offset_x + (monstres[i].pos % colonnes + 0.5f) * g_config.cell_size;
            float monstre_py = g_config.offset_y + (monstres[i].pos / colonnes + 0.5f) * g_config.cell_size;
            dessiner_personnage(rendu, monstre_texture, monstre_px, monstre_py);
        }
        SDL_RenderPresent(rendu);
    }

    free(pieces_pos); // libérer le tableau des pièces

    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        free(monstres[i].murs_connus);
        free(monstres[i].memoire_murs);
        free(monstres[i].noeuds_visites_zone);
        free(monstres[i].frontier_nodes);
    }
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyTexture(monstre_texture);
    SDL_DestroyTexture(piece_texture); 
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre); SDL_Quit();
}

int main() {
    unsigned int seed = time(NULL);
    srand(seed);
    printf("seed de la labyrinth est : %u\n", seed);
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