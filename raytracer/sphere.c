/*****************************************************************************
 * Program Name: Sphere                                                      *
 * Purpose: Contatains the implementation for sphere geometries for the ray  *
 *          tracer.                                                          *
 *                                                                           *
 * @author Dane Jensen                                                       *
 *****************************************************************************/

#include "sphere.h"

/*****************************************************************************
 * Function Name: init_sphere                                                *
 * Purpose: Creates a sphere object and initalizes its values to those       *
 *          provided                                                         *
 * @param   position:   The 3D position of the center of the sphere          *
 * @param   radius:     The radius of the sphere                             *
 * @param   color:      The Color of the sphere                              *
 * @return              The initialized sphere object                        *
 * @author Dane Jensen                                                       *
 *****************************************************************************/
void     init_sphere(Sphere* output, float* position, float radius, Material* mat){
    vec_cpy(output->position, position, 3);
    output->mat = mat;
    output->radius = radius;
}

/*****************************************************************************
 * Function Name: destroy_sphere                                             *
 * Purpose: Destroys the sphere obeject                                      *
 * @author Dane Jensen                                                       *
 *****************************************************************************/
void         destroy_sphere(Sphere* sphere){
    free(sphere);
}

void dumpSphere(Sphere* s){
    printf("Sphere Dump:\n");
    printf("\tPosition: ( %f, %f, %f )\n", s->position[0], s->position[1], s->position[2]);
    printf("\tRadius: %f\n", s->radius);
}

void dumpRay(Ray* s){
    printf("Ray Dump:\n");
    printf("\tPosition: ( %f, %f, %f )\n", s->position[0], s->position[1], s->position[2]);
    printf("\tVector: ( %f, %f, %f )\n", s->vector[0], s->vector[1], s->vector[2]);
}

/*****************************************************************************
 * Function Name: point_in_sphere                                            *
 * Purpose: Tests whether the point given is inside the sphere               *
 * @return  a float array with the first 3 being coordinates, the last one   *
 * telling how to interpret them.                                            *
 *          -1: no intersection                                              *
 *           1: intersected, reflected in this direction                     *
 *           2: interected, not reflected                                    *
 * @author Dane Jensen                                                       *
 *****************************************************************************/
Ray_Hit*        intersect_sphere(Sphere* sphere, Ray* ray){
    //−d⋅(e−c)±√((d⋅(e−c))^2−(d⋅d )((e−c)⋅(e−c)−R^2))/d DOT d
    //d is the vector representing the ray
    //c is the center of the sphere
    //e is the start pos of the ray
    //R is the radius

    //printf("Checking for a sphere intersect\n");
    //dumpSphere(sphere);
    //dumpRay(ray);
    float d_dot_d = vec3f_dot_vec3f(ray->vector, ray->vector);
    float e_min_c[3];
    vec3f_sub_vec3f(e_min_c, ray->position, sphere->position);
    float d_dot_ef = vec3f_dot_vec3f(ray->vector, e_min_c);
    float disc =   (pow(d_dot_ef,2) - d_dot_d * (vec3f_dot_vec3f(e_min_c,e_min_c) - pow(sphere->radius,2)));
    //printf("VALUE DUMP: %f %f %f\n",d_dot_d, d_dot_ef, disc);
    float t = 0;
    if(disc < 0){
        //printf("d < 0\n");
        return NULL;
    }else{
        //printf("Got a hit\n");
        t = (-d_dot_ef + sqrt(disc))/(vec3f_dot_vec3f(ray->vector,ray->vector));
    }
    //printf("Mallocing ray hit\n");
    Ray_Hit* output = (Ray_Hit*)malloc(sizeof(Ray_Hit));
    //printf("Setting t to %f\n", t);
    output->t = t;
    //printf("Setting the Material\n");
    output->mat = sphere->mat;
    float norm[3];
    float loc[3];
    //printf("Calculating the Hit Location\n");
    //printf("Hit Location: ( ");
    for(int i = 0; i < 3; i++){
        loc[i] = ray->position[i] + ray->vector[i] * t;
        //printf("%f, ", loc[i]);
    }
    //printf(")\n");

    //printf("Calculating the norm\n");
    vec3f_sub_vec3f(norm, loc, sphere->position);
    vec3f_normalize(norm, norm);
    
    vec_cpy(output->norm, norm, 3);
    //printf("Normal: (%f, %f, %f)\n",output->norm[0],output->norm[1],output->norm[2]);
    vec_cpy(output->loc, loc,3);
    //printf("Returning a hit\n");
    return output;
}




