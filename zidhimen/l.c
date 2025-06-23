#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define N 8
#define INF 9999

// ... (toutes vos structures et fonctions de base : noeud, file, tas, dijkstra, etc. sont supposées être ici)
// ... (Je remets ici les fonctions nécessaires de votre code pour que l'exemple soit complet)

// --- VOS FONCTIONS EXISTANTES (simplifiées pour la clarté) ---
typedef struct noeud { int distance[N]; int visite[N]; } noeud;
typedef struct tas { int tab[N]; int taille; } tas;
void echanger(int *a, int *b) { int temp = *a; *a = *b; *b = temp; }
void entasser_bas(tas *t, int i, noeud *n) {
    int min = i, g = 2 * i + 1, d = 2 * i + 2;
    if (g < t->taille && n->distance[t->tab[g]] < n->distance[t->tab[min]]) min = g;
    if (d < t->taille && n->distance[t->tab[d]] < n->distance[t->tab[min]]) min = d;
    if (min != i) { echanger(&t->tab[i], &t->tab[min]); entasser_bas(t, min, n); }
}
void entasser_haut(tas *t, int i, noeud *n) {
    while (i > 0 && n->distance[t->tab[i]] < n->distance[t->tab[(i - 1) / 2]]) {
        echanger(&t->tab[i], &t->tab[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}
void inserer(tas *t, int sommet, noeud *n) { t->tab[t->taille] = sommet; entasser_haut(t, t->taille, n); t->taille++; }
int extraire_min(tas *t, noeud *n) { int min = t->tab[0]; t->tab[0] = t->tab[--t->taille]; entasser_bas(t, 0, n); return min; }
void initialiser_noeuds(noeud* n, int origine){
    for(int i=0;i<N;++i){ n->visite[i]=0; n->distance[i]=INF; }
    n->distance[origine]=0;
}
void dijkstra(int graphe[N][N], int origine, noeud *n) {
    tas t; t.taille = 0;
    initialiser_noeuds(n,origine);
    inserer(&t, origine, n);
    while (t.taille > 0) {
        int u = extraire_min(&t, n);
        if (n->visite[u]) continue;
        n->visite[u] = 1;
        for (int v = 0; v < N; v++) {
            if (graphe[u][v] > 0 && graphe[u][v] != INF && !n->visite[v]) {
                int alt = n->distance[u] + graphe[u][v];
                if (alt < n->distance[v]) { n->distance[v] = alt; inserer(&t, v, n); }
            }
        }
    }
}
// --- FIN DE VOS FONCTIONS ---


// --- NOUVELLES FONCTIONS POUR L'ALGORITHME LPA* ---

// Crée une carte mentale optimiste : toutes les arêtes adjacentes coûtent 1.
void initialiser_graphe_connu(int graphe_connu[N][N], int matrice_adjacence_base[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            // On suppose que le coût est 1 si les noeuds sont adjacents, sinon infini.
            // On se base sur une topologie de base (ex: grille)
            graphe_connu[i][j] = matrice_adjacence_base[i][j];
            if (i == j) graphe_connu[i][j] = 0;
        }
    }
}

// Calcule h(n) pour tout n : distance estimée de n à la destination
void calculer_estimations(int destination, int graphe_connu[N][N], int h_distances[N]) {
    noeud n;
    // On lance Dijkstra "à l'envers" depuis la destination
    dijkstra(graphe_connu, destination, &n);
    printf("Recalcul des estimations vers la destination %d:\n", destination);
    for (int i = 0; i < N; i++) {
        h_distances[i] = n.distance[i];
        printf("  h(%d) = %d\n", i, h_distances[i]);
    }
    printf("\n");
}

// Choisit le voisin qui semble le plus proche de la destination
int choisir_prochain_pas(int pos_actuelle, int graphe_connu[N][N], int h_distances[N]) {
    int meilleur_voisin = -1;
    int min_h = INF;

    for (int v = 0; v < N; v++) {
        // Si v est un voisin connu
        if (graphe_connu[pos_actuelle][v] > 0 && graphe_connu[pos_actuelle][v] != INF) {
            if (h_distances[v] < min_h) {
                min_h = h_distances[v];
                meilleur_voisin = v;
            }
        }
    }
    return meilleur_voisin;
}

// Affiche l'état actuel de l'agent
void afficher_etat(int pos_actuelle, int graphe_connu[N][N], int h_distances[N]) {
    printf("----------------------------------\n");
    printf("Agent en position: %d\n", pos_actuelle);
    printf("Connaissance actuelle du graphe (coût) :\n");
    for(int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if(graphe_connu[i][j] == INF) printf("  X ");
            else printf("%3d ", graphe_connu[i][j]);
        }
        printf("\n");
    }
    printf("Estimations (h) actuelles vers la destination:\n  [ ");
    for(int i=0; i<N; i++) printf("%d:%d ", i, h_distances[i]);
    printf("]\n");
    printf("----------------------------------\n\n");
}

/**
 * Implémentation de l'algorithme de replanification.
 * Si une surprise est rencontrée (un coût réel est > au coût connu),
 * l'agent met à jour sa carte et recommence un "run" complet depuis le départ.
 */
void LPA_etoile_teleportation(int graphe_reel[N][N], int matrice_adjacence_base[N][N], int depart, int destination) {
    int graphe_connu[N][N];
    int h_distances[N];
    
    // Initialisation de la connaissance
    initialiser_graphe_connu(graphe_connu, matrice_adjacence_base);
    
    bool chemin_optimal_garanti = false;
    int nombre_runs = 0;

    while (!chemin_optimal_garanti) {
        nombre_runs++;
        printf(">>>>>>>>>>>>>>>>>> DEBUT DU RUN #%d <<<<<<<<<<<<<<<<<<\n\n", nombre_runs);

        // 1. Calculer les estimations basées sur la connaissance actuelle
        calculer_estimations(destination, graphe_connu, h_distances);

        int pos_actuelle = depart;
        bool surprise_rencontree = false;
        
        int chemin_suivi[N*N]; // Pour stocker le chemin du run
        int etapes = 0;
        chemin_suivi[etapes++] = pos_actuelle;

        // 2. Boucle de déplacement
        while (pos_actuelle != destination) {
            afficher_etat(pos_actuelle, graphe_connu, h_distances);
            
            // 2a. Choisir le voisin le plus prometteur
            int prochain_pas = choisir_prochain_pas(pos_actuelle, graphe_connu, h_distances);

            if (prochain_pas == -1) {
                printf("BLOQUE ! Aucun chemin connu depuis %d.\n", pos_actuelle);
                surprise_rencontree = true;
                break;
            }

            printf("Action: Essayer de se deplacer de %d a %d (h=%d)...\n", pos_actuelle, prochain_pas, h_distances[prochain_pas]);
            
            // 2b. Confronter la connaissance à la réalité
            int cout_connu = graphe_connu[pos_actuelle][prochain_pas];
            int cout_reel = graphe_reel[pos_actuelle][prochain_pas];

            if (cout_reel > cout_connu) {
                // 2c. SURPRISE !
                printf("SURPRISE! Le cout reel de %d->%d est %d (attendu: %d). C'est un mur !\n", pos_actuelle, prochain_pas, cout_reel, cout_connu);
                
                // Mettre à jour la connaissance
                graphe_connu[pos_actuelle][prochain_pas] = cout_reel; // On met INF
                graphe_connu[prochain_pas][pos_actuelle] = cout_reel; // Graphe non-orienté

                surprise_rencontree = true;
                break; // Interrompt ce run pour en recommencer un nouveau
            } else {
                // 2d. Pas de surprise, on avance
                printf("Deplacement reussi de %d a %d.\n\n", pos_actuelle, prochain_pas);
                pos_actuelle = prochain_pas;
                chemin_suivi[etapes++] = pos_actuelle;
            }
        } // Fin de la boucle de déplacement

        // 3. Fin d'un run
        if (surprise_rencontree) {
            printf("\n--- Run #%d interrompu. Mise a jour de la carte. On se teleporte au depart. ---\n\n", nombre_runs);
        } else {
            // Si on arrive ici sans surprise, le chemin suivi est optimal
            printf("\n*** Destination %d atteinte sans surprise durant le Run #%d ! ***\n", destination, nombre_runs);
            printf("Le chemin optimal garanti a ete trouve.\n");
            printf("Chemin final: ");
            for(int i = 0; i < etapes; i++) {
                printf("%d %s", chemin_suivi[i], (i == etapes - 1) ? "" : "-> ");
            }
            printf("\n");
            chemin_optimal_garanti = true;
        }
    }
}


int main() {
    // Le graphe de base définit la topologie (quels noeuds sont adjacents)
    // C'est un arbre simple.
    int matrice_adj_base[N][N] = {
        {0, 1, 1, 0, 0, 0, 0, 0}, // 0
        {1, 0, 0, 1, 1, 0, 0, 0}, // 1
        {1, 0, 0, 0, 0, 1, 1, 0}, // 2
        {0, 1, 0, 0, 0, 0, 0, 0}, // 3
        {0, 1, 0, 0, 0, 0, 0, 0}, // 4
        {0, 0, 1, 0, 0, 0, 0, 1}, // 5
        {0, 0, 1, 0, 0, 0, 0, 0}, // 6
        {0, 0, 0, 0, 0, 1, 0, 0}  // 7
    };

    // La REALITE : On place un "mur" (coût infini) entre les noeuds 2 et 5
    int graphe_reel[N][N];
    for(int i=0; i<N; i++) for(int j=0; j<N; j++) graphe_reel[i][j] = matrice_adj_base[i][j];
    graphe_reel[2][5] = INF;
    graphe_reel[5][2] = INF;

    int depart = 0;
    int destination = 7;

    printf("--- Probleme: Aller de %d a %d dans un graphe inconnu ---\n", depart, destination);
    printf("Un mur secret se trouve entre 2 et 5.\n\n");

    LPA_etoile_teleportation(graphe_reel, matrice_adj_base, depart, destination);
    
    return 0;
}