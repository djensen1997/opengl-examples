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
    float light_loc[3];
}Scene;


static Perspective* scene;
static Material* materials[10];
static int ray_recurse = 0;




/*****************************************************************************
 * Function Name: shootRay                                                   *
 * Purpose: Shoots a ray from the camera's location to the specified pixel   *
 * @author Dane Jensen                                                       *
 *****************************************************************************/
Ray* getRay(Perspective* p, float* from, float* to){
    //printf("Pixel Pos: ( %f, %f, %f )\n", to[0], to[1], to[2]);
    float vec[3];
    for(int i = 0; i < 3; i++){
        vec[i] = to[i] - from[i];
    }

    Ray* output = (Ray*)malloc(sizeof(Ray));
    vec_cpy(output->vector, vec, 3);
    vec3f_normalize(output->vector, output->vector);
    vec_cpy(output->position, from, 3);
    
    return output;
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
    int s = -1, tri = -1;

    //test spheres
    for(int i = 0; i < scene->num_spheres; i++){
        Ray_Hit* hit = intersect_sphere(&(scene->spheres[i]), ray);
        if(hit == NULL){
            //printf("No Hit\n");
            continue;
        }
        if(hit->t < t){
            //printf("There is a hit on sphere %d\n", i);
            t = hit->t;
            out = hit->mat;
            vec_cpy(loc, hit->loc,3);
            vec_cpy(norm, hit->norm,3);
            if(ray->vector[0] == 0 && ray->vector[1] == 0 && ray->vector[2] == -1){
                printf("NORM:\t");
                for(int i=0; i < 3; i++){
                    printf("%f\t",norm[i]);
                }
                printf("\n");
                printf("LOC:\t");
                for(int i=0; i < 3; i++){
                    printf("%f\t",loc[i]);
                }
                printf("\n");
            }

            s = i;
            tri = -1;
        }
        free(hit);
    }

    //test triangles
    for(int i = 0; i < scene->num_triangles; i++){
        Ray_Hit* hit = intersect_triangle(&(scene->triangles[i]), ray);
        if(hit == NULL){
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
            tri = i;
            s = -1;
        }
        free(hit);
    }

    if(t != INT_MAX){
        if(out->reflective == REFLECTIVE){
            //printf("Ray is reflected\n");
            ray_recurse++;
            if(ray_recurse == 10){
                ray_recurse = 0;
                color[0] = 20;
                color[1] = 20;
                color[2] = 20;
            }else{
                //r = d - 2(d dot n) n
                Ray* new = (Ray*) malloc(sizeof(Ray));
                vec_cpy(new->position, loc, 3);

                //calculate the reflction vector
                float temp = 2 * vec3f_dot_vec3f(ray->vector, norm);
                float temp2[3];
                for(int i = 0; i < 3; i++){
                    temp2[i] = norm[i] * temp;
                }
                 

                vec3f_sub_vec3f(new->vector, ray->vector, temp2);
                vec3f_normalize(new->vector, new->vector);
                if(ray->vector[0] == 0 && ray->vector[1] == 0 && ray->vector[2] == -1){
                    printf("REFLECTED:\t");
                    for(int i = 0; i < 3; i++){
                        printf("%f\t", new->vector[i]);
                    }
                    printf("\n");
                }
                //vec_cpy(new->vector, norm,3);
                //dumpRay(new);
                color = getRayHit(new, scene);
                ray_recurse--;
                /*printf("Reflected ray Color: ");
                printf("Color Dump: \n");
                printf("\t R %d, G %d, B %d\n",color[0],color[1],color[2]);*/
                free(new);
            }
            
        }else{
            //printf("got a ray hit\n\tColor is: (");
            for(int i = 0; i < 3; i++){
                color[i] = out->color[i];
                //printf("%d ", color[i]);
            }
            //printf(")\n");

            float light_dir[3];
            vec3f_sub_vec3f(light_dir, scene->light_loc, loc);
            vec3f_normalize(light_dir,light_dir);
            vec3f_normalize(norm,norm);
            //printf("\tLight Dir: (%f, %f, %f)\n",light_dir[0],light_dir[1],light_dir[2]);
            float diffuse = vec3f_dot_vec3f(light_dir, norm)/2 + .5;
            if(diffuse < .3){
                diffuse = 0.3;
            }

            Ray* test_ray = (Ray*)malloc(sizeof(Ray));
            vec_cpy(test_ray->position, loc,3);
            vec_cpy(test_ray->vector, light_dir,3);

            //test spheres
            for(int i = 0; i < scene->num_spheres; i++){
                if(i == s){
                    continue;
                }
                Ray_Hit* hit = intersect_sphere(&(scene->spheres[i]), test_ray);
                if(hit == NULL){
                    //printf("No Hit\n");
                    continue;
                }else{
                    diffuse = .2;
                    break;
                }
                free(hit);
            }


            //test triangles
            for(int i = 0; i < scene->num_triangles; i++){
                if(i == tri){
                    continue;
                }
                Ray_Hit* hit = intersect_triangle(&(scene->triangles[i]), test_ray);
                if(hit == NULL){
                    //printf("No Hit\n");
                    continue;
                }else{
                    diffuse = .2;
                    break;
                }
                free(hit);
            }

            free(test_ray);
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
    ray_recurse = 0;
    return color;
}

void prepTestScene(Scene* scene){
    scene->num_spheres = 3;
    scene->num_triangles = 4;
    scene->spheres = (Sphere*)malloc(sizeof(Sphere) * scene->num_spheres);
    scene->triangles = (Triangle*)malloc(sizeof(Triangle) * scene->num_triangles);

    scene->light_loc[0] = 0.00;
    scene->light_loc[1] = 1;
    scene->light_loc[2] = -2.1;

    float pos[3];
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = -3.3;
    init_sphere(&(scene->spheres[0]), pos, .3, materials[0]);

    pos[0] = -2;
    pos[1] = .1;
    pos[2] = -7;
    init_sphere(&(scene->spheres[1]), pos, 1, materials[1]);

    pos[0] = .5;
    pos[1] = -.5;
    pos[2] = -5.7;
    init_sphere(&(scene->spheres[2]), pos, .3, materials[2]);

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
        1,  -1, -2
    };
    init_triangle(&(scene->triangles[3]), verts4, norm, materials[4]);


    /*
    float verts5[9] = {
        -1, -1,  -2,
        1,  -1, -2,
        -1, 1,  -2
    };
    float norm2[3] = {
        0,0,-1
    };
    init_triangle(&(scene->triangles[4]), verts5, norm2, materials[5]);
    float verts6[9] = {
        1,  -1, -2,
        1,  1,  -2,
        -1, 1,  -2

    };
    init_triangle(&(scene->triangles[5]), verts6, norm2, materials[5]);*/
}

void prepDKScene(Scene* scene){
    scene->num_spheres = 3;
    scene->num_triangles = 5;
    scene->spheres = (Sphere*)malloc(sizeof(Sphere) * scene->num_spheres);
    scene->triangles = (Triangle*)malloc(sizeof(Triangle) * scene->num_triangles);
    
    scene->light_loc[0] = 3;
    scene->light_loc[1] = 5;
    scene->light_loc[2] = -15;
    // make a material which is reflective
    // color is not used when material is reflective!
    // make several diffuse materials to choose from

    // create three spheres

    float scale = 1;
    
    float pos1[3];
    pos1[0] = 0;
    pos1[1] = 0;
    pos1[2] = -16 * scale;
    init_sphere(&(scene->spheres[0]), pos1, 2, materials[2]);

    pos1[0] = 3;
    pos1[1] = -1;
    pos1[2] = -14 * scale;
    init_sphere(&(scene->spheres[1]), pos1, 1, materials[2]);
    
    pos1[0] = -3;
    pos1[1] = -1;
    pos1[2] = -14 * scale;
    init_sphere(&(scene->spheres[2]), pos1, 1, materials[0]);
    
    // back wall
    float verts1[9] = {
        -8,-2,-20 * scale,
        8,-2,-20 * scale,
        8,10,-20 * scale
    };
    float norm1[3] = {
        0,0,1
    };
    init_triangle(&(scene->triangles[0]), verts1, norm1, materials[1]);
    float verts2[9] = {
        -8,-2,-20 * scale,
        8,10,-20 * scale,
        -8,10,-20 * scale

    };
    init_triangle(&(scene->triangles[1]), verts2, norm1, materials[1]);

    // floor
    float verts3[9] = {
        -8,-2,-20 * scale,
        8,-2,-10 * scale,
        8,-2,-20 * scale
    };
    float norm2[3] = {
        0,1,0
    };
    init_triangle(&(scene->triangles[2]), verts3, norm2, materials[6]);
    float verts4[9] = {
        -8,-2,-20 * scale,
        -8,-2,-10 * scale,
        8,-2,-10 * scale

    };
    init_triangle(&(scene->triangles[3]), verts4, norm2, materials[6]);



    // right red triangle
    float verts5[9] = {
        8,10,-20 * scale,
        8,-2,-10 * scale,
        8,-2,-20 * scale
    };
    float norm3[3] = {
        -1,0,0
    };
    init_triangle(&(scene->triangles[4]), verts5, norm3, materials[0]);
    
}


void finalize(){
    
}

void prepMaterials(){
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
        materials[2]->color[i] = 0;


    materials[3] = (Material*)malloc(sizeof(Material));
    unsigned char green[] = {10,125,10};
    for(int i= 0; i < 3; i++)
        materials[3]->color[i] = green[i];
    materials[3]->reflective = DIFFUSE;

    materials[4] = (Material*)malloc(sizeof(Material));
    for(int i= 0; i < 3; i++)
        materials[4]->color[i] = grey[i];
    materials[4]->reflective = DIFFUSE;


    materials[5] = (Material*)malloc(sizeof(Material));
    unsigned char pink[] = {244,66,241};
    for(int i= 0; i < 3; i++)
        materials[5]->color[i] = pink[i];
    materials[5]->reflective = DIFFUSE;

    materials[6] = (Material*)malloc(sizeof(Material));
    unsigned char white[] = {255,255,255};
    for(int i= 0; i < 3; i++)
        materials[6]->color[i] = white[i];
    materials[6]->reflective = DIFFUSE;
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
    scene->screen_width = 512;
    scene->screen_height = 512;
    scene->dist_to_screen = 2;
    float camera_pos[3] = {0,0,0};
    vec_cpy(scene->camera_pos, camera_pos, 3);
    unsigned char c_img[(int)scene->screen_width][(int)scene->screen_height][3];
    unsigned char d_img[(int)scene->screen_width][(int)scene->screen_height][3];
    Scene* test_scene = (Scene*)malloc(sizeof(Scene));
    
    Scene* dk_scene = (Scene*)malloc(sizeof(Scene));
    prepMaterials();
    prepTestScene(test_scene);
    prepDKScene(dk_scene);
    
    
    printf("Sceen Made\n");

    //shoot rays
    printf("Shooting Rays\n");
    for(int i = 0; i < scene->screen_width; i++){
        for(int j = 0; j < scene->screen_height; j++){
            {
                float pixel[] = {
                    ((float)j - scene->screen_height/2)/(scene->screen_height/2),

                    -((float)i - scene->screen_width/2)/(scene->screen_width/2), 
                    
                    -scene->dist_to_screen};
                Ray* curr = getRay(scene, scene->camera_pos, pixel);
                //dumpRay(curr);
                //exit(0);
                unsigned char* color = getRayHit(curr, test_scene);
                c_img[i][j][0] = color[0];
                c_img[i][j][1] = color[1];
                c_img[i][j][2] = color[2];
                free(color);
                free(curr);
            }
            {
                float pixel[] = {
                    ((float)j - scene->screen_height/2)/(scene->screen_height/2),

                    -((float)i - scene->screen_width/2)/(scene->screen_width/2), 
                    
                    -scene->dist_to_screen};
                Ray* curr = getRay(scene, scene->camera_pos, pixel);
                unsigned char* color = getRayHit(curr, dk_scene);
                d_img[i][j][0] = color[0];
                d_img[i][j][1] = color[1];
                d_img[i][j][2] = color[2];
                free(color);
                free(curr);
            }
        }
    }
    printf("Outputting Test Scene\n");
    stbi_write_png(
        "custom.png", 
        scene->screen_width, 
        scene->screen_height, 
        3, 
        c_img, 
        scene->screen_width * 3);


    stbi_write_png(
        "reference.png", 
        scene->screen_width, 
        scene->screen_height, 
        3, 
        d_img, 
        scene->screen_width * 3);
    destroyScene(test_scene);
    destroyScene(dk_scene);
    for(int i = 0 ; i < 2; i++){
        free(materials[i]);
    }
    return 0;
}
