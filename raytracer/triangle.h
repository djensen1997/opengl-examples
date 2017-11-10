/*****************************************************************************
 * Program Name: triangle.h                                                  *
 * Purpose: Contains the definitions for Triangle geometries in the raytracer*
 *          assignment.                                                      *
 *                                                                           *
 * @author Dane Jensen                                                       *
 *****************************************************************************/

#ifndef TRIANGLE
#define TRIANGLE
//
#include <math.h>
#include "vec_util.h"
#include <stdlib.h>
#include <stdio.h>
#include "types.h"

typedef struct triangle {
    float verts[9];
    float norm[3];
    Material* mat;
}Triangle;


void        init_triangle(Triangle* output, float* verts, float* norm, Material* mat);
void        destroy_Triangle(Triangle* triangle);
Ray_Hit*    intersect_triangle(Triangle* triangle, Ray* ray);

#endif
