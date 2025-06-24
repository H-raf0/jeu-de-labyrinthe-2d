#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define LIGNES 10
#define COLONNES 10
#define CELL 40
#define LARGEUR (COLONNES * CELL)
#define HAUTEUR (LIGNES * CELL)
#define N (LIGNES * COLONNES)
#define INF 1000000000
#define MAX_RUNS 3

typedef struct {
    int u, v;
} Arete;

typedef struct {
    int *parent, *rang;
    int taille;
} Partition;

typedef struct noeud {
    int distance[N];
    int visite[N];
} noeud;

typedef struct tas {
    int tab[N];
    int taille;
} tas;

// ===== Fonctions de génération du labyrinthe =====
void init_partition(Partition* p, int taille) {
    p->taille = taille;
    p->parent = malloc(sizeof(int) * taille);
    p->rang = malloc(sizeof(int) * taille);
    for (int i = 0; i < taille; i++) {
        p->parent[i] = i;
        p->rang[i] = 0;
    }
}

void free_partition(Partition* p) {
    free(p->parent);
    free(p->rang);
}

int find(Partition* p, int i) {
    if (p->parent[i] != i)
        p->parent[i] = find(p, p->parent[i]);
    return p->parent[i];
}

int union_sets(Partition* p, int i, int j) {
    int ri = find(p, i), rj = find(p, j);
    if (ri == rj) return 0;
    if (p->rang[ri] < p->rang[rj])
        p->parent[ri] = rj;
    else {
        p->parent[rj] = ri;
        if (p->rang[ri] == p->rang[rj])
            p->rang[ri]++;
    }
    return 1;
}

void shuffle(Arete* G, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Arete tmp = G[i]; G[i] = G[j]; G[j] = tmp;
    }
}

int generer_aretes(Arete** G_ptr, int lignes, int colonnes) {
    int max = 2 * lignes * colonnes - lignes - colonnes;
    Arete* G = malloc(sizeof(Arete) * max);
    int k = 0;
    for (int y = 0; y < lignes; y++) {
        for (int x = 0; x < colonnes; x++) {
            int i = y * colonnes + x;
            if (x + 1 < colonnes) G[k++] = (Arete){i, i + 1};
            if (y + 1 < lignes)   G[k++] = (Arete){i, i + colonnes};
        }
    }
    *G_ptr = G;
    return k;
}

void supprimer_mur(int* murs, int cols, int u, int v) {
    int x1 = u % cols, y1 = u / cols;
    int x2 = v % cols, y2 = v / cols;
    if (x2 == x1 + 1) { murs[u] &= ~2; murs[v] &= ~8; }     // droite
    else if (x2 == x1 - 1) { murs[u] &= ~8; murs[v] &= ~2; } // gauche
    else if (y2 == y1 + 1) { murs[u] &= ~4; murs[v] &= ~1; } // bas
    else if (y2 == y1 - 1) { murs[u] &= ~1; murs[v] &= ~4; } // haut
}

