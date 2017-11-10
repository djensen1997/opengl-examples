/*****************************************************************************
 * Program Name: Triangle                                                    *
 * Purpose: Contatains the implementation for Triangle geometries for the ray*
 *          tracer.                                                          *
 *                                                                           *
 * @author Dane Jensen                                                       *
 *****************************************************************************/

#include "triangle.h"

/*****************************************************************************
 * Function Name: init_Triangle                                              *
 * Purpose: Creates a Triangle object and initalizes its values to those     *
 *          provided                                                         *
 * @param   verst       The 3x3 matraix of triangle points                   *
 * @param   radius:     The radius of the Triangle                           *
 * @param   color:      The Color of the Triangle                            *
 * @return              The initialized Triangle object                      *
 * @author Dane Jensen                                                       *
 *****************************************************************************/
void     init_triangle(Triangle* output, float* verts, float* norm, Material* mat){
    
    vec_cpy(output->norm, norm, 3);
    vec_cpy(output->verts, verts, 9);
    /*for (int i = 0; i < 9; i++){
        if(i %3 == 0 && i > 0){
            printf("\n");
        }
        printf(" %f ", verts[i]);
        
    }
    printf("\n\n");*/
    output->mat = mat;
}


void dump_Ray_Hit(Ray_Hit* r){
    printf("Ray_Hit DUMP:\n");
    printf("\tt: %f\n", r->t);
    printf("\tloc: (");
    for(int i = 0; i < 3; i++){
        printf(" %f ", r->loc[i]);
    }
    printf(")\n");
    printf("\tnorm: (");
    for(int i = 0; i < 3; i++){
        printf(" %f ", r->norm[i]);
    }
    printf(")\n");
}

/*****************************************************************************
 * Function Name: point_in_Triangle                                          *
 * Purpose: Tests whether the point given is inside the Triangle             *
 * @author Dane Jensen                                                       *
 *****************************************************************************/
Ray_Hit*      intersect_triangle(Triangle* triangle, Ray* ray){
    Ray_Hit* output = (Ray_Hit*)malloc(sizeof(Ray_Hit));
    //calculate t through a series of complicated equasions copied from Dr. Kuhl's slides
    //0-2 are vert a
    //3-5 are vert b
    //6-8 are vert c
    float a = triangle->verts[0] - triangle->verts[3];
    float b = triangle->verts[1] - triangle->verts[4];
    float c = triangle->verts[2] - triangle->verts[5];
    float d = triangle->verts[0] - triangle->verts[6];
    float e = triangle->verts[1] - triangle->verts[7];
    float f = triangle->verts[2] - triangle->verts[8];
    float g = ray->vector[0];
    float h = ray->vector[1];
    float i = ray->vector[2];
    float j = triangle->verts[0] - ray->position[0];
    float k = triangle->verts[1] - ray->position[1];
    float l = triangle->verts[2] - ray->position[2];
    float m = a * (e * i - h * f) + b * (g * f - d * i)+ c * (d * h - e * g);
    float beta = (j * (e * i - h * f) + k * (g * f - d * i) + l * (d * h - e * g))/m;
    float gamma = (i * (a * k - j * b) + h * (j * c - a * l) + g * (b * l - k * c))/m;
    float t = (-(f * (a * k - j * b) + e * (j * c - a * l) + d * (b * l - k * c)))/m;

    //printf("m: %f, gamma: %f, beta: %f, t: %f\n",m,gamma,beta,t);

    if(t <= 0 || gamma < 0 || gamma > 1 || beta < 0 || beta > 1- gamma){
        free(output);
        //printf("NOT ON TRIANGLE\n");
        return NULL;
    }


    output->t = t;
    output->mat = triangle->mat;
    vec_cpy(output->norm, triangle->norm, 3);
    for(int i = 0; i < 3; i++){
        output->loc[i] = ray->position[i] + ray->vector[i] * t;
    }
    //dump_Ray_Hit(output);
    //printf("Hit Locaiton: %p\n", output);
    return output;
}  


void        destroy_Triangle(Triangle* triangle){
    free(triangle);
}



