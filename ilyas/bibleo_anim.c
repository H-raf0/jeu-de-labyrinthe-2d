#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct cmpl{
    float r;
    float i;
}Complex;

Complex rotation_img(Complex z, Complex z0 , float angle ){
    Complex resultat ;
    float x = z.r;
    float y = z.i;
    float x0 = z0.r;
    float y0 = z0.i;
    float theta = M_PI * angle /180;
    resultat.r = cos(theta)*(x-x0) - sin(theta)*(y-y0) + x0;
    resultat.i = cos(theta)*(x-x0) + sin(theta)*(y-y0) + y0;
    return  resultat;
}
Complex transformation_inverse(Complex z, Complex z0, float Angle_max, int d0, int d1, int d_max){
    float x = z.r;
    float y = z.i;
    float x0 = z0.r;
    float y0 = z0.i;
    float mod_z = ((x-x0)*2*2 + (y-y0)*2*2)*0.5*0.5;
    float theta_max = M_PI*Angle_max/180;
    if (mod_z <= d0){
        return z;
    }
    else if (mod_z <= d_max){
        float angle = - theta_max / (d_max - d0)*(mod_z-d0);
        return rotation_img(z,z0,angle);

    }
    else if (mod_z <= d1){
        float angle = -(theta_max/(d_max-d1)*(mod_z-d_max)+theta_max);
        return rotation_img(z,z0,angle);
    }
    else return z;
 



}  
int main()
{
    return 0;
}