/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Runs code for the terrain assignment based on the texture.c file
 * 			provided in the samples folder.
 *
 * @author Dane Jesnen
 * @author Scott Kuhl
 */

#include "libkuhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static GLuint program = 0; /**< id value for the GLSL program */
static kuhl_geometry map;
static char layer1[] = "../images/terrain.png";
static char layer2[] = "../images/color_terrain.png";
static char clouds[] = "../images/clouds.jpg";


void prepTerrain(kuhl_geometry* geom, GLuint prog);

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action != GLFW_PRESS)
		return;
	
	switch(key)
	{
		case GLFW_KEY_Q:
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
	}
}

/** Draws the 3D scene. */
void display()
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

		/* Turn on blending (note, if you are using transparent textures,
		   the transparency may not look correct unless you draw further
		   items before closer items. This program always draws the
		   geometry in the same order.). */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		/* Get the view or camera matrix; update the frustum values if needed. */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);

		//transform from object coodinates to world coordinates (puts it into a 2x2x2 box)
		float modelMatrix[16];
		mat4f_scale_new(modelMatrix, (float)1/1024,(float)1/1024,1);

		/* Create a scale matrix. */
		float scaleMatrix[16];
		mat4f_scale_new(scaleMatrix, 3, 3, 3);

		// Modelview = (viewMatrix * scaleMatrix) * rotationMatrix
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, scaleMatrix, modelMatrix);
		mat4f_mult_mat4f_new(modelview, modelview, viewMat);

		/* Tell OpenGL which GLSL program the subsequent
		 * glUniformMatrix4fv() calls are for. */
		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();
		
		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();
		/* Draw the geometry using the matrices that we sent to the
		 * vertex programs immediately above */
		kuhl_geometry_draw(&map);

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

void prepTerrain(kuhl_geometry* geom, GLuint prog){
	printf("Preping terrain\n");
	//BASIC INIT//
	int num_verticies = 2048 * 1024;
	kuhl_geometry_new(geom, prog, num_verticies, GL_TRIANGLES);
	kuhl_errorcheck();
	printf("Geom made\n");

	//SET THE VERTEX LOCATIONS//
	GLfloat* vertexData = (GLfloat*)malloc(sizeof(GLfloat) * num_verticies * 3);
	printf("vertex data malloced\n");
	int i,j,offset=0;
	for(i = 0; i < 2048; i++){//per col

		for(j = 0; j < 1024; j++){//per row
			
			vertexData[offset++] = i;
			vertexData[offset++] = j;
			vertexData[offset++] = 2;
		}

	}
	printf("Setting Vertex Locations\n");
	kuhl_geometry_attrib(geom, vertexData, 3, "in_Position", KG_WARN);
	kuhl_errorcheck();
	printf("Vertex locations set\n");


	//TELL THE PROGRAM THE TEXTURE COORDS//
	GLfloat *texcoordData = (GLfloat*)malloc(sizeof(GLfloat) * 2048 * 1024 * 2);
	offset = 0;
	for (i = 0; i < 2048; i++){
		for(j = 0; j < 1024; j++){
			texcoordData[offset++] = (((float)i)/((float)2048));
			texcoordData[offset++] = (((float)j)/((float)1024));
		}
	}
	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);
	kuhl_errorcheck();
	printf("Tex coords added");

	//LOAD THE TERRAINS//
	GLuint texId[] = {0,0,0};
	kuhl_read_texture_file(layer1, &(texId[0]));
	kuhl_geometry_texture(geom, texId[0], "base_terrain", KG_WARN);
	printf("Base terrain loaded\n");
	kuhl_errorcheck();
	kuhl_read_texture_file(layer2, &(texId[1]));
	kuhl_geometry_texture(geom, texId[1], "color_terrain", KG_WARN);
	printf("Color terrain loaded\n");
	kuhl_errorcheck();
	kuhl_read_texture_file(clouds, &(texId[2]));
	kuhl_geometry_texture(geom, texId[2], "clouds", KG_WARN);
	printf("clouds loaded\n");
	kuhl_errorcheck();
	

	//INITIALIZE THE TRIANGES//
	int num_triangles = 2047 * 1023 * 6;//the img is going to be 2048 x 1024, so the num triangles can be found like this
	GLuint* indexData = (GLuint*)malloc(sizeof(GLuint) * num_triangles);
	int triangle = 0;
	for(i = 0; i < 1023; i++){
		for(j = 0; j < 2047; j++){
			//triangle 1
			indexData[triangle++] = j 	+ i 	* 2048;
			indexData[triangle++] = j 	+ (i+1) * 2048;
			indexData[triangle++] = j+1 + i 	* 2048;

			//triangle 2
			indexData[triangle++] = j+1 + i 	* 2048;
			indexData[triangle++] = j 	+ (i+1) * 2048;
			indexData[triangle++] = j+1 + (i+1) * 2048;
		}
	}
	kuhl_geometry_indices(geom, indexData, 6);
	kuhl_errorcheck();
	printf("triangles initialized\n");
	free(indexData);
	free(vertexData);
	free(texcoordData);
	printf("Terrain Prepped\n");

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
	program = kuhl_create_program("terrain.vert", "terrain.frag");
	glUseProgram(program);
	kuhl_errorcheck();

	prepTerrain(&map, program);
	
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,0,10}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);
	
	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}

	exit(EXIT_SUCCESS);
}
