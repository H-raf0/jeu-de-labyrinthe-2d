#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "jeu.h"

// Définitions pour le mode de jeu à 2 ÉTATS
#define AI_MODE_SEARCH_ZONE 0
#define AI_MODE_HUNT 1



int NOMBRE_MONSTRES = 3;
int SEUIL_DETECTION_HUNT = 6;
int DUREE_PISTE = 500;
int RAPP_CLDWN = 100;
int MEMOIRE_MAX = 99999;
int VITESSE_MONSTRE = 0;
int MONSTRE_PENALITE_RAYON = 4;
int MONSTRE_PENALITE_COUT = 20;

int SAUT_COOLDOWN = 30; // Cooldown en frames

int NOMBRE_PIECES = 3;




#define ASTRO_FRAME_WIDTH 16
#define ASTRO_FRAME_HEIGHT 16
#define ASTRO_FRAME_COUNT 6

#define VITESSE_DEPLACEMENT 5.0f // Vitesse du joueur en pixels par frame



#define MONSTRE_ANIM_FRAMES 6
#define VITESSE_MONSTRE_PIXELS 4.0f // Vitesse d'animation des monstres en pixels/frame
#define MONSTRE_ZOOM_FACTOR 1.5f

// Énumération pour la direction de l'animation
typedef enum {
    IDLE,
    DOWN,
    LEFT,
    RIGHT,
    UP
}
AnimDirection;


typedef struct {
    int pos;
    int mode;
    int * murs_connus;
    arete * memoire_murs;
    int memoire_tete, memoire_queue, memoire_taille_actuelle;
    int timer_piste;
    bool * noeuds_visites_zone;
    int * frontier_nodes;
    int frontier_size;
    int drnier_pos_jr_connu;
    int rapp_cooldown;
    int move_cooldown; // Cooldown de moement des monstre 

    float pixel_x, pixel_y; // Position d'affichage en pixels
    float target_x, target_y; // Position cible en pixels

    bool is_moving; // est-il en train de bouger entre deux cases ?
    AnimDirection anim_dir; // Direction de l'animation
    int anim_frame; // Frame actuelle de l'animation
    Uint32 last_frame_time; // temps du dernier frame

}
Monstre;


typedef struct {
    int pos; // Position logique (index de la case)
    int direction; // Direction logique (1:Haut, 2:Droite, 4:Bas, 8:Gauche) pour le saut
    int saut_cooldown; // Cooldown pour le saut

    SDL_Texture * texture; // Les sprites de l'astronaute
    SDL_Texture* saut_indicateur_texture;
    float pixel_x, pixel_y; // Position D'AFFICHAGE en pixels 

    float target_x, target_y; // Position cible en pixels

    bool is_moving; // Le personnage est-il en train de bouger entre deux cases ?
    AnimDirection anim_dir; // Direction de l'animation
    int anim_frame; // Frame actuelle 
    Uint32 last_frame_time; 
}
Joueur;

SDL_Texture* creer_texture_cercle(SDL_Renderer* renderer, int radius, SDL_Color color) {
    int diameter = radius * 2;
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                             SDL_TEXTUREACCESS_TARGET, diameter, diameter);
    if (!texture) {
        printf("Erreur création texture cercle: %s\n", SDL_GetError());
        return NULL;
    }

    // Activer la transparence pour le fond du cercle
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    
    // On dessine sur notre nouvelle texture
    SDL_SetRenderTarget(renderer, texture);
    
    // On met le fond en transparent
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // On dessine le cercle au centre de la texture
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer, radius + x, radius + y);
            }
        }
    }

    // On arrête de dessiner sur la texture et on retourne au renderer principal
    SDL_SetRenderTarget(renderer, NULL);
    
    return texture;
}

void melanger_voisins(int * tableau, int n) {
    if (n > 1) {
        for (int i = n - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int temp = tableau[i];
            tableau[i] = tableau[j];
            tableau[j] = temp;
        }
    }
}

