/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing a colored 3D triangle.
 *
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"

static GLuint program = 0;
static GLuint road_prog = 0; /**< id value for the GLSL program */

/*
 *
 * A collection of quads that form a building
 * 
 */
typedef struct building{
	float* modelMat;
	kuhl_geometry* quads;
	float width;
	float height;
}Building;

/*
 *	A set of 4 buildings
 */

typedef struct block{
	Building** buildings;
	float* modelMat;
	kuhl_geometry road;
	float road_color;
}Block;
Building* generateSmallBuilding(GLuint prog, float x, float y, float z);
Block* generateBlock(GLuint prog, float x, float y, float z);
void drawBuilding(Building* build);
void drawBlock(Block* object, float* viewMat, float* perspective);
void destroyBlock(Block* object);
void updateViewer();

typedef struct viewer{
	float start_pos[3];
	float start_look[3];
	float curr_pos[3];
	float curr_look[3];
	float translate[3];
	float yangle;
	float xangle;
}Viewer;

//static float min_height = 1.0;
//static float max_height = 10.0;
static float building_width = 5.0;
static Block* wall;
static int max_blocks = 1;//should be a square number (i.e 1,4,9,16,25...)
static Viewer you;
static int dirs[4];
static float last_frame;
static float curr_frame;

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS){
		switch(key)
		{
			//takes a normalized look vector and multiplies it by a unit of time (only on the x,z plane)
			//adds result to the position to get the new position
			case GLFW_KEY_W:
				dirs[0] = 1;
				break;
			//takes a normalized look vector and multiplies it by a negative unit of time (only on the x,z plane)
			//adds the result to the position to get the new position
			case GLFW_KEY_S:
				dirs[0] = -1;
				break;
			//changes the x-angle based on time elapsed (rotate left)
			//ONLY CHANGES THE LOOK REFRENCE POINT
			case GLFW_KEY_A:
				dirs[1] = 1;
				break;
			//changes the x-angle based on time elasped (rotate right)
			//ONLY CHANGES THE LOOK REFRENCE POINT
			case GLFW_KEY_D:
				dirs[1] = -1;
				break;
			//changes the y-angle based on time elasped (rotate up)
			//ONLY CHANGES THE LOOK REFRENCE POINT
			case GLFW_KEY_UP:
				dirs[2] = 1;
				break;
			//changes the y-angle based on time elasped (rotate down)
			//ONLY CHANGES THE LOOK REFRENCE POINT
			case GLFW_KEY_DOWN:
				dirs[2] = -1;
				break;

			case GLFW_KEY_SPACE:
				dirs[3] = 1;
				break;

			case GLFW_KEY_Z:
				dirs[3] = -1;
				break;

			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GL_TRUE);
				break;
		}
	}

	if(action == GLFW_RELEASE){
		switch(key)
		{
			//takes a normalized look vector and multiplies it by a unit of time (only on the x,z plane)
			//adds result to the position to get the new position
			case GLFW_KEY_W:
				if(dirs[0] == 1){
					dirs[0] = 0;
				}
				break;
			//takes a normalized look vector and multiplies it by a negative unit of time (only on the x,z plane)
			//adds the result to the position to get the new position
			case GLFW_KEY_S:
				if(dirs[0] == -1){
					dirs[0] = 0;
				}
				break;
			//changes the x-angle based on time elapsed (rotate left)
			//ONLY CHANGES THE LOOK REFRENCE POINT
			case GLFW_KEY_A:
				if(dirs[1] == 1){
					dirs[1] = 0;
				}
				break;
			//changes the x-angle based on time elasped (rotate right)
			//ONLY CHANGES THE LOOK REFRENCE POINT
			case GLFW_KEY_D:
				if(dirs[1] == -1){
					dirs[1] = 0;
				}
				break;
			//changes the y-angle based on time elasped (rotate up)
			//ONLY CHANGES THE LOOK REFRENCE POINT
			case GLFW_KEY_UP:
				if(dirs[2] == 1){
					dirs[2] = 0;
				}
				break;
			//changes the y-angle based on time elasped (rotate down)
			//ONLY CHANGES THE LOOK REFRENCE POINT
			case GLFW_KEY_DOWN:
				if(dirs[2] == -1){
					dirs[2] = 0;
				}
				break;

			case GLFW_KEY_SPACE:
				if(dirs[3] == 1){
					dirs[3] = 0;
				}
				break;

			case GLFW_KEY_Z:
				if(dirs[3] == -1){
					dirs[3] = 0;
				}
				break;

		}
	}
}

