

#ifndef TYPES

#define TYPES

#define REFLECTIVE 1
#define DIFFUSE 0

typedef struct ray{
    float vector[3];
    float position[3];
}Ray;

typedef struct material{
    char color[3];
    int reflective;
}Material;

typedef struct ray_hit{
    float t;
    float norm[3];
    Material* mat;
    float loc[3];
}Ray_Hit;



#endif