void apprendre_mur(Monstre * monstre, int u, int v, int colonnes) {
    int diff = v - u;
    int dir_flag = 0;
    if (diff == -colonnes) dir_flag = 1;
    else if (diff == 1) dir_flag = 2;
    else if (diff == colonnes) dir_flag = 4;
    else if (diff == -1) dir_flag = 8;
    if (monstre -> murs_connus[u] & dir_flag) return;
    //printf("Monstre %d apprend le mur entre %d et %d.\n", monstre -> pos, u, v);
    ajouter_mur(monstre -> murs_connus, colonnes, u, v);
    if (monstre -> memoire_taille_actuelle >= MEMOIRE_MAX) {
        arete mur_oublie = monstre -> memoire_murs[monstre -> memoire_tete];
        supprimer_mur(monstre -> murs_connus, colonnes, mur_oublie.u, mur_oublie.v);
        monstre -> memoire_tete = (monstre -> memoire_tete + 1) % MEMOIRE_MAX;
        monstre -> memoire_taille_actuelle--;
    }
    monstre -> memoire_murs[monstre -> memoire_queue] = (arete) {u, v};
    monstre -> memoire_queue = (monstre -> memoire_queue + 1) % MEMOIRE_MAX;
    monstre -> memoire_taille_actuelle++;
}

int gidc(Monstre * monstre, int * murs_reels, int lignes, int colonnes, int destination, const int * penalite_map) {
    int nb_cellules = lignes * colonnes;
    int depart = monstre -> pos;
    int ** graphe_connu = creer_matrice_couts_dynamiques(monstre -> murs_connus, penalite_map, lignes, colonnes);
    if (!graphe_connu) {
        return depart;
    }
    noeud plan;
    Dijkstra_laby(graphe_connu, nb_cellules, destination, & plan);
    int meilleur_voisin = -1;
    int min_dist_estimee = INF;
    int x_act, y_act;
    indice_vers_coord(depart, colonnes, & x_act, & y_act);
    int voisins[4] = {
        (y_act > 0) ? (y_act - 1) * colonnes + x_act : -1,
        (x_act < colonnes - 1) ? y_act * colonnes + (x_act + 1) : -1,
        (y_act < lignes - 1) ? (y_act + 1) * colonnes + x_act : -1,
        (x_act > 0) ? y_act * colonnes + (x_act - 1) : -1
    };
    for (int i = 0; i < 4; i++) {
        int v = voisins[i];
        if (v == -1) continue;
        int diff_temp = v - depart;
        int dir_flag_temp = 0;
        if (diff_temp == -colonnes) dir_flag_temp = 1;
        else if (diff_temp == 1) dir_flag_temp = 2;
        else if (diff_temp == colonnes) dir_flag_temp = 4;
        else if (diff_temp == -1) dir_flag_temp = 8;
        if (!(monstre -> murs_connus[depart] & dir_flag_temp)) {
            if (plan.distance[v] < min_dist_estimee) {
                min_dist_estimee = plan.distance[v];
                meilleur_voisin = v;
            }
        }
    }
    free_noeuds( & plan);
    liberer_matrice_adjacence(graphe_connu, nb_cellules);
    if (meilleur_voisin == -1) {
        return depart;
    }
    int diff = meilleur_voisin - depart;
    int direction_flag = 0;
    if (diff == -colonnes) direction_flag = 1;
    else if (diff == 1) direction_flag = 2;
    else if (diff == colonnes) direction_flag = 4;
    else if (diff == -1) direction_flag = 8;
    if (murs_reels[depart] & direction_flag) {
        apprendre_mur(monstre, depart, meilleur_voisin, colonnes);
        return depart;
    } else {
        return meilleur_voisin;
    }
}

