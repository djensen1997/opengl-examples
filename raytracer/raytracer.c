/*****************************************************************************
 * Program Name: Raytracer                                                   *
 * Purpose:                                                                  *
 *                                                                           *
 *                                                                           *
 * @author Dane Jensen                                                       *
 *****************************************************************************/

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "sphere.h"
#include "types.h"
#include "triangle.h"
#include "vec_util.h"
#include <limits.h>
#define DEBUG



typedef struct perspective{
    float camera_pos[3];
    float dist_to_screen;
    float screen_width;
    float world_width;
    float screen_height;
    float world_height;
}Perspective;

typedef struct scene{
    Sphere* spheres;
    int num_spheres;
    Triangle* triangles;
    int num_triangles;
}Scene;


static float light_loc[] = {1,1,-3};
static Perspective* scene;
static Material* materials[10];
static int ray_recurse = 0;




/*****************************************************************************
 * Function Name: shootRay                                                   *
 * Purpose: Shoots a ray from the camera's location to the specified pixel   *
 * @author Dane Jensen                                                       *
 *****************************************************************************/
Ray* getRay(Perspective* p, float* from, float* to){
    //printf("Pixel Pos: ( %f, %f, %f )\n", pixel[0], pixel[1], pixel[2]);
    float vec[3];
    for(int i = 0; i < 3; i++){
        vec[i] = to[i] - from[i];
    }
    Ray* output = (Ray*)malloc(sizeof(Ray));
    vec_cpy(output->vector, vec, 3);
    vec_cpy(output->position, to, 3);
    
    return output;
}

void prepTestScene(Scene* scene){
    scene->spheres = (Sphere*)malloc(sizeof(Sphere) * 3);
    scene->num_spheres = 3;
    scene->triangles = (Triangle*)malloc(sizeof(Triangle) * 4);
    scene->num_triangles = 4;


    materials[0] = (Material*)malloc(sizeof(Material));
    unsigned char blue[] = {10,10,255};
    for(int i= 0; i < 3; i++)
        materials[0]->color[i] = blue[i];
    materials[0]->reflective = DIFFUSE;



    materials[1] = (Material*)malloc(sizeof(Material));
    unsigned char red[] = {255,10,10};
    for(int i= 0; i < 3; i++)
        materials[1]->color[i] = red[i];
    materials[1]->reflective = DIFFUSE;

    materials[2] = (Material*)malloc(sizeof(Material));
    materials[2]->reflective = REFLECTIVE;
    unsigned char grey[] = {100,100,100 };
    for(int i = 0 ; i < 3; i++)
        materials[2]->color[i] = grey[i];


    materials[3] = (Material*)malloc(sizeof(Material));
    unsigned char green[] = {10,125,10};
    for(int i= 0; i < 3; i++)
        materials[3]->color[i] = green[i];
    materials[3]->reflective = DIFFUSE;

    materials[4] = (Material*)malloc(sizeof(Material));
    for(int i= 0; i < 3; i++)
        materials[4]->color[i] = grey[i];
    materials[4]->reflective = DIFFUSE;

    float pos[3];
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = -2.3;
    init_sphere(&(scene->spheres[0]), pos, .1, materials[0]);

    pos[0] = -.4;
    pos[1] = .1;
    pos[2] = -3;
    init_sphere(&(scene->spheres[1]), pos, .3, materials[1]);

    pos[0] = .2;
    pos[1] = -.1;
    pos[2] = -3.7;
    init_sphere(&(scene->spheres[2]), pos, .1, materials[2]);

    float verts[9] = {
        -1, -1, -10,
        1,  1,  -10,
        -1, 1,  -10
    };
    float norm1[3] = {
        0,0,1
    };
    init_triangle(&(scene->triangles[0]), verts, norm1, materials[3]);
    float verts2[9] = {
        -1, -1, -10,
        1,  1,  -10,
        1,  -1, -10
    };
    init_triangle(&(scene->triangles[1]), verts2, norm1, materials[3]);



    float verts3[9] = {
        -1, -1, -10,
        -1, -1,  -2,
        1,  -1, -10
    };
    float norm[3] = {
        0,1,0
    };
    init_triangle(&(scene->triangles[2]), verts3, norm, materials[4]);
    float verts4[9] = {
        1, -1, -10,
        -1, -1, -2,
        1,  -1, 2
    };
    init_triangle(&(scene->triangles[3]), verts4, norm, materials[4]);
}


