#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "laby.h"
#include "labySDL.h"



#define AI_MODE_SEARCH 0
#define AI_MODE_HUNT   1


#define NOMBRE_MONSTRES 3
#define SEUIL_DETECTION_VUE 10 // Distance (Manhattan) à laquelle les monstres "voient" le joueur
#define MEMOIRE_MAX 50         // Le monstre peut se souvenir des 50 derniers murs découverts
#define PLANNING_COOLDOWN 15   // Re-planifie toutes les 15 frames, non il faut qu il planiffie quand il fini
#define VITESSE_MONSTRE 1      // Bouge toutes les . frames

typedef struct {
    int pos;
    int mode;
    int joueur_derniere_pos_connue;
    int destination_actuelle;

    int* murs_connus;             // Sa propre carte mentale

    arete* memoire_murs;          // La file (FIFO) de sa mémoire
    int memoire_tete;
    int memoire_queue;
    int memoire_taille_actuelle;
    
    int plan_cooldown;
    int move_cooldown;
} Monstre;


void apprendre_mur(Monstre* monstre, int u, int v, int colonnes) {
    // 1. Apprendre le nouveau mur en l'ajoutant à la carte mentale
    ajouter_mur(monstre->murs_connus, colonnes, u, v);

    // 2. Vérifier si la mémoire est pleine
    if (monstre->memoire_taille_actuelle >= MEMOIRE_MAX) {
        // 2a. Oublier le mur le plus ancien
        arete mur_oublie = monstre->memoire_murs[monstre->memoire_tete];
        supprimer_mur(monstre->murs_connus, colonnes, mur_oublie.u, mur_oublie.v);
        printf("Monstre à la pos %d oublie le mur entre %d et %d.\n", monstre->pos, mur_oublie.u, mur_oublie.v);
        // Avancer la tête de la file
        monstre->memoire_tete = (monstre->memoire_tete + 1) % MEMOIRE_MAX;
        monstre->memoire_taille_actuelle--;
    }

    // 3. Ajouter le nouveau mur à la fin de la file mémoire
    monstre->memoire_murs[monstre->memoire_queue] = (arete){u, v};
    monstre->memoire_queue = (monstre->memoire_queue + 1) % MEMOIRE_MAX;
    monstre->memoire_taille_actuelle++;
}


