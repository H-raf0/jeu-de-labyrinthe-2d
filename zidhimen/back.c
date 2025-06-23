#include <stdlib.h>
#include <stdio.h>
#include<stdbool.h>
#include <math.h>
#include <stdbool.h>
#define N 10
#define INF 3000000
typedef struct file{
    int tab[N];
    int tete;
    int queue;
}file;
typedef struct noeud{
    int distance[N];
    int visite[N];
}noeud;
void initialiser_noeuds(noeud* n,int origine){
    for(int i=0;i<N;++i){
        n->visite[i]=0;
        n->distance[i]=INF;
    }
    n->distance[origine]=0;
}

void initialiser_file(file * f){
    f->tete=0;
    f->queue=0;
}

int filevide(file *f){
    return f->tete==f->queue;
}

void enfiler(file * f , int x){
    if(f->queue==N){
        printf("file pleine\n");
        return;
    }
    f->tab[f->queue]=x;
    f->queue = (f->queue + 1) % N;
}

int defiler(file *f){
    if(filevide(f)){
        printf("file vide\n");
        return 0;
    }
    int x=f->tab[f->tete];
    f->tete = (f->tete + 1) % N;
    return x;
}
/**********************************************
                    BFS
***********************************************/
void BFS(int m_adj[N][N], int origine,noeud* n){
    file * f = malloc(sizeof(file));
    initialiser_file(f);
    initialiser_noeuds(n ,origine);
    n->visite[origine]=1;
    enfiler(f,origine);
    while(!filevide(f)){
        int x=defiler(f);
        for(int k=0;k<N;++k){
            if(n->visite[k]==0 && m_adj[x][k]==1){
                enfiler(f,k);
                n->visite[k]=1;
                n->distance[k]=n->distance[x]+1;
            }
        }
    }
    free(f);
}
int min (int a,int b, int c, int d){
    if(a<=b &&a<=c && a<=d){
        return a;
    }
    else if(b<=a &&b<=c && b<=d){
        return b;
    }
    else if(c<=b &&c<=a && c<=d){
        return c;
    }
    else if(d<=b &&d<=c && d<=a){
        return d ;
    }
    
}
void Recherche( int m_adj[N][N], int origine ,int destination){
    int a,b,c,d,nv_origine;
    noeud *n =NULL;
    initialiser_noeuds(n ,origine);
    BFS(m_adj,destination,n);
    bool a_transité = false;
    while(!a_transité){
        if(m_adj[origine][origine+N]!=0){
            if(origine+N==destination){
                a_transité=true;
            }
            a=n->distance[origine+N];
        }
        if(m_adj[origine][origine-N]!=0){
            if(origine-N==destination){
                a_transité=true;
            }
            b=n->distance[origine-N];
        }
        if(m_adj[origine][origine+1]!=0){
            if(origine+1==destination){
                a_transité=true;
            }
            c=n->distance[origine+1];
        }
        if(m_adj[origine][origine-1]!=0){
            if(origine-1==destination){
                a_transité=true;
            }
            d=n->distance[origine-1];
        }
        int nv_origine=min(a,b,c,d);
        Recherche(m_adj,nv_origine,destination);


    }

}