void destroyScene(Scene* scene){
    free(scene->spheres);
    free(scene->triangles);
    free(scene);
}



unsigned char* getRayHit(Ray* ray, Scene* scene){
    //printf("Getting a Ray Hit\n");
    //printf("(%f, %f)\n", ray->position[0], ray->position[1]);
    Material* out = NULL;
    float loc[3];
    float norm[3];
    unsigned char* color = malloc(sizeof(char) * 3);
    float t = INT_MAX;

    //test spheres
    for(int i = 0; i < scene->num_spheres; i++){
        Ray_Hit* hit = intersect_sphere(&(scene->spheres[i]), ray);
        if(hit == NULL || hit->t <= .01){
            //printf("No Hit\n");
            continue;
        }
        if(hit->t < t){
            //printf("There is a hit on sphere %d\n", i);
            t = hit->t;
            out = hit->mat;
            vec_cpy(loc, hit->loc,3);
            vec_cpy(norm, hit->norm,3);
        }
        //free(hit);
    }

    //test triangles
    for(int i = 0; i < scene->num_triangles; i++){
        Ray_Hit* hit = intersect_triangle(&(scene->triangles[i]), ray);
        if(hit == NULL || hit->t <= 0.01){
            //printf("No Hit\n");
            continue;
        }
        //printf("Hit Location: %p\n", hit);
        //dump_Ray_Hit(hit);
        if(hit->t < t){
            if(i == 1){
                //printf("There is a hit on triangle %d\n", i);
                //dump_Ray_Hit(hit);
            }
            t = hit->t;
            out = hit->mat;
            vec_cpy(loc, hit->loc,3);
            vec_cpy(norm, hit->norm,3);
        }
        //free(hit);
    }

    if(t != INT_MAX){
        if(out->reflective == REFLECTIVE){
            printf("Ray is reflected\n");
            ray_recurse++;
            if(ray_recurse == 10){
                ray_recurse = 0;
                color[0] = 50;
                color[1] = 50;
                color[2] = 50;
            }else{
                //r = d - 2(d dot n) n
                Ray* new = (Ray*) malloc(sizeof(Ray));
                new->position[0] = loc[0];
                new->position[1] = loc[1];
                new->position[2] = loc[2];
                float temp = 2 * vec3f_dot_vec3f(ray->vector, norm);
                float temp2[3];
                for(int i = 0; i < 3; i++){
                    temp2[i] = norm[i] * temp;
                }

                vec3f_sub_vec3f(new->vector, ray->vector, temp2);

                //dumpRay(new);
                color = getRayHit(new, scene);
                /*printf("Color Dump: \n");
                printf("\t R %d, G %d, B %d\n",color[0],color[1],color[2]);*/
                //free(new);
            }
            
        }else{
            //printf("got a ray hit\n\tColor is: (");
            for(int i = 0; i < 3; i++){
                color[i] = out->color[i];
                //printf("%d ", color[i]);
            }
            //printf(")\n");

            float light_dir[3];
            vec3f_sub_vec3f(light_dir, light_loc, loc);
            vec3f_normalize(light_dir,light_dir);
            vec3f_normalize(norm,norm);
            //printf("\tLight Dir: (%f, %f, %f)\n",light_dir[0],light_dir[1],light_dir[2]);
            float diffuse = vec3f_dot_vec3f(light_dir, norm)/2 + .5;
            if(diffuse < .4){
                diffuse = 0.4;
            }

            Ray test_ray;
            vec_cpy(test_ray.position, loc,3);
            vec_cpy(test_ray.vector, light_dir,3);

            //test spheres
            for(int i = 0; i < scene->num_spheres; i++){
                Ray_Hit* hit = intersect_sphere(&(scene->spheres[i]), &test_ray);
                if(hit == NULL || hit->t < 0){
                    //printf("No Hit\n");
                    continue;
                }else if(hit->t <= 0.01){
                    //dump_Ray_Hit(hit);
                    //printf ("PANIC!!!!!\n");
                    continue;
                }else{
                    //diffuse = .3;
                    break;
                }
                free(hit);
            }


            //test triangles
            for(int i = 0; i < scene->num_triangles; i++){
                Ray_Hit* hit = intersect_triangle(&(scene->triangles[i]), &test_ray);
                if(hit == NULL || hit->t < 0){
                    //printf("No Hit\n");
                    continue;
                }else if(hit->t <= 0.01){
                    //dump_Ray_Hit(hit);
                    //exit(1);
                    continue;
                }else{
                    //diffuse = .3;
                    break;
                }
                free(hit);
            }


            //printf("\tDiffuse: %f\n", diffuse);
            for(int i = 0; i < 3; i++){
                color[i] = (unsigned char)(color[i] * diffuse);
            }
        }
    }else{
        color[0] = 20;
        color[1] = 20;
        color[2] = 20;
    }

    return color;
}


