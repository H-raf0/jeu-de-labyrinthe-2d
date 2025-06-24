#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "laby.h"
#include "labySDL.h"

#define CURRENT_ALGO 1
// 0 BFS, 1 DIJSKTRA, 2 A*
#define VITESSE 0.2f

#define CURRENT_HEURISTIC HEURISTIC_MANHATTAN // HEURISTIC_MANHATTAN ou HEURISTIC_EUCLIDEAN ou HEURISTIC_TCHEBYCHEV

#define DELAI_PAS 50

#define DELAI_PAS_EXPLORATION 50

void lancer_animation_labyrinthe(int* murs, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;

    // --- Initialisation SDL ---
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* fenetre = SDL_CreateWindow("Labyrinthe Animé", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");
    if (!perso_texture) { fprintf(stderr, "Erreur chargement personnage.png: %s\n", IMG_GetError()); return; }

    // --- Initialisation de l'état du jeu ---
    int depart = 0;
    int destination = nb_cellules - 1;
    noeud n;
    int* chemin = malloc(sizeof(int) * nb_cellules);
    int nb_etapes = 0;
    int etape_actuelle = 0;
    
    float perso_x, perso_y;


    // --- Création de la matrice d'adjacence ---
    printf("Création de la matrice d'adjacence...\n");
    int** graphe = NULL;
    if (CURRENT_ALGO == 1) {
        graphe = creer_matrice_adjacence(murs, lignes, colonnes);
        if (!graphe) {
            fprintf(stderr, "Impossible de créer la matrice d'adjacence.\n");
            return;
        }
    }

    // Calculer le premier chemin
    if(CURRENT_ALGO == 0) BFS_laby(murs, lignes, colonnes, destination, &n);
    else if (CURRENT_ALGO == 1) Dijkstra_laby(graphe, nb_cellules, destination, &n);
    else if (CURRENT_ALGO == 2) A_etoile_laby(murs, lignes, colonnes, depart, destination, &n, CURRENT_HEURISTIC);
    else printf("option inconnu\n");

    if (CURRENT_ALGO == 2) { // A* cherche depuis le départ
        nb_etapes = reconstruire_chemin_inverse(&n, depart, destination, nb_cellules, chemin);
    } else { // BFS et Dijkstra cherchent depuis la destination
        nb_etapes = reconstruire_chemin(&n, depart, destination, chemin);
    }
    etape_actuelle = 0;
    perso_x = (depart % colonnes) * TAILLE_CELLULE + TAILLE_CELLULE / 2.0f;
    perso_y = (depart / colonnes) * TAILLE_CELLULE + TAILLE_CELLULE / 2.0f;
    
    // --- Boucle Principale de l'Application ---
    int quitter = 0;
    SDL_Event e;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quitter = 1;
        }

        // --- Logique de mise à jour ---
        if (etape_actuelle < nb_etapes - 1) {
            int cell_suivante = chemin[etape_actuelle + 1];
            float target_x = (cell_suivante % colonnes) * TAILLE_CELLULE + TAILLE_CELLULE / 2.0f;
            float target_y = (cell_suivante / colonnes) * TAILLE_CELLULE + TAILLE_CELLULE / 2.0f;

            float vitesse = VITESSE;
            perso_x += (target_x - perso_x) * vitesse;
            perso_y += (target_y - perso_y) * vitesse;

            if (fabs(target_x - perso_x) < 1.0f && fabs(target_y - perso_y) < 1.0f) {
                etape_actuelle++;
                perso_x = target_x;
                perso_y = target_y;
            }
        } else { // Destination atteinte
            depart = destination;
            do {
                destination = rand() % nb_cellules;
            } while (destination == depart);
            
            printf("Nouvel objectif ! De %d vers %d\n", depart, destination);

            free_noeuds(&n);

            // --- APPEL DE L'ALGORITHME CHOISI ---
            if(CURRENT_ALGO == 0) BFS_laby(murs, lignes, colonnes, destination, &n);
            else if (CURRENT_ALGO == 1) Dijkstra_laby(graphe, nb_cellules, destination, &n);
            else if (CURRENT_ALGO == 2) A_etoile_laby(murs, lignes, colonnes, depart, destination, &n, CURRENT_HEURISTIC);
            else printf("option inconnu\n");

            if (CURRENT_ALGO == 2) {
                nb_etapes = reconstruire_chemin_inverse(&n, depart, destination, nb_cellules, chemin);
            } else {
                nb_etapes = reconstruire_chemin(&n, depart, destination, chemin);
            }
            etape_actuelle = 0;
        }

        // --- Dessin de la scène ---
        SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
        SDL_RenderClear(rendu);
        
        dessiner_fond(rendu, &n, lignes, colonnes);
        dessiner_chemin(rendu, chemin, nb_etapes, colonnes);
        SDL_SetRenderDrawColor(rendu, 255, 255, 255, 255);
        for (int i = 0; i < nb_cellules; i++) dessiner_murs(rendu, i % colonnes, i / colonnes, murs, colonnes);
        dessiner_marqueurs(rendu, depart, destination, colonnes);
        dessiner_personnage(rendu, perso_texture, perso_x, perso_y);
        SDL_RenderPresent(rendu);
    }

    if (CURRENT_ALGO == 2) comparer_heuristiques_A_etoile(murs, lignes, colonnes, depart, destination);

    // --- Nettoyage ---
    printf("Libération de la matrice d'adjacence...\n");
    liberer_matrice_adjacence(graphe, nb_cellules);
    free_noeuds(&n);
    free(chemin);
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}

