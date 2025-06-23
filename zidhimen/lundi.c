#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

// On définit une grille 10x10, donc 100 noeuds au total.
#define LARGEUR_GRILLE 10
#define N (LARGEUR_GRILLE * LARGEUR_GRILLE)
#define INF 3000000

// La structure de la file est correcte
typedef struct file{
    int tab[N];
    int tete;
    int queue;
}file;

// La structure noeud stocke les résultats du parcours
typedef struct noeud{
    int distance[N];
    int visite[N];
    int predecesseur[N]; // Utile pour reconstruire le chemin
}noeud;

// --- Fonctions utilitaires pour la file (votre code, légèrement amélioré) ---

void initialiser_file(file * f){
    f->tete = 0;
    f->queue = 0;
}

int filevide(file *f){
    return f->tete == f->queue;
}

void enfiler(file * f , int x){
    // Utilisation correcte de l'opérateur modulo pour une file circulaire
    if((f->queue + 1) % N == f->tete){
        //printf("file pleine\n"); // On peut enlever ce message pour ne pas polluer l'output
        return;
    }
    f->tab[f->queue] = x;
    f->queue = (f->queue + 1) % N;
}

int defiler(file *f){
    if(filevide(f)){
        //printf("file vide\n"); // Idem
        return -1; // Renvoyer une valeur d'erreur est plus sûr
    }
    int x = f->tab[f->tete];
    f->tete = (f->tete + 1) % N;
    return x;
}

// --- Fonctions de l'algorithme (votre code et les ajouts) ---

void initialiser_noeuds(noeud* n, int origine){
    for(int i = 0; i < N; ++i){
        n->visite[i] = 0;
        n->distance[i] = INF;
        n->predecesseur[i] = -1;
    }
    n->distance[origine] = 0;
}

/**********************************************
                    BFS
    Calcule la distance depuis 'origine' à tous les
    autres noeuds dans le graphe m_adj.
***********************************************/
void BFS(int m_adj[N][N], int origine, noeud* n){
    file f; // Pas besoin d'allouer dynamiquement pour une variable locale
    initialiser_file(&f);
    initialiser_noeuds(n, origine);

    n->visite[origine] = 1;
    enfiler(&f, origine);

    while(!filevide(&f)){
        int u = defiler(&f);
        // On parcourt tous les voisins potentiels
        for(int v = 0; v < N; ++v){
            // S'il y a une arête (pas un mur) et que le noeud n'a pas été visité
            if(m_adj[u][v] == 1 && n->visite[v] == 0){
                enfiler(&f, v);
                n->visite[v] = 1;
                n->distance[v] = n->distance[u] + 1;
                n->predecesseur[v] = u;
            }
        }
    }
}

