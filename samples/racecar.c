/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Is a animated racecar based on a triangle shader written by
 * Dr. Scott Kuhl
 *
 * @author Dane Jensen
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"

static GLuint program = 0; /**< id value for the GLSL program */
static int geometries = 9;
static int moving = 0; //determines whether the wheels are turning
static float m_start_time = 0; //the time when the user started the wheels
static float m_start_angle = 0;
static float w_lst_angle = 0; //the last angle of the wheels
static float turn_angle = 0;
static float turn_start_time = 0;
static float turn_start_angle = 0;
static float turn = 0;
static float t_wheel[4][16];
static float t_cube[5][16];
static float rot = 0;
static float r_start_angle = 0;
static float r_start_time = 0;
static float rot_angle = 0;

void init_geometryCube(
	kuhl_geometry *geom, 
	GLuint prog, 
	float scale);

void init_geometryWheel(
	kuhl_geometry *geom, 
	GLuint prog);

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS){
	
		switch(key)
		{
			case GLFW_KEY_Q:
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GL_TRUE);
				break;
			case GLFW_KEY_SPACE:
				if(moving==0){
					moving=1;
					m_start_time = glfwGetTime();
					m_start_angle = w_lst_angle;
				}
				break;
			case GLFW_KEY_A:
				if(turn == 0){
					turn = 1;
					turn_start_time = glfwGetTime();
					turn_start_angle = turn_angle;
				}
				break;
			case GLFW_KEY_S:
				if(turn == 0){
					turn = -1;
					turn_start_time = glfwGetTime();
					turn_start_angle = turn_angle;
				}
				break;
			case GLFW_KEY_D:
				if(rot == 0){
					rot = 1;
					r_start_time=glfwGetTime();
					r_start_angle=rot_angle;
				}
				break;
			case GLFW_KEY_F:
				if(rot == 0){
					rot = -1;
					r_start_time=glfwGetTime();
					r_start_angle=rot_angle;
				}
		}
	}

	if(action == GLFW_RELEASE){
		switch(key){
			case GLFW_KEY_SPACE:
				if(moving == 1){
					moving = 0;
				}
				break;
			case GLFW_KEY_A:
				if(turn == 1){
					turn = 0;
				}
				break;
			case GLFW_KEY_S:
				if(turn == -1){
					turn = 0;
				}
				break;
			case GLFW_KEY_D:
				if(rot == 1){
					rot = 0;
				}
				break;
			case GLFW_KEY_F:
				if(rot == -1){
					rot = 0;
				}
				break;
		}
	}
}