// G.I.D.C //dijkstra matrice adj pas correct
void demarrer_exploration_dynamique(int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    int depart = 0;
    int destination = nb_cellules - 1;

    int* murs_connus = calloc(nb_cellules, sizeof(int));
    if (!murs_connus) { return; }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* fenetre = SDL_CreateWindow("Exploration Dynamique (Dijkstra)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");

    int pos_actuelle = depart;
    bool quitter_programme = false;

    while (pos_actuelle != destination && !quitter_programme) {
        
        // --- Boucle de Réflexion: L'agent reste ici jusqu'à ce qu'il puisse bouger ---
        bool a_transite = false;
        while (!a_transite && !quitter_programme) {
            
            // --- CORRECTION: Le "cerveau" est créé ici, au début de chaque décision ---
            int** graphe_connu = creer_matrice_adjacence_connue(murs_connus, lignes, colonnes);
            if (!graphe_connu) {
                fprintf(stderr, "Erreur création graphe connu.\n");
                quitter_programme = true;
                continue;
            }

            noeud plan;
            // On planifie avec Dijkstra sur ce graphe frais
            Dijkstra_laby(graphe_connu, nb_cellules, destination, &plan);
            
            SDL_Event e;
            if (SDL_PollEvent(&e) && e.type == SDL_QUIT) {
                quitter_programme = true;
                // --- CORRECTION: Nettoyer avant de continuer ---
                free_noeuds(&plan);
                liberer_matrice_adjacence(graphe_connu, nb_cellules);
                continue;
            }

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
                quitter_programme = true;
                // --- CORRECTION: Nettoyer avant de continuer ---
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
                
                // DESSINER l'état actuel
                SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
                SDL_RenderClear(rendu);
                for (int i = 0; i < nb_cellules; i++) dessiner_murs_connus(rendu, i % colonnes, i / colonnes, murs_connus, colonnes);
                dessiner_marqueurs(rendu, depart, destination, colonnes);
                dessiner_personnage(rendu, perso_texture, (pos_actuelle % colonnes + 0.5f) * TAILLE_CELLULE, (pos_actuelle / colonnes + 0.5f) * TAILLE_CELLULE);
                SDL_RenderPresent(rendu);
                SDL_Delay(DELAI_PAS);
            }
            
            // --- CORRECTION: Le "cerveau" est détruit ici, à la fin de chaque décision ---
            free_noeuds(&plan);
            liberer_matrice_adjacence(graphe_connu, nb_cellules);

        } // Fin de la boucle de réflexion
    } // Fin de la boucle de déplacement
    

    if (!quitter_programme) {
        printf("\nDestination atteinte !\n");
        SDL_Delay(3000);
    }

    free(murs_connus);
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}

