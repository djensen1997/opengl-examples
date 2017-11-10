/*****************************************************************************
 * Program Name: Sphere.h                                                    *
 * Purpose: Contains the definitions for sphere geometries in the raytracer  *
 *          assignment.                                                      *
 *                                                                           *
 * @author Dane Jensen                                                       *
 *****************************************************************************/

#ifndef SPHERE
#define SPHERE
//
#include <math.h>
#include "vec_util.h"
#include <stdlib.h>
#include <stdio.h>
#include "types.h"

typedef struct sphere {
    float position[3];
    float radius;
    Material* mat;
}Sphere;


void        init_sphere(Sphere* output, float* position, float radius, Material* mat);
void        destroy_sphere(Sphere* sphere);
Ray_Hit*    intersect_sphere(Sphere* sphere, Ray* ray);

#endif
