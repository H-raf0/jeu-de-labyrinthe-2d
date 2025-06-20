#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "TAS_FILE_ABR_H"
#define MAX_SIZE 100

typedef struct {
    int data[MAX_SIZE];
    int size;
} tas;


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