// G.I.D.I
void demarrer_exploration_inconnue(int* murs_reels, int lignes, int colonnes, int destination_reelle) {
    int nb_cellules = lignes * colonnes;
    int depart = 0;

    // --- Structures de données de l'agent ---
    int* murs_connus = calloc(nb_cellules, sizeof(int));
    int* noeuds_visites = calloc(nb_cellules, sizeof(int)); // F (Closed set)
    int* frontier_nodes = malloc(sizeof(int) * nb_cellules); // O (Open set)
    int frontier_size = 0;
    int* passages_counts = calloc(nb_cellules, sizeof(int)); // Pour la heatmap
    int max_passages = 0;

    // --- Initialisation SDL ---
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* fenetre = SDL_CreateWindow("Exploration (Destination Inconnue)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");

    // --- Initialisation de l'algorithme ---
    int pos_actuelle = depart;
    bool destination_trouvee = false;
    
    while (!destination_trouvee) {
        // Gérer la fermeture de la fenêtre
        SDL_Event e;
        if (SDL_PollEvent(&e) && e.type == SDL_QUIT) break;
        
        // --- ÉTAPE 1: L'AGENT EST SUR UNE CASE ET OBSERVE ---
        
        // On vérifie si on a trouvé la destination par chance
        if (pos_actuelle == destination_reelle) {
            destination_trouvee = true;
            printf("DESTINATION TROUVÉE en %d !\n", pos_actuelle);
            continue;
        }

        // Marquer la position actuelle comme visitée (ajout à F)
        noeuds_visites[pos_actuelle] = 1;

        // "Scanner" les murs autour et mettre à jour la connaissance (G)
        int x_act, y_act;
        indice_vers_coord(pos_actuelle, colonnes, &x_act, &y_act);
        int voisins[4] = {
            (y_act > 0) ? (y_act - 1) * colonnes + x_act : -1, (x_act < colonnes - 1) ? y_act * colonnes + (x_act + 1) : -1,
            (y_act < lignes - 1) ? (y_act + 1) * colonnes + x_act : -1, (x_act > 0) ? y_act * colonnes + (x_act - 1) : -1
        };
        for (int i = 0; i < 4; i++) {
            if (voisins[i] == -1) continue;
            int diff = voisins[i] - pos_actuelle;
            int dir_flag = 0;
            if (diff == -colonnes) dir_flag = 1; else if (diff == 1) dir_flag = 2; else if (diff == colonnes) dir_flag = 4; else if (diff == -1) dir_flag = 8;
            
            if (murs_reels[pos_actuelle] & dir_flag) { // Il y a un mur
                ajouter_mur(murs_connus, colonnes, pos_actuelle, voisins[i]);
            } else { // Il n'y a pas de mur, c'est un voisin accessible
                // Si le voisin n'a pas encore été visité (pas dans F), on l'ajoute à la frontière (O)
                if (!noeuds_visites[voisins[i]]) {
                    // Vérifier s'il est déjà dans la frontière pour éviter les doublons
                    bool deja_dans_frontiere = false;
                    for (int j = 0; j < frontier_size; j++) {
                        if (frontier_nodes[j] == voisins[i]) { deja_dans_frontiere = true; break; }
                    }
                    if (!deja_dans_frontiere) {
                        frontier_nodes[frontier_size++] = voisins[i];
                    }
                }
            }
        }

        // --- ÉTAPE 2: CHOISIR LA PROCHAINE DESTINATION (LE NOEUD LE PLUS PROCHE DE LA FRONTIÈRE) ---
        
        // Retirer la position actuelle de la frontière si elle s'y trouvait
        for (int i = 0; i < frontier_size; i++) {
            if (frontier_nodes[i] == pos_actuelle) {
                frontier_nodes[i] = frontier_nodes[--frontier_size]; // Remplacer par le dernier et réduire la taille
                break;
            }
        }

        if (frontier_size == 0) {
            printf("Exploration terminée. Plus aucun noeud sur la frontière.\n");
            break;
        }


        // Au lieu de BFS, on planifie avec Dijkstra sur la carte connue
        printf("Planification locale avec Dijkstra...\n");

        // 1. Créer le graphe pondéré à partir de la connaissance actuelle
        int** graphe_connu = creer_matrice_adjacence_connue(murs_connus, lignes, colonnes);
        if (!graphe_connu) {
            printf("Erreur: impossible de créer le graphe connu.\n");
            break; // Quitter si l'allocation échoue
        }

        // Plan local pour trouver le chemin le plus court vers les noeuds de la frontière
        noeud plan_local;
        Dijkstra_laby(graphe_connu, nb_cellules, pos_actuelle, &plan_local);

        liberer_matrice_adjacence(graphe_connu, nb_cellules);



        int target_node = -1;
        int min_dist_to_frontier = INF;

        for (int i = 0; i < frontier_size; i++) {
            int node_on_frontier = frontier_nodes[i];
            if (plan_local.distance[node_on_frontier] < min_dist_to_frontier) {
                min_dist_to_frontier = plan_local.distance[node_on_frontier];
                target_node = node_on_frontier;
            }
        }

        if (target_node == -1) {
            printf("IMPASSE. Aucun noeud de la frontière n'est accessible.\n");
            free_noeuds(&plan_local);
            break;
        }

        // --- ÉTAPE 3: SE DÉPLACER VERS LA DESTINATION CHOISIE ---
        
        int* chemin_vers_target = malloc(sizeof(int) * nb_cellules);
        // Le dijkstra est parti de pos_actuelle, il faut donc reconstruire le chemin en inverse
        int nb_etapes = reconstruire_chemin_inverse(&plan_local, pos_actuelle, target_node, nb_cellules, chemin_vers_target);
        
        free_noeuds(&plan_local); // Le plan local n'est plus nécessaire

        printf("Cible la plus proche sur la frontière: %d. S'y rend en %d pas.\n", target_node, nb_etapes);

        // Animer le déplacement
        for (int i = 1; i < nb_etapes; i++) { // On commence à 1 car l'étape 0 est la pos_actuelle
            pos_actuelle = chemin_vers_target[i];
            passages_counts[pos_actuelle]++;
            if (passages_counts[pos_actuelle] > max_passages) {
                max_passages = passages_counts[pos_actuelle];
            }

            // Dessin
            SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
            SDL_RenderClear(rendu);
            dessiner_heatmap_passage(rendu, passages_counts, lignes, colonnes, max_passages);
            for (int j = 0; j < nb_cellules; j++) dessiner_murs_connus(rendu, j % colonnes, j / colonnes, murs_connus, colonnes);
            SDL_SetRenderDrawColor(rendu, 0, 255, 0, 255);
            SDL_Rect dest_rect = {(destination_reelle % colonnes) * TAILLE_CELLULE, (destination_reelle / colonnes) * TAILLE_CELLULE, TAILLE_CELLULE, TAILLE_CELLULE};
            SDL_RenderFillRect(rendu, &dest_rect); // Marquer la destination cachée
            dessiner_personnage(rendu, perso_texture, (pos_actuelle % colonnes + 0.5f) * TAILLE_CELLULE, (pos_actuelle / colonnes + 0.5f) * TAILLE_CELLULE);
            SDL_RenderPresent(rendu);
            SDL_Delay(DELAI_PAS_EXPLORATION);
            
            if (pos_actuelle == destination_reelle) {
                destination_trouvee = true;
                break;
            }
        }
        free(chemin_vers_target);

    } // Fin de la boucle d'exploration principale

    printf("Fin de la phase d'exploration.\n");
    SDL_Delay(3000);

    // Nettoyage
    free(murs_connus);
    free(noeuds_visites);
    free(frontier_nodes);
    free(passages_counts);
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}