int gidi(Monstre * monstre, int * murs_reels, int lignes, int colonnes,const int * penalite_map) {
    int nb_cellules = lignes * colonnes;
    monstre -> noeuds_visites_zone[monstre -> pos] = true;
    for (int k = 0; k < monstre -> frontier_size; k++) {
        if (monstre -> frontier_nodes[k] == monstre -> pos) {
            monstre -> frontier_nodes[k] = monstre -> frontier_nodes[--monstre -> frontier_size];
            break;
        }
    }
    int x_act, y_act;
    indice_vers_coord(monstre -> pos, colonnes, & x_act, & y_act);
    int voisins[4] = {
        (y_act > 0) ? (y_act - 1) * colonnes + x_act : -1,
        (x_act < colonnes - 1) ? y_act * colonnes + (x_act + 1) : -1,
        (y_act < lignes - 1) ? (y_act + 1) * colonnes + x_act : -1,
        (x_act > 0) ? y_act * colonnes + (x_act - 1) : -1
    };
    melanger_voisins(voisins, 4);
    for (int i = 0; i < 4; i++) {
        int v = voisins[i];
        if (v == -1) continue;
        int diff = v - monstre -> pos;
        int dir_flag = 0;
        if (diff == -colonnes) dir_flag = 1;
        else if (diff == 1) dir_flag = 2;
        else if (diff == colonnes) dir_flag = 4;
        else if (diff == -1) dir_flag = 8;
        if (murs_reels[monstre -> pos] & dir_flag) {
            apprendre_mur(monstre, monstre -> pos, v, colonnes);
        } else {
            if (!monstre -> noeuds_visites_zone[v]) {
                bool deja_dans_frontiere = false;
                for (int j = 0; j < monstre -> frontier_size; j++) {
                    if (monstre -> frontier_nodes[j] == v) {
                        deja_dans_frontiere = true;
                        break;
                    }
                }
                if (!deja_dans_frontiere) {
                    if (monstre -> frontier_size < nb_cellules) {
                        monstre -> frontier_nodes[monstre -> frontier_size++] = v;
                    } else {
                        //la frontière est pleine, ce qui est anormal
                        printf("Avertissement: La frontière est pleine, impossible d'ajouter le noeud %d\n", v);
                    }
                }
            }
        }
    }
    if (monstre -> frontier_size == 0) return monstre -> pos;
    noeud plan_vers_frontiere;
    int ** graphe_connu_1 = creer_matrice_couts_dynamiques(monstre -> murs_connus, penalite_map, lignes, colonnes);
    if (!graphe_connu_1) {
        return monstre -> pos;
    }
    Dijkstra_laby(graphe_connu_1, nb_cellules, monstre -> pos, & plan_vers_frontiere);
    liberer_matrice_adjacence(graphe_connu_1, nb_cellules);
    int target_node = -1;
    int min_dist = INF;
    for (int i = 0; i < monstre -> frontier_size; i++) {
        int node_f = monstre -> frontier_nodes[i];
        if (plan_vers_frontiere.distance[node_f] < min_dist) {
            min_dist = plan_vers_frontiere.distance[node_f];
            target_node = node_f;
        }
    }
    free_noeuds( & plan_vers_frontiere);
    if (target_node == -1) return monstre -> pos;
    noeud plan_vers_cible;
    int ** graphe_connu_2 = creer_matrice_couts_dynamiques(monstre -> murs_connus, penalite_map, lignes, colonnes);
    if (!graphe_connu_2) {
        return monstre -> pos;
    }
    Dijkstra_laby(graphe_connu_2, nb_cellules, target_node, & plan_vers_cible);
    liberer_matrice_adjacence(graphe_connu_2, nb_cellules);
    int prochain_pas_planifie = -1;
    if (plan_vers_cible.distance[monstre -> pos] != INF) {
        prochain_pas_planifie = plan_vers_cible.parent[monstre -> pos];
    }
    free_noeuds( & plan_vers_cible);
    if (prochain_pas_planifie == -1 || prochain_pas_planifie == monstre -> pos) {
        return monstre -> pos;
    }
    int diff = prochain_pas_planifie - monstre -> pos;
    int direction_flag = 0;
    if (diff == -colonnes) direction_flag = 1;
    else if (diff == 1) direction_flag = 2;
    else if (diff == colonnes) direction_flag = 4;
    else if (diff == -1) direction_flag = 8;
    if (murs_reels[monstre -> pos] & direction_flag) {
        apprendre_mur(monstre, monstre -> pos, prochain_pas_planifie, colonnes);
        return monstre -> pos;
    } else {
        return prochain_pas_planifie;
    }
}

void dessiner_monstre_anime(SDL_Renderer* rendu, Monstre* monstre, SDL_Texture* textures[]) {
    // Choisir la bonne texture dans le tableau en fonction de la direction
    SDL_Texture* current_texture = textures[monstre->anim_dir];

    if (current_texture == NULL) return;
    
    int texture_w, texture_h;
    SDL_QueryTexture(current_texture, NULL, NULL, &texture_w, &texture_h);
    int frame_w = texture_w / MONSTRE_ANIM_FRAMES;
    int frame_h = texture_h;


    SDL_Rect src = {
        .x = monstre->anim_frame * frame_w,
        .y = 0,
        .w = frame_w,
        .h = frame_h
    };

    // Zoom
    int zoomed_w = g_config.cell_size * MONSTRE_ZOOM_FACTOR;
    int zoomed_h = g_config.cell_size * MONSTRE_ZOOM_FACTOR;

    SDL_Rect dst = {
        .x = (int)(monstre->pixel_x - zoomed_w / 2), // Centre le sprite zoomé horizontalement
        .y = (int)(monstre->pixel_y - zoomed_h / 2), // Centre le sprite zoomé verticalement
        .w = zoomed_w,
        .h = zoomed_h
    };
    
    // Retourner l'image si le monstre va à gauche
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (monstre->anim_dir == LEFT) {
        flip = SDL_FLIP_HORIZONTAL;
    }
    
    SDL_RenderCopyEx(rendu, current_texture, &src, &dst, 0, NULL, flip);
}