/** Draws the 3D scene. */
void display(kuhl_geometry* car)
{
	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right). */
	viewmat_begin_frame();
	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		viewmat_begin_eye(viewportID);

		/* Where is the viewport that we are drawing onto and what is its size? */
		int viewport[4]; // x,y of lower left corner, width, height
		viewmat_get_viewport(viewport, viewportID);
		/* Tell OpenGL the area of the window that we will be drawing in. */
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Clear the current viewport. Without glScissor(), glClear()
		 * clears the entire screen. We could call glClear() before
		 * this viewport loop---but in order for all variations of
		 * this code to work (Oculus support, etc), we can only draw
		 * after viewmat_begin_eye(). */
		glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(.2,.2,.2,0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		/* Get the view matrix and the projection matrix */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);

		float w_angle;
		if(moving == 1){
			/* Calculate an angle to rotate the object. glfwGetTime() gets
			* the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
			w_angle = fmod((glfwGetTime()-m_start_time)*60, 360);
			w_angle = fmod(m_start_angle + w_angle,360);
			w_lst_angle = w_angle;
		}else{
			w_angle = w_lst_angle;
		}

		float t_angle;
		if(turn != 0){
			t_angle = fmod((glfwGetTime()-turn_start_time) * 20 * turn,360);
			t_angle = turn_start_angle + t_angle;
			if(t_angle < -45){
				t_angle = -45;
			}else if(t_angle > 45){
				t_angle = 45;
			}
			turn_angle = t_angle;

		}else{
			t_angle = turn_angle;
		}

		float r_angle;
		if(rot != 0){
			r_angle = fmod((glfwGetTime()-r_start_time)*45*rot,360);
			r_angle = fmod(r_start_angle + r_angle,360);
			rot_angle = r_angle;
		}else{
			r_angle = rot_angle;
		}

		/* Make sure all computers/processes use the same angle */
		dgr_setget("angle", &w_angle, sizeof(GLfloat));

		/* Create a 4x4 rotation matrix based on the angle we computed. */
		float rotateMat[16];
		mat4f_rotateAxis_new(rotateMat, w_angle, 0,0,1);

		float turnMat[16];
		mat4f_rotateAxis_new(turnMat,t_angle,0,1,0);

		float rotMat[16];
		mat4f_rotateAxis_new(rotMat,r_angle,0,1,0);

		/* Create a scale matrix. */
		float scaleMat[16];
		mat4f_scale_new(scaleMat, 3, 3, 3);

		//make the model matrices by combining
		//all the transformation matricies
		float model[geometries][16];
		for (int i = 0; i < 5; i++){
			mat4f_mult_mat4f_new(model[i],scaleMat,t_cube[i]);
		}

		//front left wheel
		mat4f_mult_mat4f_new(model[5], scaleMat, rotateMat);
		mat4f_mult_mat4f_new(model[5], turnMat, model[5]);
		mat4f_mult_mat4f_new(model[5], t_wheel[0], model[5]);

		//front right wheel
		mat4f_mult_mat4f_new(model[6], scaleMat, rotateMat);
		mat4f_mult_mat4f_new(model[6], turnMat, model[6]);
		mat4f_mult_mat4f_new(model[6], t_wheel[1], model[6]);

		//rear left wheel
		mat4f_mult_mat4f_new(model[7], scaleMat, rotateMat);
		mat4f_mult_mat4f_new(model[7], t_wheel[2], model[7]);

		//rear right wheel
		mat4f_mult_mat4f_new(model[8], scaleMat, rotateMat);
		mat4f_mult_mat4f_new(model[8], t_wheel[3], model[8]);

		for(int i = 0; i < geometries; i++){
			if(i < 5){
				/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
				the correct program. */
				glUseProgram(program);
				kuhl_errorcheck();
				/* Set the uniform variable in the shader that is named "red" to the value 1. */
				glUniform1i(kuhl_get_uniform("red"),1);
				//glUniform1i(kuhl_get_uniform("green"), 0);
				kuhl_errorcheck();
				/* Good practice: Unbind objects until we really need them. */
				glUseProgram(0);
			}else{
				/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
				the correct program. */
				glUseProgram(program);
				kuhl_errorcheck();
				/* Set the uniform variable in the shader that is named "red" to the value 1. */
				glUniform1i(kuhl_get_uniform("red"),0);
				//glUniform1i(kuhl_get_uniform("green"), 1);
				kuhl_errorcheck();
				/* Good practice: Unbind objects until we really need them. */
				glUseProgram(0);
			}
			mat4f_mult_mat4f_new(model[i],rotMat,model[i]);
			/* Construct a modelview matrix: modelview = viewMat * modelMat */
			float modelview[16];
			mat4f_mult_mat4f_new(modelview, viewMat, model[i]);
			/* Tell OpenGL which GLSL program the subsequent
			 * glUniformMatrix4fv() calls are for. 
			 */
			kuhl_errorcheck();
			glUseProgram(program);
			kuhl_errorcheck();
			/* Send the perspective projection matrix to the vertex program. */
			glUniformMatrix4fv(	kuhl_get_uniform("Projection"),
								1, // number of 4x4 float matrices
								0, // transpose
								perspective); // value
			/* Send the modelview matrix to the vertex program. */
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
								1, // number of 4x4 float matrices
								0, // transpose
								modelview); // value
			/* Generate an appropriate normal matrix based on the model view matrix:
			 * normalMat = transpose(inverse(modelview))
			 */
			float normalMat[9];
			mat3f_from_mat4f(normalMat, modelview);
			mat3f_invert(normalMat);
			mat3f_transpose(normalMat);
			glUniformMatrix3fv(kuhl_get_uniform("NormalMat"),
							1, // number of 3x3 float matrices
							0, // transpose
							normalMat); // value

			kuhl_errorcheck();
			/* Draw the geometry using the matrices that we sent to the
			* vertex programs immediately above */
			//draw the front wheels
			kuhl_geometry_draw(&(car[i]));
		}


		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