int main() {
    srand(time(NULL));
    int lignes = 15;
    int colonnes = 20;
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

    lancer_animation_labyrinthe(murs_reels, lignes, colonnes);
    //demarrer_exploration_dynamique(murs_reels, lignes, colonnes);
    
    int destination_secrete = nb_cellules-1;
    
    printf("La destination secrète est en %d\n", destination_secrete);

    demarrer_exploration_inconnue(murs_reels, lignes, colonnes, destination_secrete);
    
    
    
    
    
    free(murs_reels);
    printf("Programme terminé.\n");
    return 0;
}


/*
//affichage des 3 algos
int main() {
    srand(time(NULL));

    // --- Génération du Labyrinthe ---
    int lignes = 20;
    int colonnes = 30;
    int nb_cellules = lignes * colonnes;
    
    arete *toutes_aretes;
    int nb_total_aretes = generation_grille_vide(&toutes_aretes, lignes, colonnes);
    fisher_yates(toutes_aretes, nb_total_aretes);
    arete *arbre = malloc(sizeof(arete) * (nb_cellules - 1)); // MAX : (N-1)*M +N*(M-1)
    int nb_aretes_arbre;
    construire_arbre_couvrant(toutes_aretes, nb_total_aretes, arbre, &nb_aretes_arbre, nb_cellules);
    free(toutes_aretes);
    
    int *murs = malloc(sizeof(int) * nb_cellules);
    for (int i = 0; i < nb_cellules; i++) murs[i] = 1 | 2 | 4 | 8;
    for (int i = 0; i < nb_aretes_arbre; i++) supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);
    free(arbre);

    // --- Lancement de l'application ---
    lancer_animation_labyrinthe(murs, lignes, colonnes);

    // --- Libération finale des ressources ---
    free(murs);

    printf("Programme terminé.\n");
    return 0;
}
*/