/** Draws the 3D scene. */
void display()
{
	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right). */
	updateViewer();
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
		mat4f_lookat_new(
			viewMat, 
			you.curr_pos[0], you.curr_pos[1], you.curr_pos[2],
			you.curr_look[0], you.curr_look[1], you.curr_look[2], 
			 
			0, 1, 0);
		mat4f_perspective_new(perspective, 90, 1, .1, 100);



		//BUILD A WALL
		

		/* Tell OpenGL which GLSL program the subsequent
		 * glUniformMatrix4fv() calls are for. */
		kuhl_errorcheck();
		/* Draw the geometry using the matrices that we sent to the
		 * vertex programs immediately above */
		for(int i = 0; i < max_blocks; i++){
			drawBlock(wall, viewMat, perspective);
		}
		
		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

/**
 * 
 * 	Updates the viewers location
 * 
 **/
void updateViewer(){
	curr_frame = glfwGetTime();
	float time_diff = curr_frame - last_frame;
	float m_speed = 4;//moves 4 units per second
	float ym_speed = 8;//moves up/down at 8 units per second
	float y_speed = 45; //rotates the look point up 45 degrees per second
	float x_speed = 180;	//rotates the look point left or right 45 degrees per second
	you.xangle = fmod(time_diff * x_speed * dirs[1] + you.xangle, 360);
	you.yangle = fmod(time_diff * y_speed * dirs[2] + you.yangle, 360);
	float look_model[16], yrotate[16], xrotate[16];
	mat4f_rotateAxis_new(yrotate,you.yangle, 1, 0, 0);
	mat4f_rotateAxis_new(xrotate,you.xangle, 0, 1, 0);

	//calculate the new angle
	mat4f_mult_mat4f_new(look_model, yrotate, xrotate);
	//holds the rotated look point
	float temp[4];
	temp[0] = you.start_look[0];
	temp[1] = you.start_look[1];
	temp[2] = you.start_look[2];
	temp[3] = 1;
	mat4f_mult_vec4f_new(temp, look_model, temp);

	float look_vec[3], scalar;
	//look_point - camera_pos = look_vec
	for(int i = 0; i < 3; i++){
		look_vec[i] = temp[i] - you.start_pos[i];
	}
	//for an x-z tranlastion, we don't need the y compnent of the look vector
	look_vec[1] = 0;
	//noramlize the look vector
	vec3f_normalize_new(look_vec, look_vec);
	scalar = m_speed * time_diff * dirs[0];

	//direction * velosity = distance traveled
	for(int i = 0; i < 3; i++){
		you.translate[i] += scalar * look_vec[i];
	}
	you.translate[1] += ym_speed * time_diff * dirs[3];
	if(you.translate[1] < 0){
		you.translate[1] = 0;
	}

	if(you.translate[1] > 50){
		you.translate[1] = 50;
	}

	float translation[16];//translation matrix for the camera and the look point
	mat4f_translate_new(translation, you.translate[0],you.translate[1],you.translate[2]);
	//move the look camera point
	mat4f_mult_vec4f_new(temp, translation, temp);
	//copy the transformed camera look point to the viewer struct
	for(int i = 0; i < 3; i++){
		you.curr_look[i] = temp[i];
	}

	/****move the camera*****/

	//copy the start pos to temp
	for(int i = 0; i < 3; i++){
		temp[i] = you.start_pos[i];
	}
	mat4f_mult_mat4f_new(temp, translation, temp);
	//copy the transformed point back to the viewer
	for(int i = 0; i < 3; i++){
		you.curr_pos[i] = temp[i];
	}
	last_frame = curr_frame;
}

