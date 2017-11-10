/*****************************************************************************
 * Program Name: Vec_util.c                                                  *
 * Purpose: Contatains the implementation for vectors for the ray tracer     *
 *          assignment                                                       *
 *                                                                           *
 * @author Dane Jensen                                                       *
 *****************************************************************************/

#include <math.h>
#include "vec_util.h"

/*****************************************************************************
 * Function Name: vec_cpy                                                    *
 * Purpose: Copies the contents of one vector to another vector              *
 * @author Dane Jensen                                                       *
 *****************************************************************************/
int vec_cpy(float* dest, float* src, int size){
    for(int i = 0; i < size; i++){
        dest[i] = src[i];
    }
    return 0;
}

float vec4f_dot_vec4f(float* v1, float* v2){
    float output = 0;
    for(int i = 0 ; i < 4; i++){
        output += v1[i] * v2[i];
    }
    return output;
}

void vec4f_normalize(float* dest, float* src){
    float length = sqrt(vec4f_dot_vec4f(src,src));
    for(int i = 0; i < 4; i++){
        dest[i] = src[i] / length;
    }
}

void vec4f_mult_mat4f(float* dest, float* mat, float* vec){
    for(int i = 0; i < 4; i++){
        float v1[4] = {mat[i*4], mat[i*4+1], mat[i*4+2], mat[i*4+3]};
        dest[i] = vec4f_dot_vec4f(v1, vec);
    }
    vec4f_normalize(dest,dest);
} 

int vec3f_sub_vec3f(float* dest, float* v1, float* v2){
    for(int i = 0; i < 3; i++){
        dest[i] = v1[i] - v2[i];
    }
    return 0;
}

float vec3f_dot_vec3f(float* v1, float* v2){
    float out = 0;
    for(int i = 0; i < 3; i++){
        out += v1[i] * v2[i];
        //printf("vec3f_dot_intermitant: %f\n", out);
    }
    //printf("vec3f_dot_result: %f\n", out);
    return out;
}

void vec3f_normalize(float* dest, float* src){
    float length = sqrt(vec3f_dot_vec3f(src,src));
    for(int i = 0; i < 3; i++){
        dest[i] = src[i] / length;
    }
}

int vec_cmp(float* v1, float* v2, int size){
    for(int i = 0; i < size; i++){
        if(v1[i] != v2[i]){
            return 0;
        }
    }
    return 1;
}