/*
int main(int argc, char* argv[]) {
    srand(time(NULL));

    // Définir la taille du labyrinthe
    int lignes = 20;
    int colonnes = 30;
    int nb_cellules = lignes * colonnes;

    // 1. Génération du labyrinthe (votre code existant)
    arete *toutes_aretes;
    int nb_total_aretes = generation_grille_vide(&toutes_aretes, lignes, colonnes);
    
    fisher_yates(toutes_aretes, nb_total_aretes);

    arete *arbre = malloc(sizeof(arete) * (nb_cellules - 1));
    int nb_aretes_arbre;
    construire_arbre_couvrant(toutes_aretes, nb_total_aretes, arbre, &nb_aretes_arbre, nb_cellules);
    
    free(toutes_aretes);

    // Créer la représentation avec les murs
    int *murs = malloc(sizeof(int) * nb_cellules);
    for (int i = 0; i < nb_cellules; i++) {
        murs[i] = 1 | 2 | 4 | 8; // Tous les murs sont présents au début
    }
    for (int i = 0; i < nb_aretes_arbre; i++) {
        supprimer_mur(murs, colonnes, arbre[i].u, arbre[i].v);
    }
    free(arbre);
    
    // Définir le point de départ et d'arrivée
    int depart = 0; // Coin supérieur gauche
    int destination = nb_cellules - 1; // Coin inférieur droit

    // 2. Afficher le labyrinthe résolu avec SDL
    printf("Affichage du labyrinthe résolu avec le chemin et le dégradé de distance...\n");
    afficher_labyrinthe_resolu_sdl(murs, lignes, colonnes, depart, destination);
    
    // Libérer la mémoire des murs
    free(murs);

    printf("Programme terminé.\n");
    return 0;
}
*/


/*
//affichage seul
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

    
    srand(time(NULL));
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
    //generer_dot("graphe.dot", graphe, nb_aretes);
    //generer_dot("arbre.dot", arbre, nb_arbre);
    //printf("Fichiers DOT générés : graphe.dot et arbre.dot\n");

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
*/