void mettre_a_jour_monstre(Monstre * monstres, int monstre_index, int joueur_pos, int * murs_reels, int lignes, int colonnes) {
    Monstre * monstre = &monstres[monstre_index];
    
    // Si le monstre est déjà en train de se déplacer, on ne prend pas de nouvelle décision
    if (monstre->is_moving) {
        return;
    }


    int nb_cellules = lignes * colonnes;
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
    } else if (monstre->timer_piste > 0) {
        monstre->mode = AI_MODE_HUNT;
        if (old_mode != AI_MODE_HUNT) printf("Monstre %d -> MODE CHASSE (Poursuite de la piste)\n", monstre->pos);
    } else {
        monstre->mode = AI_MODE_SEARCH_ZONE;
        if (old_mode != AI_MODE_SEARCH_ZONE) {
            printf("Monstre %d -> MODE RECHERCHE (Piste perdue)\n", monstre->pos);
            monstre->drnier_pos_jr_connu = monstre->pos;
            monstre->frontier_size = 0;
            memset(monstre->noeuds_visites_zone, 0, sizeof(bool) * (lignes * colonnes));
            monstre->frontier_nodes[monstre->frontier_size++] = monstre->pos;
        }
    }
    if (monstre->move_cooldown > 0) {
        monstre->move_cooldown--;
        return;
    }
    monstre->move_cooldown = VITESSE_MONSTRE;
    int * penalite_map = calloc(nb_cellules, sizeof(int));
    if (!penalite_map) return;
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        if (i == monstre_index) continue;
        int autre_monstre_pos = monstres[i].pos;
        int am_x, am_y;
        indice_vers_coord(autre_monstre_pos, colonnes, &am_x, &am_y);
        for (int y_p = am_y - MONSTRE_PENALITE_RAYON; y_p <= am_y + MONSTRE_PENALITE_RAYON; y_p++) {
            for (int x_p = am_x - MONSTRE_PENALITE_RAYON; x_p <= am_x + MONSTRE_PENALITE_RAYON; x_p++) {
                if (x_p >= 0 && x_p < colonnes && y_p >= 0 && y_p < lignes) {
                    int dist = abs(x_p - am_x) + abs(y_p - am_y);
                    if (dist <= MONSTRE_PENALITE_RAYON) {
                        int penalite = MONSTRE_PENALITE_COUT * (MONSTRE_PENALITE_RAYON - dist) / MONSTRE_PENALITE_RAYON;
                        int cell_idx = y_p * colonnes + x_p;
                        penalite_map[cell_idx] += penalite;
                    }
                }
            }
        }
    }
    int prochaine_position_planifiee = monstre->pos;
    switch (monstre->mode) {
    case AI_MODE_HUNT:
        prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, joueur_pos, penalite_map);
        monstre->rapp_cooldown = 0;
        break;
    case AI_MODE_SEARCH_ZONE:
        if (monstre->pos != monstre->drnier_pos_jr_connu && monstre->rapp_cooldown == 0) {
            prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, monstre->drnier_pos_jr_connu, penalite_map);
            if (prochaine_position_planifiee == monstre->drnier_pos_jr_connu) {
                monstre->rapp_cooldown = RAPP_CLDWN;
            }
        } else {
            prochaine_position_planifiee = gidi(monstre, murs_reels, lignes, colonnes, penalite_map);
            if (monstre->rapp_cooldown >= 1) {
                monstre->rapp_cooldown--;
            }
            if (monstre->rapp_cooldown == 1) {
                monstre->drnier_pos_jr_connu = joueur_pos;
            }
        }
        break;
    }
    free(penalite_map);




    int ancienne_pos = monstre->pos;

    // Si l'IA a décidé de bouger, on lance l'animation
    if (prochaine_position_planifiee != ancienne_pos) {
        monstre->is_moving = true;
        
        
        monstre->pos = prochaine_position_planifiee;
        
        // Déterminer la direction de l'animation en comparant la nouvelle et l'ancienne position
        int diff = monstre->pos - ancienne_pos;
        if (diff == -colonnes) monstre->anim_dir = UP;
        else if (diff == colonnes) monstre->anim_dir = DOWN;
        else if (diff == -1) monstre->anim_dir = LEFT;
        else if (diff == 1) monstre->anim_dir = RIGHT;
        
        
        // Définir la cible en pixels
        monstre->target_x = g_config.offset_x + (monstre->pos % colonnes + 0.5f) * g_config.cell_size;
        monstre->target_y = g_config.offset_y + (monstre->pos / colonnes + 0.5f) * g_config.cell_size;
    }
}

