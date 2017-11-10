

#ifndef VECTOR

#define VECTOR

#include <stdio.h>
#include <stdlib.h>

void vec4f_mult_mat4f(float* dest, float* mat, float* vec);
void vec4f_normalize(float* dest, float* src);
void vec3f_normalize(float* dest, float* src);
float vec4f_dot_vec4f(float* v1, float* v2);
int vec_cpy(float* dest, float* src, int size);
float vec3f_dot_vec3f(float* v1, float* v2);
int vec3f_sub_vec3f(float* dest, float* v1, float* v2);
int vec_cmp(float* v1, float* v2, int size);

#endif