/**********************************************
          RECHERCHE INTELLIGENTE (D* like)
***********************************************/
void recherche_intelligente(int monde_reel[N][N], int carte_connue[N][N], int origine, int destination){
    int position_actuelle = origine;
    noeud estimations_distance;
    int chemin[N];
    int etape_chemin = 0;
    chemin[etape_chemin++] = position_actuelle;

    printf("Début de la recherche de %d à %d.\n", origine, destination);

    while(position_actuelle != destination){
        // 1. Calculer les estimations de distance depuis la destination sur la CARTE CONNUE
        printf("\n--- Itération depuis la position %d ---\n", position_actuelle);
        printf("Planification: Calcul du chemin optimal sur la carte connue...\n");
        BFS(carte_connue, destination, &estimations_distance);

        // Si la destination est inatteignable sur la carte connue
        if(estimations_distance.distance[position_actuelle] == INF){
            printf("ERREUR: La destination %d est inatteignable depuis %d selon la carte actuelle. Abandon.\n", destination, position_actuelle);
            return;
        }

        bool a_transite = false;
        while(!a_transite){
            // 2. Déterminer le meilleur voisin où aller depuis la position actuelle
            int meilleur_voisin = -1;
            int dist_min_voisin = INF;

            // Voisins potentiels (haut, bas, gauche, droite)
            int voisins[4];
            voisins[0] = position_actuelle - LARGEUR_GRILLE; // Haut
            voisins[1] = position_actuelle + LARGEUR_GRILLE; // Bas
            voisins[2] = position_actuelle - 1;              // Gauche
            voisins[3] = position_actuelle + 1;              // Droite
            
            // Logique pour éviter de passer d'un bord à l'autre (ex: de 20 à 19)
            if (position_actuelle % LARGEUR_GRILLE == 0) voisins[2] = -1; // Pas de voisin à gauche
            if ((position_actuelle + 1) % LARGEUR_GRILLE == 0) voisins[3] = -1; // Pas de voisin à droite

            for(int i = 0; i < 4; i++){
                int v = voisins[i];
                // Si le voisin est valide et a une distance estimée plus faible
                if (v >= 0 && v < N && estimations_distance.distance[v] < dist_min_voisin) {
                    dist_min_voisin = estimations_distance.distance[v];
                    meilleur_voisin = v;
                }
            }

            if (meilleur_voisin == -1) {
                printf("ERREUR: Bloqué ! Aucun voisin accessible trouvé. Abandon.\n");
                return;
            }
            
            printf("Décision: Le meilleur mouvement semble être vers %d (distance estimée: %d).\n", meilleur_voisin, dist_min_voisin);

            // 3. Tenter le déplacement et mettre à jour la carte si nécessaire
            // On "regarde" le monde réel
            if(monde_reel[position_actuelle][meilleur_voisin] == 1){
                printf("Action: Mouvement vers %d réussi.\n", meilleur_voisin);
                position_actuelle = meilleur_voisin;
                chemin[etape_chemin++] = position_actuelle;
                a_transite = true; // On a bougé, on sort de la boucle interne pour replanifier
            } else {
                printf("APPRENTISSAGE: Obstacle imprévu entre %d et %d!\n", position_actuelle, meilleur_voisin);
                printf("Mise à jour de la carte...\n");
                // On met à jour notre carte avec l'information du mur
                carte_connue[position_actuelle][meilleur_voisin] = 0;
                carte_connue[meilleur_voisin][position_actuelle] = 0; // Le graphe est non-orienté
                // On ne bouge pas. La boucle 'while(!a_transite)' va recommencer, mais
                // comme la boucle externe va se relancer, elle va d'abord faire un nouveau BFS
                // avec la carte mise à jour. On peut donc directement sortir.
                break; 
            }
        }
    }

    printf("\n>>> Destination %d atteinte! <<<\n", destination);
    printf("Chemin final trouvé: ");
    for(int i = 0; i < etape_chemin; i++){
        printf("%d ", chemin[i]);
    }
    printf("\n");
}


// --- Programme principal pour tester l'algorithme ---
int main() {
    int monde_reel[N][N] = {0};   // Le vrai labyrinthe, inconnu de l'algo au début
    int carte_connue[N][N] = {0}; // La carte que l'algo construit

    // Initialisation des graphes
    // 1 = chemin possible, 0 = mur
    for(int i=0; i<N; i++){
        for(int j=0; j<N; j++){
            // On vérifie si j est un voisin direct de i (haut, bas, gauche, droite)
            bool est_voisin = (j == i - 1 && i % LARGEUR_GRILLE != 0) ||
                              (j == i + 1 && (i + 1) % LARGEUR_GRILLE != 0) ||
                              (j == i - LARGEUR_GRILLE) ||
                              (j == i + LARGEUR_GRILLE);
            if(est_voisin){
                monde_reel[i][j] = 1;
                carte_connue[i][j] = 1; // Hypothèse optimiste initiale
            }
        }
    }

    // Ajoutons quelques murs dans le "monde_reel" que l'algo ne connaît pas
    // Exemple: un mur vertical
    monde_reel[14][24] = monde_reel[24][14] = 0;
    monde_reel[24][34] = monde_reel[34][24] = 0;
    monde_reel[34][44] = monde_reel[44][34] = 0;
    // La case 44 est maintenant bloquée par le haut
    // monde_reel[44][54] = monde_reel[54][44] = 0; // Décommenter pour un chemin plus complexe
    monde_reel[54][64] = monde_reel[64][54] = 0;
    monde_reel[64][74] = monde_reel[74][64] = 0;
    
    // Définir l'origine et la destination
    int origine = 21;
    int destination = 28;

    // Lancer la recherche
    recherche_intelligente(monde_reel, carte_connue, origine, destination);

    return 0;
}