void drawBuilding(Building* build){
	int i = 0;
	int iter = 1 + (int)build->width * (int)build->height * 4;
	for(i = 0; i < iter; i++){
		//sets color ether green or black
		if(i == 0){
			//sets color to grey
			float color = 0.0;
			glUniform1f(
				kuhl_get_uniform("color"),	//tells it what attribute to set
				color						//tells it what to set the attribute
			);
		}else{
			float color = (float)(i % 3);
			if(color != 2.0){
				color = 1.0;
			}
			glUniform1f(
				kuhl_get_uniform("color"),	//tells it what attribute to set
				color						//tells it what to set the attribute
			);
		}
		kuhl_geometry_draw(&(build->quads[i]));
	}
}

void drawBlock(Block* object, float* viewMat, float* perspective){
	//draw buildings
	glUseProgram(program);
	for(int i = 0; i < 4; i++){
		float modelview[16];
		mat4f_mult_mat4f_new(modelview,viewMat,object->buildings[i]->modelMat);
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		    1, // number of 4x4 float matrices
		    0, // transpose
			modelview); // value
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1,
			0,
			perspective);
		kuhl_errorcheck();
		drawBuilding(object->buildings[i]);
	}

	glUseProgram(0);
	//draw road
	glUseProgram(road_prog);
	float modelview[16];
	mat4f_mult_mat4f_new(modelview,viewMat,object->modelMat);
	/* Send the modelview matrix to the vertex program. */
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
	    1, // number of 4x4 float matrices
	    0, // transpose
		modelview); // value
	glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1,
			0,
			perspective);
	kuhl_errorcheck();
	kuhl_geometry_draw(&(object->road));
	glUseProgram(0);
}

/*	
 *	@param prog the program that puts the points on the screen
 *	@param x	the world x coord
 * 	@param y	the world y coord
 * 	@param z 	the world z coord
 * 	@return a build building (with one wall having windows)
 */