void dessiner_joueur_anime(SDL_Renderer * rendu, Joueur * joueur) {
    int row;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    // Choisir la ligne du spritesheet en fonction de la direction de l'animation et le mouvement
    if (!joueur -> is_moving) {
        row = 1; // Ligne d'animation "idle"
    } else {
        switch (joueur -> anim_dir) {
        case DOWN:
            row = 2;
            break;
        case RIGHT:
            row = 3;
            break;
        case LEFT:
            row = 3;
            flip = SDL_FLIP_HORIZONTAL;
            break;
        case UP:
            row = 4;
            break;
        default:
            row = 1;
            break;
        }
    }


    SDL_Rect src = {
        .x = joueur -> anim_frame * ASTRO_FRAME_WIDTH,
        .y = row * ASTRO_FRAME_HEIGHT,
        .w = ASTRO_FRAME_WIDTH,
        .h = ASTRO_FRAME_HEIGHT
    };


    SDL_Rect dst = {
        .x = (int)(joueur -> pixel_x - g_config.cell_size / 2),
        .y = (int)(joueur -> pixel_y - g_config.cell_size / 2),
        .w = g_config.cell_size,
        .h = g_config.cell_size
    };

    SDL_RenderCopyEx(rendu, joueur -> texture, & src, & dst, 0, NULL, flip);
}