void lancer_jeu_complexe(int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;

    // --- Initialisation SDL ---
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* fenetre = SDL_CreateWindow("Jeu de Poursuite IA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");
    SDL_Texture* monstre_texture = IMG_LoadTexture(rendu, "monstre.png");

    // --- Initialisation des Entités ---
    int joueur_pos = 0;
    Monstre monstres[NOMBRE_MONSTRES];

    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        Monstre* m = &monstres[i];
        m->pos = nb_cellules - 1 - i;
        m->mode = AI_MODE_SEARCH;
        m->joueur_derniere_pos_connue = -1; // Ne sait pas où est le joueur
        m->destination_actuelle = rand() % nb_cellules; // Patrouille au hasard au début

        m->murs_connus = calloc(nb_cellules, sizeof(int));
        m->memoire_murs = malloc(sizeof(arete) * MEMOIRE_MAX);
        m->memoire_tete = 0;
        m->memoire_queue = 0;
        m->memoire_taille_actuelle = 0;
        
        m->plan_cooldown = i * 5; // Décale leur planification
        m->move_cooldown = 0;
    }

    // --- Boucle de Jeu Principale ---
    bool quitter = false;
    SDL_Event e;

    while (!quitter) {
        // --- 1. GESTION DES ÉVÉNEMENTS (INPUTS) ---
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

        // --- 2. MISE À JOUR DE LA LOGIQUE (UPDATE) ---
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            Monstre *monstre = &monstres[i];

            // --- "BRAIN TICK" DE L'IA ---

            // A. PERCEPTION: Le monstre "voit"-il le joueur ?
            int j_x, j_y, m_x, m_y;
            indice_vers_coord(joueur_pos, colonnes, &j_x, &j_y);
            indice_vers_coord(monstre->pos, colonnes, &m_x, &m_y);
            int distance = abs(j_x - m_x) + abs(j_y - m_y);

            // B. DÉCISION: Changer de mode si nécessaire
            if (distance < SEUIL_DETECTION_VUE) {
                if (monstre->mode == AI_MODE_SEARCH) {
                    printf("Monstre %d VOIT LE JOUEUR! Passage en mode CHASSE.\n", i);
                }
                monstre->mode = AI_MODE_HUNT;
                monstre->joueur_derniere_pos_connue = joueur_pos; // Met à jour la dernière position connue
                monstre->destination_actuelle = joueur_pos;
                monstre->plan_cooldown = 0; // Force la re-planification immédiate
            } else {
                if (monstre->mode == AI_MODE_HUNT) {
                    printf("Monstre %d A PERDU LE JOUEUR. Passage en mode RECHERCHE (vers la dernière position connue).\n", i);
                    monstre->destination_actuelle = monstre->joueur_derniere_pos_connue;
                }
                monstre->mode = AI_MODE_SEARCH;
            }

            // C. PLANIFICATION: Si le cooldown est terminé, calculer le prochain pas
            int prochain_pas = -1;
            if (monstre->plan_cooldown <= 0) {
                 monstre->plan_cooldown = PLANNING_COOLDOWN;

                // Si en mode recherche et qu'on a atteint la dernière pos connue, choisir une nouvelle cible de patrouille
                if(monstre->mode == AI_MODE_SEARCH && monstre->pos == monstre->destination_actuelle) {
                    monstre->destination_actuelle = rand() % nb_cellules; // zone?
                    printf("Monstre %d a fini sa recherche, nouvelle patrouille vers %d.\n", i, monstre->destination_actuelle);
                }

                // Utilise sa connaissance pour planifier
                noeud plan;
                //dijkstra --v
                BFS_laby(monstre->murs_connus, lignes, colonnes, monstre->destination_actuelle, &plan);

                // Si le plan est impossible, l'agent est bloqué.
                if(plan.distance[monstre->pos] == INF) {
                    printf("Monstre %d est bloqué, ne peut atteindre sa cible.\n", i);
                    // Choisit une nouvelle cible aléatoire pour se débloquer
                    monstre->destination_actuelle = rand() % nb_cellules;
                } else {
                    prochain_pas = plan.parent[monstre->pos];
                }
                free_noeuds(&plan);
            }
             
            monstre->plan_cooldown--;


            // D. ACTION: Se déplacer et apprendre
            if (monstre->move_cooldown <= 0) {
                if(prochain_pas != -1) {
                    // Vérifier la réalité
                    int diff = prochain_pas - monstre->pos;
                    int dir_flag = 0;
                    if (diff == -colonnes) dir_flag = 1; else if (diff == 1) dir_flag = 2; else if (diff == colonnes) dir_flag = 4; else if (diff == -1) dir_flag = 8;

                    if (murs_reels[monstre->pos] & dir_flag) {
                        // MUR INATTENDU
                        printf("Monstre %d heurte un mur de %d à %d.\n", i, monstre->pos, prochain_pas);
                        apprendre_mur(monstre, monstre->pos, prochain_pas, colonnes);
                        monstre->plan_cooldown = 0; // Re-planifier immédiatement
                    } else {
                        // PAS DE MUR, on bouge
                        monstre->pos = prochain_pas; // il m a j à chaque pas?
                    }
                }
                monstre->move_cooldown = VITESSE_MONSTRE;
            } else {
                monstre->move_cooldown--;
            }

            // Condition de défaite
            if (monstre->pos == joueur_pos) {
                printf("GAME OVER ! Le monstre %d vous a attrapé.\n", i);
                quitter = true;
            }
        }

        // --- 3. DESSIN (RENDER) ---
        SDL_SetRenderDrawColor(rendu, 20, 0, 30, 255); // Fond sombre
        SDL_RenderClear(rendu);
        
        // Dessiner les murs
        SDL_SetRenderDrawColor(rendu, 100, 80, 200, 255);
        for (int i = 0; i < nb_cellules; i++) dessiner_murs(rendu, i % colonnes, i / colonnes, murs_reels, colonnes);
        
        // Dessiner le joueur
        dessiner_personnage(rendu, perso_texture, (joueur_pos % colonnes + 0.5f) * TAILLE_CELLULE, (joueur_pos / colonnes + 0.5f) * TAILLE_CELLULE);

        // Dessiner les monstres
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
             // Change la couleur du monstre selon son mode
            if(monstres[i].mode == AI_MODE_HUNT) SDL_SetTextureColorMod(monstre_texture, 255, 100, 100); // Rouge en mode chasse
            else SDL_SetTextureColorMod(monstre_texture, 100, 255, 100); // Vert en mode recherche
            
            dessiner_personnage(rendu, monstre_texture, (monstres[i].pos % colonnes + 0.5f) * TAILLE_CELLULE, (monstres[i].pos / colonnes + 0.5f) * TAILLE_CELLULE);
        
        }
    
        SDL_RenderPresent(rendu);
    }
    // --- Nettoyage ---
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        free(monstres[i].murs_connus);
        free(monstres[i].memoire_murs);
    }
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyTexture(monstre_texture);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}





int main(){
    srand(time(NULL));
    int lignes = 20;
    int colonnes = 35;
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

        
    lancer_jeu_complexe(murs_reels, lignes, colonnes);
    
    free(murs_reels);
    printf("Programme terminé.\n");
    return 0;
}