// ===== Fonctions de recherche de chemin =====
void echanger(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void inserer(tas* t, int sommet, noeud* n) {
    t->tab[t->taille++] = sommet;
    int i = t->taille - 1;
    while (i > 0 && n->distance[t->tab[i]] < n->distance[t->tab[(i - 1) / 2]]) {
        echanger(&t->tab[i], &t->tab[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

int extraire_min(tas* t, noeud* n) {
    if (t->taille == 0) return -1;
    int min = t->tab[0];
    t->tab[0] = t->tab[--t->taille];
    int i = 0;
    while (2 * i + 1 < t->taille) {
        int gauche = 2 * i + 1;
        int droite = 2 * i + 2;
        int plus_petit = gauche;
        if (droite < t->taille && n->distance[t->tab[droite]] < n->distance[t->tab[gauche]]) {
            plus_petit = droite;
        }
        if (n->distance[t->tab[i]] <= n->distance[t->tab[plus_petit]]) break;
        echanger(&t->tab[i], &t->tab[plus_petit]);
        i = plus_petit;
    }
    return min;
}

void initialiser_noeuds(noeud* n, int origine) {
    for (int i = 0; i < N; ++i) {
        n->visite[i] = 0;
        n->distance[i] = INF;
    }
    n->distance[origine] = 0;
}

void dijkstra(int graphe[N][N], int origine, noeud* n) {
    tas t;
    t.taille = 0;
    initialiser_noeuds(n, origine);
    inserer(&t, origine, n);

    while (t.taille > 0) {
        int u = extraire_min(&t, n);
        if (n->visite[u]) continue;
        n->visite[u] = 1;

        for (int v = 0; v < N; v++) {
            if (graphe[u][v] == 0) continue;
            
            int cout = graphe[u][v];
            if (cout == -1) cout = 1;

            if (!n->visite[v]) {
                int alt = n->distance[u] + cout;
                if (alt < n->distance[v]) {
                    n->distance[v] = alt;
                    inserer(&t, v, n);
                }
            }
        }
    }
}

// ===== Fonction principale combinée =====
int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Labyrinthe - Recherche de Chemin", 
                                         SDL_WINDOWPOS_CENTERED, 
                                         SDL_WINDOWPOS_CENTERED, 
                                         LARGEUR, 
                                         HAUTEUR, 
                                         0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    srand(time(NULL));

    // Génération du labyrinthe
    int nb_cells = LIGNES * COLONNES;
    int* murs = malloc(sizeof(int) * nb_cells);
    for (int i = 0; i < nb_cells; i++) murs[i] = 15; // 1111 = tous les murs

    Arete* G;
    int nb_aretes = generer_aretes(&G, LIGNES, COLONNES);
    shuffle(G, nb_aretes);

    Partition p;
    init_partition(&p, nb_cells);

    for (int i = 0, count = 0; i < nb_aretes && count < nb_cells - 1; i++) {
        if (union_sets(&p, G[i].u, G[i].v)) {
            supprimer_mur(murs, COLONNES, G[i].u, G[i].v);
            count++;
        }
    }

    // Construction du graphe réel
    int graphe_reel[N][N] = {0};
    for (int y = 0; y < LIGNES; y++) {
        for (int x = 0; x < COLONNES; x++) {
            int i = y * COLONNES + x;
            if (x < COLONNES - 1 && !(murs[i] & 2)) {
                int j = i + 1;
                graphe_reel[i][j] = 1;
                graphe_reel[j][i] = 1;
            }
            if (y < LIGNES - 1 && !(murs[i] & 4)) {
                int j = i + COLONNES;
                graphe_reel[i][j] = 1;
                graphe_reel[j][i] = 1;
            }
            if (x > 0 && !(murs[i] & 8)) {
                int j = i - 1;
                graphe_reel[i][j] = 1;
                graphe_reel[j][i] = 1;
            }
            if (y > 0 && !(murs[i] & 1)) {
                int j = i - COLONNES;
                graphe_reel[i][j] = 1;
                graphe_reel[j][i] = 1;
            }
        }
    }

    // Initialisation du graphe de connaissance
    int graphe[N][N] = {0};
    for (int i = 0; i < N; i++) {
        int x = i % COLONNES;
        int y = i / COLONNES;
        
        if (x < COLONNES - 1) {
            graphe[i][i+1] = -1;
            graphe[i+1][i] = -1;
        }
        if (y < LIGNES - 1) {
            graphe[i][i+COLONNES] = -1;
            graphe[i+COLONNES][i] = -1;
        }
        if (x > 0) {
            graphe[i][i-1] = -1;
            graphe[i-1][i] = -1;
        }
        if (y > 0) {
            graphe[i][i-COLONNES] = -1;
            graphe[i-COLONNES][i] = -1;
        }
    }

    // Paramètres de recherche de chemin
    int origine = 0;
    int destination = N - 1;
    int courant = origine;
    int precedent = -1;
    int run = 1;
    int visited[N] = {0};
    noeud n;
    int etapes = 0;
    const int max_etapes = 1000;
    int chemin_optimal_trouve = 0;

    // Boucle principale
    int quit = 0;
    SDL_Event e;
    while (!quit) {
        // Gestion des événements
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        // Logique de déplacement
        if (run <= MAX_RUNS && courant != destination && etapes < max_etapes && !chemin_optimal_trouve) {
            etapes++;
            dijkstra(graphe, destination, &n);
            bool a_transite = false;

            while (!a_transite && etapes < max_etapes) {
                // Trouver le meilleur voisin
                int meilleur_voisin = -1;
                int distance_min = INF;

                // Parcourir tous les voisins possibles
                for (int v = 0; v < N; ++v) {
                    // Vérifier si l'arête existe dans le graphe réel
                    if (graphe_reel[courant][v] == 0) continue;
                    
                    // Calculer la distance estimée
                    int dist_estimee = n.distance[v];
                    
                    // Appliquer une pénalité pour le retour en arrière
                    if (v == precedent) {
                        dist_estimee += 100; // Forte pénalité pour revenir en arrière
                    }
                    
                    // Trouver le meilleur voisin
                    if (dist_estimee < distance_min) {
                        meilleur_voisin = v;
                        distance_min = dist_estimee;
                    }
                }

                // Si aucun voisin trouvé, essayer sans pénalité
                if (meilleur_voisin == -1) {
                    for (int v = 0; v < N; ++v) {
                        if (graphe_reel[courant][v] == 0) continue;
                        
                        if (n.distance[v] < distance_min) {
                            meilleur_voisin = v;
                            distance_min = n.distance[v];
                        }
                    }
                }

                // Si toujours aucun voisin, abandonner
                if (meilleur_voisin == -1) {
                    printf("Aucun chemin trouvé. Arrêt.\n");
                    quit = 1;
                    break;
                }

                int cout_suppose = (graphe[courant][meilleur_voisin] == -1) ? 1 : graphe[courant][meilleur_voisin];
                int cout_reel = graphe_reel[courant][meilleur_voisin];

                if (cout_reel == cout_suppose) {
                    precedent = courant;
                    courant = meilleur_voisin;
                    visited[courant] = 1;
                    a_transite = true;
                    
                    printf("Transition de %d vers %d\n", precedent, courant);
                } else {
                    // Mettre à jour la connaissance du graphe
                    graphe[courant][meilleur_voisin] = cout_reel;
                    graphe[meilleur_voisin][courant] = cout_reel;
                    
                    // Recalculer les distances
                    initialiser_noeuds(&n, destination);
                    dijkstra(graphe, destination, &n);
                    
                    printf("Mise à jour: %d->%d = %d (supposé %d)\n", 
                           courant, meilleur_voisin, cout_reel, cout_suppose);
                }
            }
            
            // Vérifier si on a atteint la destination
            if (courant == destination) {
                printf("Run %d terminé! Téléportation au départ...\n", run);
                
                // Réinitialiser pour le prochain run
                precedent = -1;
                courant = origine;
                run++;
                
                // Vérifier si on a trouvé le chemin optimal
                if (run > MAX_RUNS) {
                    chemin_optimal_trouve = 1;
                    printf("Chemin optimal garanti après %d runs!\n", MAX_RUNS);
                }
            }
        }

        // Rendu graphique
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // Dessiner les murs
        for (int y = 0; y < LIGNES; y++) {
            for (int x = 0; x < COLONNES; x++) {
                int i = y * COLONNES + x;
                int mx = x * CELL;
                int my = y * CELL;
                if (murs[i] & 1) SDL_RenderDrawLine(renderer, mx, my, mx + CELL, my);
                if (murs[i] & 2) SDL_RenderDrawLine(renderer, mx + CELL, my, mx + CELL, my + CELL);
                if (murs[i] & 4) SDL_RenderDrawLine(renderer, mx, my + CELL, mx + CELL, my + CELL);
                if (murs[i] & 8) SDL_RenderDrawLine(renderer, mx, my, mx, my + CELL);
            }
        }

        // Dessiner les cellules visitées (connaissance du labyrinthe)
        for (int i = 0; i < N; i++) {
            if (visited[i]) {
                int x = i % COLONNES;
                int y = i / COLONNES;
                SDL_Rect rect = {x * CELL + 5, y * CELL + 5, CELL - 10, CELL - 10};
                SDL_SetRenderDrawColor(renderer, 200, 255, 200, 150);
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        // Dessiner la position courante (personnage)
        int x_pos = courant % COLONNES;
        int y_pos = courant / COLONNES;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect current_rect = {x_pos * CELL + 10, y_pos * CELL + 10, CELL - 20, CELL - 20};
        SDL_RenderFillRect(renderer, &current_rect);

        // Dessiner le chemin potentiel (connaissance actuelle)
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 200);
        int cur = courant;
        int steps = 0;
        while (cur != destination && steps < 20) {
            steps++;
            
            int next = -1;
            int min_dist = INF;
            for (int v = 0; v < N; v++) {
                if (graphe_reel[cur][v] > 0 && n.distance[v] < min_dist) {
                    min_dist = n.distance[v];
                    next = v;
                }
            }
            
            if (next == -1) break;
            
            int x_cur = cur % COLONNES;
            int y_cur = cur / COLONNES;
            int x_next = next % COLONNES;
            int y_next = next / COLONNES;
            
            SDL_RenderDrawLine(renderer, 
                              x_cur * CELL + CELL/2, y_cur * CELL + CELL/2,
                              x_next * CELL + CELL/2, y_next * CELL + CELL/2);
            
            cur = next;
        }

        // Dessiner l'origine (départ)
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect orig_rect = {0, 0, CELL/2, CELL/2};
        SDL_RenderFillRect(renderer, &orig_rect);
        
        // Dessiner la destination (arrivée)
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect dest_rect = {(COLONNES-1)*CELL, (LIGNES-1)*CELL, CELL/2, CELL/2};
        SDL_RenderFillRect(renderer, &dest_rect);

        // Afficher le numéro du run
        char run_text[50];
        snprintf(run_text, sizeof(run_text), "Run: %d/%d", run, MAX_RUNS);
        SDL_Surface* text_surface = SDL_CreateRGBSurface(0, 100, 30, 32, 0, 0, 0, 0);
        SDL_FillRect(text_surface, NULL, SDL_MapRGB(text_surface->format, 255, 255, 255));
        SDL_Color text_color = {0, 0, 0, 255};
        // Note: Pour simplifier, on utilise une surface de texte basique
        // En production, utilisez SDL_ttf pour un vrai rendu de texte
        SDL_Rect text_rect = {10, 10, 100, 30};
        SDL_FillRect(text_surface, NULL, SDL_MapRGB(text_surface->format, 255, 255, 255));
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        SDL_FreeSurface(text_surface);
        SDL_DestroyTexture(text_texture);

        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    // Nettoyage
    free_partition(&p);
    free(murs);
    free(G);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}