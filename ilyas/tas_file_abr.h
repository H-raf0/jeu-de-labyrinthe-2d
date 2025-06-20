#ifndef TAS_FILE_ABR_H
#define TAS_FILE_ABR_H
typedef struct {
    int valeur;
    int priorite; // plus la valeur est grande, plus la priorité est haute
} Noeud;

typedef struct {
    Noeud data[MAX_SIZE];
    int size;
} tas;



struct bst {
        int key;
        struct bst * left;
        struct bst * right;
};
struct bst * bstCreateNode(int key) ;
void bstInorderTraversal(struct bst * root) ;
struct bst * bstInsert(struct bst * root, int key) ;
struct bst * bstMinValue(struct bst * root) ;
struct bst * bstDelete(struct bst * root, int key) ;
void bstFree(struct bst * root) ;
void bstDisplay(struct bst * root) ;























#endif