void make_car(kuhl_geometry* car, GLuint prog){
	//make the body
	//think 1 = 1m


	printf("MAKING MAIN BLOCK\n");
	init_geometryCube(&(car[0]), prog, 1);//put the main cube in the middle of the car
	mat4f_translate_new(t_cube[0],0,.5,0);
	printf("MAKING HOOD\n");
	init_geometryCube(&(car[1]), prog, .5);//put the main cube in the middle of the car
	mat4f_translate_new(t_cube[1],-.75,.25,.25);
	init_geometryCube(&(car[2]), prog, .5);//put the main cube in the middle of the car
	mat4f_translate_new(t_cube[2],-.75,.25,-.25);
	printf("MAKING TRUNK\n");
	init_geometryCube(&(car[3]), prog, .5);//put the main cube in the middle of the car
	mat4f_translate_new(t_cube[3],.75,.25,.25);
	init_geometryCube(&(car[4]), prog, .5);//put the main cube in the middle of the car
	mat4f_translate_new(t_cube[4],.75,.25,-.25);
	


	
	init_geometryWheel(&(car[5]),prog);//x,z
	init_geometryWheel(&(car[6]),prog);
	init_geometryWheel(&(car[7]),prog);//x,z
	init_geometryWheel(&(car[8]),prog);

	//the wheel translation matricies
	float fw_translate[16];
	mat4f_translate_new(fw_translate,-1.5,0,0);

	float lw_translate[16];
	mat4f_translate_new(lw_translate,0,0,1.75);

	float rw_translate[16];
	mat4f_translate_new(rw_translate,0,0,-1.75);

	//make matrix for front and rear wheels
	float bw_translate[16];
	mat4f_translate_new(bw_translate,1.5,0,0);

	//front left wheel
	mat4f_mult_mat4f_new(t_wheel[0],fw_translate,lw_translate);
	//front right wheel
	mat4f_mult_mat4f_new(t_wheel[1],fw_translate,rw_translate);
	//rear left wheel
	mat4f_mult_mat4f_new(t_wheel[2],bw_translate,lw_translate);
	//rear right wheel
	mat4f_mult_mat4f_new(t_wheel[3],bw_translate,rw_translate);
	
}

void init_geometryCube(
	kuhl_geometry *geom, 
	GLuint prog, 
	float scale){

	printf("MAKING A CUBE\n");
	kuhl_geometry_new(geom,prog,24,GL_TRIANGLES);
	

	GLfloat vertexPositions[] = {

		//front
		-.5,-.5,-.5,//A
		-.5,.5,-.5,//B
		.5,-.5,-.5,//C
		.5,.5,-.5,//D

		//back
		-.5,-.5,.5,//E
		-.5,.5,.5,//F
		.5,-.5,.5,//G
		.5,.5,.5,//H

		//top
		-.5,.5,-.5,//B
		-.5,.5,.5,//F
		.5,.5,-.5,//D
		.5,.5,.5,//H


		//bottom
		-.5,-.5,-.5,//A
		-.5,-.5,.5,//E
		.5,-.5,-.5,//C
		.5,-.5,.5,//G

		//left
		-.5,-.5,-.5,//A
		-.5,-.5,.5,//E
		-.5,.5,-.5,//B
		-.5,.5,.5,//F

		//right
		.5,-.5,-.5,//C
		.5,-.5,.5,//G
		.5,.5,-.5,//D
		.5,.5,.5,//H
	};

	int i = 0;
	for (i=0; i < 72; i+=1){
		vertexPositions[i]*=scale;
	}

	kuhl_geometry_attrib(geom,vertexPositions,3,"in_Position",KG_WARN);
	GLfloat normalData[] = {
		//front
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		//back
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		//top
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		//bottom
		0, -1, 0,
		0, -1, 0,
		0, -1, 0,
		0, -1, 0,
		//left
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,
		//right
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		1, 0, 0
		
	};
	kuhl_geometry_attrib(geom, normalData,3,"in_Normal",KG_WARN);
	/* A list of triangles that we want to draw. "0" refers to the
	 * first vertex in our list of vertices. Every three numbers forms
	 * a single triangle. */
	 GLuint indexData[] = { 
		//front
		0, 1, 3,  
		0, 2, 3,
		//back
		4,5,7,
		4,6,7,
		//top
		8,9,11,
		8,10,11,
		//bottom
		12,13,15,
		12,14,15,
		//left
		16,17,19,
		16,18,19,
		//right
		20,21,23,
		20,22,23
		};
	kuhl_geometry_indices(geom, indexData, 36);
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	

	kuhl_errorcheck();
	printf("A CUBE WAS MADE\n");
}