Building* generateSmallBuilding(GLuint prog, float x, float y, float z){
	Building* output = (Building*)malloc(sizeof(Building));
	output->width = building_width;
	output->height = 10;
	
	//the 1 is the base building, the width and height detirmine the number
	//of windows on the building and there are 4 sides of the building that
	//need windows
	output->quads = (kuhl_geometry*)malloc(sizeof(kuhl_geometry) * (int)(1 + building_width * output->height * 4));
	//create quads centered on the origin

	//create the base cube
	{
		kuhl_geometry* geom = &(output->quads[0]);
		//modified code from the racecar assignment
		kuhl_geometry_new(&(output->quads[0]),prog,24,GL_TRIANGLES);

		float height = output->height;

		//sets the verticies for a rectangular prism with a square base and
		//a variable height
		//centered on the origin
		GLfloat vertexPositions[] = {

			//front
			
			-building_width/2,-height/2,-building_width/2,//A
			-building_width/2,height/2,-building_width/2,//B
			building_width/2,-height/2,-building_width/2,//C
			building_width/2,height/2,-building_width/2,//D
			
			//back
			-building_width/2,-height/2,building_width/2,//E
			-building_width/2,height/2,building_width/2,//F
			building_width/2,-height/2,building_width/2,//G
			building_width/2,height/2,building_width/2,//H
			
			//top
			-building_width/2,height/2,-building_width/2,//B
			-building_width/2,height/2,building_width/2,//F
			building_width/2,height/2,-building_width/2,//D
			building_width/2,height/2,building_width/2,//H
			

			//bottom
			-building_width/2,-height/2,-building_width/2,//A
			-building_width/2,-height/2,building_width/2,//E
			building_width/2,-height/2,-building_width/2,//C
			building_width/2,-height/2,building_width/2,//G

			//left
			-building_width/2,-height/2,-building_width/2,//A
			-building_width/2,-height/2,building_width/2,//E
			-building_width/2,height/2,-building_width/2,//B
			-building_width/2,height/2,building_width/2,//F

			//right
			building_width/2,-height/2,-building_width/2,//C
			building_width/2,-height/2,building_width/2,//G
			building_width/2,height/2,-building_width/2,//D
			building_width/2,height/2,building_width/2,//H
			
		};

		//sets the normals for the cube
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
			0, 1, 2,  
			1, 2, 3,
			
			//back
			4,5,6,
			5,6,7,
			
			//top
			8,9,10,
			9,10,11,

			//bottom
			12,13,15,
			12,14,15,

			//left
			16,17,18,
			17,18,19,

			//right
			20,21,23,
			20,22,23

			};
		kuhl_geometry_indices(geom, indexData, 36);
		/* Set the uniform variable in the shader that is named "red" to the value 1. */
		

		kuhl_errorcheck();
	}
	
	//puts windows on the back of the building
	//set the windows
	int num_windows = output->width * output->height;
	for(int i = 0; i < num_windows; i++){
		//modified code from the racecar assignment
		kuhl_geometry_new(&(output->quads[i+1]),prog,4,GL_TRIANGLES);
		float height = output->height;
		int j = i % num_windows;

		//sets the verticies for a rectangular prism with a square base and
		//a variable height
		//centered on the origin
		GLfloat vertexPositions[] = {
			//front
			-building_width/2 + 1.0 * (i%((int)building_width)) + .03,//x
			-height/2 + j/((int)building_width) + .03,//y
			-building_width/2 - .01,//z,A

			-building_width/2 + 1.0 * (i%((int)building_width)) + .03,
			-height/2 + j/((int)building_width) + .97,
			-building_width/2 - .01,//B

			-building_width/2 + 1.0 * (i%((int)building_width)) + .97,
			-height/2 + j/((int)building_width) + .03,
			-building_width/2 - .01,//C

			-building_width/2 + 1.0 * (i%((int)building_width)) + .97,
			-height/2 + j/((int)building_width) + .97,
			-building_width/2 - .01//D
		};

		//sets the normals for the cube
		kuhl_geometry_attrib(&(output->quads[i+1]),vertexPositions,3,"in_Position",KG_WARN);
		GLfloat normalData[] = {
			//front
			0, 0, -1,
			0, 0, -1,
			0, 0, -1,
			0, 0, -1,
		};

		kuhl_geometry_attrib(&(output->quads[i+1]), normalData,3,"in_Normal",KG_WARN);
		GLuint indexData[] = { 
			//front
			0, 1, 3,  
			0, 2, 3,
			};
		kuhl_geometry_indices(&(output->quads[i+1]), indexData, 6);
		

		kuhl_errorcheck();
	}
	//puts windows on the front of the building
	//set the windows
	for(int i = num_windows; i < num_windows*2; i++){
		//modified code from the racecar assignment
		kuhl_geometry_new(&(output->quads[i+1]),prog,4,GL_TRIANGLES);
		float height = output->height;
		int j = i % num_windows;
		//sets the verticies for a rectangular prism with a square base and
		//a variable height
		//centered on the origin
		GLfloat vertexPositions[] = {
			//front
			-building_width/2 + 1.0 * (i%((int)building_width)) + .03,//x
			-height/2 + j/((int)building_width) + .03,//y
			building_width/2 + .01,//z,A

			-building_width/2 + 1.0 * (i%((int)building_width)) + .03,
			-height/2 + j/((int)building_width) + .97,
			building_width/2 + .01,//B

			-building_width/2 + 1.0 * (i%((int)building_width)) + .97,
			-height/2 + j/((int)building_width) + .03,
			building_width/2 + .01,//C

			-building_width/2 + 1.0 * (i%((int)building_width)) + .97,
			-height/2 + j/((int)building_width) + .97,
			building_width/2 + .01//D
		};

		//sets the normals for the cube
		kuhl_geometry_attrib(&(output->quads[i+1]),vertexPositions,3,"in_Position",KG_WARN);
		GLfloat normalData[] = {
			//front
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
		};

		kuhl_geometry_attrib(&(output->quads[i+1]), normalData,3,"in_Normal",KG_WARN);
		GLuint indexData[] = { 
			//front
			0, 1, 3,  
			0, 2, 3,
			};
		kuhl_geometry_indices(&(output->quads[i+1]), indexData, 6);
		

		kuhl_errorcheck();
	}
	//left side of the building
	//set the windows
	for(int i = num_windows*2; i < num_windows*3; i++){
		//modified code from the racecar assignment
		kuhl_geometry_new(&(output->quads[i+1]),prog,4,GL_TRIANGLES);
		float height = output->height;
		int j = i % num_windows;

		//sets the verticies for a rectangular prism with a square base and
		//a variable height
		//centered on the origin
		GLfloat vertexPositions[] = {
			//front
			-building_width/2 - .01,//x
			-height/2 + j/((int)building_width) + .03,//y
			-building_width/2 + 1.0 * (i%((int)building_width)) + .03,//z,A

			-building_width/2 - .01,//x
			-height/2 + j/((int)building_width) + .97,//y
			-building_width/2 + 1.0 * (i%((int)building_width)) + .03,//z,B

			-building_width/2 - .01,//x
			-height/2 + j/((int)building_width) + .03,//y
			-building_width/2 + 1.0 * (i%((int)building_width)) + .97,//z,C

			-building_width/2 - .01,//x
			-height/2 + j/((int)building_width) + .97,//y
			-building_width/2 + 1.0 * (i%((int)building_width)) + .97//z,D
		};

		//sets the normals for the cube
		kuhl_geometry_attrib(&(output->quads[i+1]),vertexPositions,3,"in_Position",KG_WARN);
		GLfloat normalData[] = {
			//front
			-1, 0, 0,
			-1, 0, 0,
			-1, 0, 0,
			-1, 0, 0,
		};

		kuhl_geometry_attrib(&(output->quads[i+1]), normalData,3,"in_Normal",KG_WARN);
		GLuint indexData[] = { 
			//front
			0, 1, 3,  
			0, 2, 3,
			};
		kuhl_geometry_indices(&(output->quads[i+1]), indexData, 6);
		

		kuhl_errorcheck();
	}

	//set the windows
	for(int i = num_windows*3; i < num_windows*4; i++){
		//modified code from the racecar assignment
		kuhl_geometry_new(&(output->quads[i+1]),prog,4,GL_TRIANGLES);

		float height = output->height;
		int j = i % num_windows;
		//sets the verticies for a rectangular prism with a square base and
		//a variable height
		//centered on the origin
		GLfloat vertexPositions[] = {
			//front
			building_width/2 + .01,//x
			-height/2 + j/((int)building_width) + .03,//y
			-building_width/2 + 1.0 * (i%((int)building_width)) + .03,//z,A

			building_width/2 + .01,//x
			-height/2 + j/((int)building_width) + .97,//y
			-building_width/2 + 1.0 * (i%((int)building_width)) + .03,//z,B

			building_width/2 + .01,//x
			-height/2 + j/((int)building_width) + .03,//y
			-building_width/2 + 1.0 * (i%((int)building_width)) + .97,//z,C

			building_width/2 + .01,//x
			-height/2 + j/((int)building_width) + .97,//y
			-building_width/2 + 1.0 * (i%((int)building_width)) + .97//z,D
		};

		//sets the normals for the cube
		kuhl_geometry_attrib(&(output->quads[i+1]),vertexPositions,3,"in_Position",KG_WARN);
		GLfloat normalData[] = {
			//front
			1, 0, 0,
			1, 0, 0,
			1, 0, 0,
			1, 0, 0,
		};

		kuhl_geometry_attrib(&(output->quads[i+1]), normalData,3,"in_Normal",KG_WARN);
		GLuint indexData[] = { 
			//front
			0, 1, 3,  
			0, 2, 3,
			};
		kuhl_geometry_indices(&(output->quads[i+1]), indexData, 6);
		

		kuhl_errorcheck();
	}

	output->modelMat = (float*)malloc(sizeof(float) * 16);
	mat4f_translate_new(output->modelMat,x,y + 1,z);
	return output;
}