// Fonction de jeu principale
GameResult lancer_jeu(SDL_Renderer* rendu, int* murs_reels, int lignes, int colonnes, AudioData* audio){
    int nb_cellules = lignes * colonnes;


    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        exit(-1);
    }
    g_config.window_w = dm.w;
    g_config.window_h = dm.h;
    int cell_w = g_config.window_w / colonnes;
    int cell_h = g_config.window_h / lignes;
    g_config.cell_size = (cell_w < cell_h) ? cell_w : cell_h;
    g_config.wall_thickness = (int)(g_config.cell_size / 16.0f);
    if (g_config.wall_thickness < 1) g_config.wall_thickness = 1;
    g_config.offset_x = (g_config.window_w - (colonnes * g_config.cell_size)) / 2;
    g_config.offset_y = (g_config.window_h - (lignes * g_config.cell_size)) / 2;


    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "img/CosmicLilac_AnimatedSpriteSheet.png");
    SDL_Texture* piece_texture = IMG_LoadTexture(rendu, "img/o2.png");
    SDL_Texture* monstre_textures[5];
    monstre_textures[IDLE] = NULL;
    monstre_textures[DOWN] = IMG_LoadTexture(rendu, "img/Carry_Run_Down-Sheet.png");
    monstre_textures[RIGHT] = IMG_LoadTexture(rendu, "img/Carry_Run_Side-Sheet.png");
    monstre_textures[UP] = IMG_LoadTexture(rendu, "img/Carry_Run_Up-Sheet.png");
    monstre_textures[LEFT] = monstre_textures[RIGHT];
    if (!perso_texture || !piece_texture || !monstre_textures[DOWN] || !monstre_textures[RIGHT] ||
        !monstre_textures[UP]) {
        printf("Erreur lors du chargement d'une texture: %s\n", IMG_GetError());
        exit(-1);
    }


    Joueur joueur;
    joueur.pos = 0;
    joueur.direction = 4;
    joueur.saut_cooldown = 0;
    joueur.texture = perso_texture;
    joueur.is_moving = false;
    joueur.anim_dir = DOWN;
    joueur.anim_frame = 0;
    joueur.last_frame_time = 0;
    joueur.pixel_x = g_config.offset_x + (joueur.pos % colonnes + 0.5f) * g_config.cell_size;
    joueur.pixel_y = g_config.offset_y + (joueur.pos / colonnes + 0.5f) * g_config.cell_size;
    joueur.target_x = joueur.pixel_x;
    joueur.target_y = joueur.pixel_y;
    SDL_Color couleur_saut_pret = {255, 255, 255, 255}; 
    // Le rayon est proportionnel à la taille d'une cellule
    int indicateur_radius = g_config.cell_size / 8; 
    joueur.saut_indicateur_texture = creer_texture_cercle(rendu, indicateur_radius, couleur_saut_pret);

    Monstre monstres[NOMBRE_MONSTRES];
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        Monstre* m = &monstres[i];
        m->pos = nb_cellules - 1 - i * 2 * (rand()%colonnes);
        m->mode = AI_MODE_SEARCH_ZONE;
        m->timer_piste = 0;
        m->murs_connus = calloc(nb_cellules, sizeof(int));
        m->memoire_murs = malloc(sizeof(arete) * MEMOIRE_MAX);
        m->memoire_tete = 0;
        m->memoire_queue = 0;
        m->memoire_taille_actuelle = 0;
        m->noeuds_visites_zone = calloc(nb_cellules, sizeof(bool));
        m->frontier_nodes = malloc(sizeof(int) * nb_cellules);
        m->frontier_size = 0;
        m->drnier_pos_jr_connu = 0;
        m->rapp_cooldown = 0;
        m->move_cooldown = 1;
        m->is_moving = false;
        m->anim_dir = DOWN;
        m->anim_frame = 0;
        m->last_frame_time = 0;
        m->pixel_x = g_config.offset_x + (m->pos % colonnes + 0.5f) * g_config.cell_size;
        m->pixel_y = g_config.offset_y + (m->pos / colonnes + 0.5f) * g_config.cell_size;
        m->target_x = m->pixel_x;
        m->target_y = m->pixel_y;
    }

    // Initialisation des Pièces 
    int* pieces_pos = malloc(sizeof(int) * NOMBRE_PIECES);
    int pieces_collectees = 0;
    for (int i = 0; i < NOMBRE_PIECES; i++) {
        int pos;
        bool position_valide;
        do {
            position_valide = true;
            pos = rand() % nb_cellules;
            if (pos == joueur.pos) {
                position_valide = false;
                continue;
            }
            for (int j = 0; j < NOMBRE_MONSTRES; j++) {
                if (pos == monstres[j].pos) {
                    position_valide = false;
                    break;
                }
            }
            if (!position_valide) continue;
            for (int k = 0; k < i; k++) {
                if (pos == pieces_pos[k]) {
                    position_valide = false;
                    break;
                }
            }
        } while (!position_valide);
        pieces_pos[i] = pos;
    }

    // --- BOUCLE DE JEU PRINCIPALE ---
    bool quitter = false;
    SDL_Event e;
    while (!quitter) {
        if (joueur.saut_cooldown > 0) {
            joueur.saut_cooldown--;
        }

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quitter = true;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quitter = true;
                        break;

                    case SDLK_SPACE:
                        if (!joueur.is_moving && joueur.saut_cooldown == 0) {
                            int pos_apres_saut = -1;
                            if (murs_reels[joueur.pos] & joueur.direction) {
                                if (joueur.direction == 1)
                                    pos_apres_saut = joueur.pos - colonnes;
                                else if (joueur.direction == 4)
                                    pos_apres_saut = joueur.pos + colonnes;
                                else if (joueur.direction == 8)
                                    pos_apres_saut = joueur.pos - 1;
                                else if (joueur.direction == 2)
                                    pos_apres_saut = joueur.pos + 1;

                                int x, y;
                                indice_vers_coord(pos_apres_saut, colonnes, &x, &y);
                                if (x >= 0 && x < colonnes && y >= 0 && y < lignes) {
                                    if (((joueur.direction == 2 || joueur.direction == 8) &&
                                         joueur.pos / colonnes == y) ||
                                        joueur.direction == 1 || joueur.direction == 4) {
                                        joueur.pos = pos_apres_saut;
                                        joueur.saut_cooldown = SAUT_COOLDOWN;
                                        joueur.pixel_x =
                                            g_config.offset_x +
                                            (joueur.pos % colonnes + 0.5f) * g_config.cell_size;
                                        joueur.pixel_y =
                                            g_config.offset_y +
                                            (joueur.pos / colonnes + 0.5f) * g_config.cell_size;
                                        joueur.target_x = joueur.pixel_x;
                                        joueur.target_y = joueur.pixel_y;
                                    }
                                }
                            }
                        }
                        break;
                }
            }
        }


        // Gestion du mouvement continu du joueur
        if (!joueur.is_moving) {
            const Uint8* keystate = SDL_GetKeyboardState(NULL);
            int nouvelle_pos = joueur.pos;
            int direction_flag = 0;
            AnimDirection future_anim_dir = joueur.anim_dir;
            if (keystate[SDL_SCANCODE_UP]) {
                nouvelle_pos -= colonnes;
                direction_flag = 1;
                future_anim_dir = UP;
            } else if (keystate[SDL_SCANCODE_DOWN]) {
                nouvelle_pos += colonnes;
                direction_flag = 4;
                future_anim_dir = DOWN;
            } else if (keystate[SDL_SCANCODE_LEFT]) {
                nouvelle_pos -= 1;
                direction_flag = 8;
                future_anim_dir = LEFT;
            } else if (keystate[SDL_SCANCODE_RIGHT]) {
                nouvelle_pos += 1;
                direction_flag = 2;
                future_anim_dir = RIGHT;
            }
            if (direction_flag != 0) {
                joueur.direction = direction_flag;
                if (!(murs_reels[joueur.pos] & direction_flag)) {
                    joueur.pos = nouvelle_pos;
                    joueur.is_moving = true;
                    joueur.anim_dir = future_anim_dir;
                    joueur.target_x =
                        g_config.offset_x + (joueur.pos % colonnes + 0.5f) * g_config.cell_size;
                    joueur.target_y =
                        g_config.offset_y + (joueur.pos / colonnes + 0.5f) * g_config.cell_size;
                }
            }
        }

        // Mise à jour de la position du joueur
        if (joueur.is_moving) {
            float dx = joueur.target_x - joueur.pixel_x;
            float dy = joueur.target_y - joueur.pixel_y;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance < VITESSE_DEPLACEMENT) {
                joueur.pixel_x = joueur.target_x;
                joueur.pixel_y = joueur.target_y;
                joueur.is_moving = false;
            } else {
                joueur.pixel_x += (dx / distance) * VITESSE_DEPLACEMENT;
                joueur.pixel_y += (dy / distance) * VITESSE_DEPLACEMENT;
            }
            Uint32 now = SDL_GetTicks();
            if (now - joueur.last_frame_time > 120) {
                joueur.anim_frame = (joueur.anim_frame + 1) % ASTRO_FRAME_COUNT;
                joueur.last_frame_time = now;
            }
        } else {
            joueur.anim_frame = 0;
        }

        // Mise à jour de la position des monstres
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            Monstre* m = &monstres[i];
            if (m->is_moving) {
                float dx = m->target_x - m->pixel_x;
                float dy = m->target_y - m->pixel_y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance < VITESSE_MONSTRE_PIXELS) {
                    m->pixel_x = m->target_x;
                    m->pixel_y = m->target_y;
                    m->is_moving = false;
                } else {
                    m->pixel_x += (dx / distance) * VITESSE_MONSTRE_PIXELS;
                    m->pixel_y += (dy / distance) * VITESSE_MONSTRE_PIXELS;
                }
                Uint32 now = SDL_GetTicks();
                if (now - m->last_frame_time > 1000 / 12) {
                    m->anim_frame = (m->anim_frame + 1) % MONSTRE_ANIM_FRAMES;
                    m->last_frame_time = now;
                }
            } else {
                m->anim_frame = 0;
            }
        }

        // (collecte, victoire, défaite)
        for (int i = 0; i < NOMBRE_PIECES; i++) {
            if (pieces_pos[i] != -1 && joueur.pos == pieces_pos[i]) {
                pieces_pos[i] = -1;
                pieces_collectees++;
                printf("Pièce collectée ! (%d / %d)\n", pieces_collectees, NOMBRE_PIECES);

                //Jouer le son de la pièce !
                play_o2_sound(audio);
            }
        }
        if (pieces_collectees == NOMBRE_PIECES) {
            printf("VICTOIRE !\n");

            Mix_HaltMusic(); // Arrête la musique de fond
            play_victory_sound(audio);
            
            free(pieces_pos);
            for (int i = 0; i < NOMBRE_MONSTRES; i++) {
                free(monstres[i].murs_connus);
                free(monstres[i].memoire_murs);
                free(monstres[i].noeuds_visites_zone);
                free(monstres[i].frontier_nodes);
            }
            if (joueur.saut_indicateur_texture) {
                SDL_DestroyTexture(joueur.saut_indicateur_texture);
            }
            SDL_DestroyTexture(perso_texture);
            SDL_DestroyTexture(piece_texture);
            SDL_DestroyTexture(monstre_textures[DOWN]);
            SDL_DestroyTexture(monstre_textures[RIGHT]);
            SDL_DestroyTexture(monstre_textures[UP]);
            
            return GAME_WON; // On retourne l'état de victoire
        }
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            mettre_a_jour_monstre(monstres, i, joueur.pos, murs_reels, lignes, colonnes);
            if (monstres[i].pos == joueur.pos) {
                printf("GAME OVER !\n");
                
                Mix_HaltMusic(); // Arrête la musique de fond
                play_failure_sound(audio);

                free(pieces_pos);
                for (int i = 0; i < NOMBRE_MONSTRES; i++) {
                    free(monstres[i].murs_connus);
                    free(monstres[i].memoire_murs);
                    free(monstres[i].noeuds_visites_zone);
                    free(monstres[i].frontier_nodes);
                }
                if (joueur.saut_indicateur_texture) {
                SDL_DestroyTexture(joueur.saut_indicateur_texture);
                }
                SDL_DestroyTexture(perso_texture);
                SDL_DestroyTexture(piece_texture);
                SDL_DestroyTexture(monstre_textures[DOWN]);
                SDL_DestroyTexture(monstre_textures[RIGHT]);
                SDL_DestroyTexture(monstre_textures[UP]);
                return GAME_LOST; 
            }
        }

        // Dessin
        SDL_SetRenderDrawColor(rendu, 20, 0, 30, 255);
        SDL_RenderClear(rendu);
        dessiner_bg(rendu, lignes, colonnes);
        for (int i = 0; i < NOMBRE_PIECES; i++) {
            if (pieces_pos[i] != -1) {
                float piece_px =
                    g_config.offset_x + (pieces_pos[i] % colonnes + 0.5f) * g_config.cell_size;
                float piece_py =
                    g_config.offset_y + (pieces_pos[i] / colonnes + 0.5f) * g_config.cell_size;
                dessiner_personnage(rendu, piece_texture, piece_px, piece_py);
            }
        }
        SDL_SetRenderDrawColor(rendu, 100, 80, 200, 255);
        for (int i = 0; i < nb_cellules; i++)
            dessiner_murs_connus(rendu, i % colonnes, i / colonnes, murs_reels, colonnes);

        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            if (monstres[i].mode == AI_MODE_HUNT) {
                dessiner_rayon_detection(rendu, monstres[i].pos, SEUIL_DETECTION_HUNT, lignes, colonnes);
            }
        }

        //SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_BLEND);

        if (joueur.saut_indicateur_texture) {
            // Si le saut est disponible
            if (joueur.saut_cooldown == 0) {
                // On met l'indicateur en vert et visible
                SDL_SetTextureColorMod(joueur.saut_indicateur_texture, 0, 255, 100);
                SDL_SetTextureAlphaMod(joueur.saut_indicateur_texture, 200); 
            } else { // Si le saut est en cooldown
                // On met l'indicateur en rouge et moins visible
                SDL_SetTextureColorMod(joueur.saut_indicateur_texture, 255, 50, 50);
                // On fait varier l'alpha pour un effet "recharge"
                Uint8 alpha = 50 + (Uint8)(150.0f * (SAUT_COOLDOWN - joueur.saut_cooldown) / SAUT_COOLDOWN);
                SDL_SetTextureAlphaMod(joueur.saut_indicateur_texture, alpha);
            }

            // On dessine la texture de l'indicateur sous les pieds du joueur
            int w, h;
            SDL_QueryTexture(joueur.saut_indicateur_texture, NULL, NULL, &w, &h);

            int indicateur_y_offset = g_config.cell_size*2/3;

            SDL_Rect indicateur_dst = {
                (int)(joueur.pixel_x - w / 2),
                (int)(joueur.pixel_y - h / 2 + indicateur_y_offset),
                w,
                h
            };
            SDL_RenderCopy(rendu, joueur.saut_indicateur_texture, NULL, &indicateur_dst);
        }

        dessiner_joueur_anime(rendu, &joueur);
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            Monstre* m = &monstres[i];
            if (m->mode == AI_MODE_HUNT) {
                SDL_SetTextureColorMod(monstre_textures[m->anim_dir], 255, 100, 100);
            }
            dessiner_monstre_anime(rendu, m, monstre_textures);
            if (m->mode == AI_MODE_HUNT) {
                SDL_SetTextureColorMod(monstre_textures[m->anim_dir], 255, 255, 255);
            }
        }
        SDL_RenderPresent(rendu);
        SDL_Delay(16);
    }

    free(pieces_pos);
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        free(monstres[i].murs_connus);
        free(monstres[i].memoire_murs);
        free(monstres[i].noeuds_visites_zone);
        free(monstres[i].frontier_nodes);
    }
    if (joueur.saut_indicateur_texture) {
        SDL_DestroyTexture(joueur.saut_indicateur_texture);
    }
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyTexture(piece_texture);
    SDL_DestroyTexture(monstre_textures[DOWN]);
    SDL_DestroyTexture(monstre_textures[RIGHT]);
    SDL_DestroyTexture(monstre_textures[UP]);

    return GAME_QUIT_MANUALLY;
}