/*Scene* genDKScene(){
    // make a material which is reflective
    material refl;
    refl.reflective = 1;
    refl.color = vec3(0,0,0); // color is not used when material is reflective!
    // make several diffuse materials to choose from
    material red;
    red.reflective = 0;
    red.color = vec3(1,0,0);
    material blue;
    blue.reflective = 0;
    blue.color = vec3(0,0,1);
    material white;
    white.reflective = 0;
    white.color = vec3(1,1,1);
    // create three spheres
    sphere sph1;
    sph1.pos = vec3(0,0,-16);
    sph1.radius = 2;
    sph1.mat = refl;
    sphere sph2;
    sph2.pos = vec3(3,-1,-14);
    sph2.radius = 1;
    sph2.mat = refl;
    sphere sph3;
    sph3.pos = vec3(-3,-1,-14);
    sph3.radius = 1;
    sph3.mat = red;
    // back wall
    triangle back1;
    triangle_new(vec3(-8,-2,-20),
    vec3(8,-2,-20),
    vec3(8,10,-20), &back1);
    back1.mat = blue;
    triangle back2;
    triangle_new(vec3(-8,-2,-20),
    vec3(8,10,-20),
    vec3(-8,10,-20), &back2);
    back2.mat = blue;
    // floor
    triangle bot1;
    triangle_new(vec3(-8,-2,-20),
    vec3(8,-2,-10),vec3(8,-2,-20), &bot1);
    bot1.mat = white;
    triangle bot2;
    triangle_new(vec3(-8,-2,-20),
    vec3(-8,-2,-10),
    vec3(8,-2,-10), &bot2);
    bot2.mat = white;
    // right red triangle
    triangle right;
    triangle_new(vec3(8,-2,-20),
    vec3(8,-2,-10),
    vec3(8,10,-20), &right);
    right.mat = red;
}*/


void finalize(){
    
}



int main(int argc, char** argv){
    #ifdef DEBUG
    {
        unsigned char img[100][100][3];
        for(int i = 0; i < 100; i++){
            for(int j = 0; j < 100; j++){
                for(int a = 0; a < 3; a++){
                    img[i][j][a] = 0;
                }
            }
        }
        stbi_write_png("test.png", 100, 100, 3, img, 100*3);
    }
    #endif
    //initialize scene
    printf("Making Sceen\n");
    scene = (Perspective*)malloc(sizeof(Perspective));
    scene->screen_width = 1080;
    scene->screen_height = 1080;
    scene->dist_to_screen = 2;
    float camera_pos[3] = {0,0,0};
    vec_cpy(scene->camera_pos, camera_pos, 3);
    unsigned char img[(int)scene->screen_width][(int)scene->screen_height][3];
    Scene* test_scene = (Scene*)malloc(sizeof(Scene));
    prepTestScene(test_scene);
    printf("Sceen Made\n");

    //shoot rays
    printf("Shooting Rays\n");
    for(int i = 0; i < scene->screen_width; i++){
        for(int j = 0; j < scene->screen_height; j++){
            float pixel[] = {
                ((float)j - scene->screen_height/2)/scene->screen_height/2,

                -((float)i - scene->screen_width/2)/scene->screen_width/2, 
                
                -scene->dist_to_screen};
            Ray* curr = getRay(scene, scene->camera_pos, pixel);
            unsigned char* color = getRayHit(curr, test_scene);
            img[i][j][0] = color[0];
            img[i][j][1] = color[1];
            img[i][j][2] = color[2];
            //free(color);
        }
    }
    printf("Outputting Test Scene\n");
    stbi_write_png(
        "Test_Scene.png", 
        scene->screen_width, 
        scene->screen_height, 
        3, 
        img, 
        scene->screen_width * 3);
    destroyScene(test_scene);
    for(int i = 0 ; i < 2; i++){
        free(materials[i]);
    }
    return 0;
}