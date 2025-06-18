#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#define L 500
#define C 500
typedef struct complx{
    float r;
    float i;
}complx;
typedef struct coord{
    float u;
    float v;

}coord;
complx* translation(float dx, float dy, complx* t ){
    complx*e;
    e=(complx*)malloc(sizeof(complx));
    e->r=t->r+dx;
    e->i=t->i+dy;
    return e;
}

complx* tanslation_inverse(complx*z_inv,float dx ,float dy){
    complx*z;
    z=(complx*)malloc(sizeof(complx));
    z->r=(z_inv->r)- dx;
    z->i=(z_inv->i)- dy;
    return z;
}
coord *coordonnee_image_vers_complexe(i,j){
    
} 
int main (){
    complx*r;
    r=(complx*)malloc(sizeof(complx));
    r->r=1;
    r->i=3;
    complx *z=tanslation_inverse(r,0.1,0.3);
    printf("%f",z->r);
}