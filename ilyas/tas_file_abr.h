#ifndef TAS_FILE_ABR_H
#define TAS_FILE_ABR_H
typedef struct {
    int data[MAX_SIZE];
    int size;
} tas;

/*typedef struct {
    int data[MAX_SIZE];  // Tableau de stockage
    int front;           // Indice de la tête de la file
    int rear;            // Indice de la queue
    int size;            // Nombre d’éléments dans la file
} CircularQueue;*/

typedef struct file_element {
    int value;
    struct queue_element * previous;
    struct queue_element * next;
} queue_element;



typedef struct file {
    int size;
    struct queue_element * head;
}


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