void init_geometryWheel(kuhl_geometry *geom, GLuint prog){

	printf("MAKING A WHEEL\n");
	//all of the math to determine the vertex positions
	float height = .5;//.25m
	
	float theight = height/2;//triangle height
	float side = 2 * (height/2) * tan(3.14 /8);//the length of one side of the octogon
	printf("Side: %f\n",side);
	float sq3 = side/2;
	float width = 0.15;//.1m
	kuhl_geometry_new(geom,prog,50,GL_TRIANGLES);

	GLfloat vertexPositions[] = {
		//front
		0, 			0, 			-width/2,
		sq3,		theight,	-width/2,
		-sq3,		theight,	-width/2,
		sq3,		-theight,	-width/2,
		-sq3,		-theight,	-width/2,
		theight,	sq3,		-width/2,
		theight ,	-sq3,		-width/2,
		-theight ,	sq3,		-width/2,
		-theight ,	-sq3,		-width/2,

		//back
		0 , 		0, 			width/2,
		sq3 ,		theight,	width/2,
		-sq3 ,		theight,	width/2,
		sq3 ,		-theight,	width/2,
		-sq3 ,		-theight,	width/2,
		theight ,	sq3,		width/2,
		theight ,	-sq3,		width/2,
		-theight ,	sq3,		width/2,
		-theight ,	-sq3,		width/2,

		//top
		sq3,		theight,	-width/2,
		-sq3,		theight,	-width/2,
		sq3 ,		theight,	width/2,
		-sq3 ,		theight,	width/2,

		//bottom
		sq3,		-theight,	-width/2,
		-sq3,		-theight,	-width/2,
		sq3 ,		-theight,	width/2,
		-sq3 ,		-theight,	width/2,

		//left
		-theight ,	sq3,		-width/2,
		-theight ,	-sq3,		-width/2,
		-theight ,	sq3,		width/2,
		-theight ,	-sq3,		width/2,

		//right
		theight,	sq3,		-width/2,
		theight ,	-sq3,		-width/2,
		theight ,	sq3,		width/2,
		theight ,	-sq3,		width/2,

		//upper left
		-sq3,		theight,	-width/2,
		-sq3 ,		theight,	width/2,
		-theight ,	sq3,		-width/2,
		-theight ,	sq3,		width/2,

		//upper right
		sq3,		theight,	-width/2,
		sq3 ,		theight,	width/2,
		theight,	sq3,		-width/2,
		theight ,	sq3,		width/2,

		//lower left
		-sq3,		-theight,	-width/2,
		-sq3 ,		-theight,	width/2,
		-theight ,	-sq3,		-width/2,
		-theight ,	-sq3,		width/2,

		//lower right
		sq3,		-theight,	-width/2,
		sq3 ,		-theight,	width/2,
		theight,	-sq3,		-width/2,
		theight ,	-sq3,		width/2,
	
	};

	kuhl_geometry_attrib(geom,vertexPositions,3,"in_Position",KG_WARN);
	GLfloat normalData[] = {
		//front
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,

		//back
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,

		//top
		0,1,0,
		0,1,0,
		0,1,0,
		0,1,0,

		//bottom
		0,-1,0,
		0,-1,0,
		0,-1,0,
		0,-1,0,

		//left
		-1,0,0,
		-1,0,0,
		-1,0,0,
		-1,0,0,

		//right
		1,0,0,
		1,0,0,
		1,0,0,
		1,0,0,

		//ul
		-sin(2),sin(2),0,
		-sin(2),sin(2),0,
		-sin(2),sin(2),0,
		-sin(2),sin(2),0,

		//ur
		sin(2),sin(2),0,
		sin(2),sin(2),0,
		sin(2),sin(2),0,
		sin(2),sin(2),0,

		//ll
		-sin(2),-sin(2),0,
		-sin(2),-sin(2),0,
		-sin(2),-sin(2),0,
		-sin(2),-sin(2),0,
		
		//lr
		sin(2),-sin(2),0,
		sin(2),-sin(2),0,
		sin(2),-sin(2),0,
		sin(2),-sin(2),0

	};
	kuhl_geometry_attrib(geom, normalData,3,"in_Normal",KG_WARN);
	/* A list of triangles that we want to draw. "0" refers to the
	 * first vertex in our list of vertices. Every three numbers forms
	 * a single triangle. */
	 GLuint indexData[] = { 
		 	//front
			0,1,2,
			0,5,6,
			0,3,4,
			0,8,7,
			0,1,5,
			0,6,3,
			0,4,8,
			0,7,2,
			
			//back
			9,10,11,
			9,14,15,
			9,12,13,
			9,16,17,
			9,10,14,
			9,15,12,
			9,13,17,
			9,16,11,
			
			//top
			18,19,20,
			19,20,21,

			//bottom
			22,23,24,
			23,24,25,

			//left
			26,27,28,
			27,28,29,

			//right
			30,31,32,
			31,32,33,

			//ul
			34,35,36,
			35,36,37,

			//ur
			38,39,40,
			39,40,41,

			//ll
			42,43,44,
			43,44,45,

			//lr
			46,47,48,
			47,48,49

		};
	kuhl_geometry_indices(geom, indexData, 96);
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	

	kuhl_errorcheck();
	printf("A WHEEL WAS MADE\n");
}

int main(int argc, char** argv)
{
	printf("Hello World\n");
	kuhl_geometry* car = (kuhl_geometry*)malloc(sizeof(kuhl_geometry)*geometries);
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program("triangle-shade.vert", "triangle-shade.frag");

	/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
	   the correct program. */
	glUseProgram(program);
	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	glUniform1i(kuhl_get_uniform("red"), 1);
	kuhl_errorcheck();
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	/* Create kuhl_geometry structs for the objects that we want to
	 * draw. */
	printf("PRE MAKE CAR\n");
	make_car(car, program);
	printf("POST MAKE CAR\n");

	

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,0,10}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);
	
	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display(car);
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}

	free(car);

	exit(EXIT_SUCCESS);
}
