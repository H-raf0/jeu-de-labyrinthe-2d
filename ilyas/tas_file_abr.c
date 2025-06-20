#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#include "TAS_FILE_ABR_H"


typedef struct {
    int valeur;
    int priorite; // plus grand = plus prioritaire
} Noeud;

typedef struct {
    Noeud *data;
    int size;
    int capacite;
} tas;

typedef struct file_element {
    int value;
    struct queue_element * previous;
    struct queue_element * next;
} queue_element;



typedef struct file {
    int size;
    struct queue_element * head;
}file;


struct bst {
        int key;
        struct bst * left;
        struct bst * right;
};
//tas_binaire:

void echanger(Noeud * a, Noeud *b)
{
    Noeud c = *a;
    *a = *b;
    *b = c;
}
bool est_vide(tas *t) {
    return t->size == 0;
}

tas *creer_tas(int capacite) {
    tas *t = malloc(sizeof(tas));
    t->data = malloc(sizeof(Noeud) * capacite);
    t->size = 0;
    t->capacite = capacite;
    return t;
}

tas *inserer_elmt(tas *a, int valeur, int priorite) {
    if (!a || a->size >= a->capacite) return a;

    int i = a->size++;
    a->data[i].valeur = valeur;
    a->data[i].priorite = priorite;

    while (i > 0 && a->data[i].priorite > a->data[(i - 1) / 2].priorite) {
        echanger(&a->data[i], &a->data[(i - 1) / 2]);
        i = (i - 1) / 2;
    }

    return a;
}






//ABR

struct bst * bstCreateNode(int key) {
    struct bst * l;
    l = (struct bst *)malloc(sizeof(struct bst));
    l->key = key;
    l->left = NULL;
    l->right = NULL;
    return l;
}


void bstInorderTraversal(struct bst * root) {
	printf("( ");
    if (root == NULL)
    {
        return;
    }
    else 
    {
        bstInorderTraversal(root->left);
        printf("%d ",root->key);
        bstInorderTraversal(root->right);
    }
	printf(")\n");
}

struct bst * bstInsert(struct bst * root, int key) {
    if (root == NULL)
    {
        root= bstCreateNode(key);
    }
    else
    {
        if (key > root->key)
        {
            root->right = bstInsert(root->right,key);
        }
        else
        {
            root->left = bstInsert(root->left,key);
        }
    }
    return root;
}

struct bst * bstMinValue(struct bst * root) {
    struct bst * l = root->right;
    if (l->left == NULL)
    {
        root->key = l->key;
    }
    else
    {
        l->left = bstMinValue(l->left);
    }
	return root;
}

struct bst * bstDelete(struct bst * root, int key) {
    if (root==NULL)
    {
        return NULL;
    }
    else
    {
        if (key == root->key)
        {
            if (root->left == NULL && root->right== NULL)
            {
                struct bst * l = root;
                root = NULL;
                free(l);
            }
            else if (root->left == NULL)
            {
                struct bst * l = root;
                root = root -> right;
                free(l);
            }
            else if (root->right == NULL)
            {
                struct bst * l = root;
                root = root -> left;
                free(l);
            }
            else
            {
                root->key=bstMinValue(root->right)->key;
                root->right=bstDelete(root->right,key);
            }
        }
        else if (key < root->key)
        {
            root -> right = bstDelete(root->left,key);
        }
        else if (key > root->key)
        {
            root -> left = bstDelete(root->left,key);
        }

    }
    return root;
    


}

void bstFree(struct bst * root) {
    if (root == NULL)
    {
        return;
    }
    else
    {
        if (root->left == NULL)
        {
            bstFree(root->right);
            free(root);
        }
        else if (root->right == NULL)
        {
            bstFree(root->left);
            free(root);
        }
    }
}

void bstDisplay(struct bst * root) {
    if (root == NULL)
    {
        return;
    }
    else
    {
        
            printf("%d",root->key);
            printf("[");
            bstDisplay(root->left);
            printf("]");
            printf("[");
            bstDisplay(root->right);
            printf("]");
    }
      
}
int main(){
    return 0;
}

/*
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int valeur;
    int priorite; // plus grand = plus prioritaire
} Noeud;

typedef struct {
    Noeud *data;
    int size;
    int capacite;
} tas;

void echanger(Noeud *a, Noeud *b) {
    Noeud tmp = *a;
    *a = *b;
    *b = tmp;
}

tas *creer_tas(int capacite) {
    tas *t = malloc(sizeof(tas));
    t->data = malloc(sizeof(Noeud) * capacite);
    t->size = 0;
    t->capacite = capacite;
    return t;
}

tas *inserer_elmt(tas *a, int valeur, int priorite) {
    if (!a || a->size >= a->capacite) return a;

    int i = a->size++;
    a->data[i].valeur = valeur;
    a->data[i].priorite = priorite;

    while (i > 0 && a->data[i].priorite > a->data[(i - 1) / 2].priorite) {
        echanger(&a->data[i], &a->data[(i - 1) / 2]);
        i = (i - 1) / 2;
    }

    return a;
}
*/