Block* generateBlock(GLuint prog, float x, float y, float z){
	int type = 0;//will detirmine the contents of the block (range 0-15)
	Block* output = malloc(sizeof(Block));
	float road_width = building_width / 2;
	output->buildings = (Building**)malloc(sizeof(Building*)*4);
	glUseProgram(0);
	glUseProgram(road_prog);
	//code to make the road

	{
		//modified code from the racecar assignment
		kuhl_geometry_new(&(output->road),road_prog,4,GL_TRIANGLES);
		float height = 1;
		//sets the verticies for a rectangular prism with a square base and
		//a variable height
		//centered on the origin
		GLfloat vertexPositions[] = {
			-building_width * 1.5,
			height,
			-building_width * 1.5,

			building_width * 1.5,
			height,
			-building_width * 1.5,

			-building_width * 1.5,
			height,
			building_width * 1.5,

			building_width * 1.5,
			height,
			building_width * 1.5
		};

		//sets the normals for the cube
		kuhl_geometry_attrib(&(output->road),vertexPositions,3,"in_Position",KG_WARN);

		GLfloat texcoordData[] = {
				0, 0,
	            1, 0,
				0, 1,
	        	1, 1 };
		kuhl_geometry_attrib(&(output->road), texcoordData, 2, "in_TexCoord", KG_WARN);

		GLfloat normalData[] = {
			//front
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		kuhl_geometry_attrib(&(output->road), normalData,3,"in_Normal",KG_WARN);
		GLuint indexData[] = { 
			//front
			0, 1, 3,  
			0, 2, 3,
			};
		kuhl_geometry_indices(&(output->road), indexData, 6);
		
		/* Load the texture. It will be bound to texId */	
		GLuint texId = 0;
		kuhl_read_texture_file("../images/road.png", &texId);
		/* Tell this piece of geometry to use the texture we just loaded. */
		kuhl_geometry_texture(&(output->road), texId, "tex", KG_WARN);

		kuhl_errorcheck();
	}
	glUseProgram(prog);

	//code to make the buildings
	float x_shift = building_width * .75;
	float z_shift = building_width * .75;
	float y_shift = 5;
	//building on the far left
	if(type % 2 == 0){
		//generate simple building
		output->buildings[0] = generateSmallBuilding(prog, -x_shift, y_shift, -z_shift);
	}else{
		//generate complex building
	}

	//far right
	if(type % 4 == 0){
		//generate simple building
		output->buildings[1] = generateSmallBuilding(prog, x_shift, y_shift, -z_shift);
	}else{
		//generate complex building
	}

	//close left
	if(type % 8 == 0){
		//generate simple building
		output->buildings[2] = generateSmallBuilding(prog, -x_shift, y_shift, z_shift);
	}else{
		//generate complex building
	}

	//close right
	if(type%16 == 0){
		//generate simple building
		output->buildings[3] = generateSmallBuilding(prog, x_shift, y_shift, z_shift);
	}else{
		//generate complex building
	}

	output->modelMat = (float*)malloc(sizeof(float) * 16);

	//x,y,z are the world coordinates for the center of the city block
	mat4f_translate_new(output->modelMat,x,y,z);
	for(int i = 0; i < 4; i++){
		//make the modelMat for each building their position in world coordinates
		mat4f_mult_mat4f_new(output->buildings[i]->modelMat, output->modelMat, output->buildings[i]->modelMat);
	}

	return output;
}

void destroyBlock(Block* object){
	free(object->modelMat);
	for(int i = 0 ; i < 4; i++){
		free(object->buildings[i]->modelMat);
		free(object->buildings[i]->quads);
		free(object->buildings[i]);
	}
	free(object);
}

int main(int argc, char** argv)
{
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program("infinicity.vert", "infinicity.frag");
	road_prog = kuhl_create_program("infinicity-road.vert", "infinicity-road.frag");
	/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
	   the correct program. */
	glUseProgram(program);
	kuhl_errorcheck();

	wall = generateBlock(program, 0,-2,5);
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	dgr_init();     /* Initialize DGR based on environment variables. */

	you.yangle = 0;
	you.xangle = 0;
	you.start_pos[0] = 0;
	you.start_pos[1] = 0;
	you.start_pos[2] = 0;
	you.start_look[0] = 0;
	you.start_look[1] = 0;
	you.start_look[2] = 10;
	
	for(int i = 0; i<4; i++)
		dirs[i] = 0;
	
	float initCamPos[3]  = {0,0,10}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	last_frame = glfwGetTime();
	viewmat_init(initCamPos, initCamLook, initCamUp);
	
	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	destroyBlock(wall);
	exit(EXIT_SUCCESS);
}
