/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing textured geometry by making a carousel
 *
 * @author Dane Jensen
 * @author Scott Kuhl
 */

#include "libkuhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static GLuint program = 0; /**< id value for the GLSL program */
static kuhl_geometry* quads;
static float** t_matrix;
static int num_images;
static float pi = 3.141529;
static float* angles;
static float std_angle;


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

		/* Calculate an angle to rotate the object. glfwGetTime() gets
		 * the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
		//float angle = fmod(glfwGetTime()*45, 360);
		float angle = 0;
		/* Create a scale matrix. */
		float scaleMatrix[16];
		mat4f_scale_new(scaleMatrix, 3, 3, 3);
		
		for (int i = 0; i < num_images; i+=1){
			int rot = (int)angle / (int)std_angle;
			int index = (i + rot)%num_images;
			float rotMat[16];
			float t_angle = angle + angles[index];

			t_angle = fmod(t_angle,360);
			if(t_angle == 0){
				printf(".");
			}
			mat4f_rotateAxis_new(rotMat, t_angle, 0, 1, 0);
			// Modelview = (viewMatrix * scaleMatrix) * rotationMatrix
			float modelview[16];
			mat4f_mult_mat4f_new(modelview, t_matrix[index], scaleMatrix );
			mat4f_mult_mat4f_new(modelview, rotMat,  modelview);
			
			mat4f_mult_mat4f_new(modelview, viewMat, modelview);
			
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
			kuhl_geometry_draw(&quads[index]);

			//glUseProgram(0); // stop using a GLSL program.
		}
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	printf("\n");
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

void init_geometryQuad(kuhl_geometry *geom, GLuint prog, char* filename){
	kuhl_geometry_new(geom, prog, 4, GL_TRIANGLES);
	//printf("Making a quad\n");
	kuhl_errorcheck();
	GLfloat texcoordData[] = {0, 0,
	                          1, 0,
							  1, 1,
							  0, 1 };
	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);
	kuhl_errorcheck();
	// The 2 parameter above means each texture coordinate is a 2D coordinate.
	
	/* Load the texture. It will be bound to texId */	
	//printf("loading a texture: %s\n",filename);
	GLuint texId = 0;
	float ratio = kuhl_read_texture_file(filename, &texId);
	//printf("Quad Aspect Ratio: %f\n", ratio);
	/* The data that we want to draw */
	float angle,x=0;
	if(num_images >= 3){
		angle = (2 * pi) / num_images;
		x = 2*sin(angle/2);
	}else{
		angle = (pi/2);
		x = 2*sin(angle/2);
	}
	

	float xratio,yratio = 1;
	xratio = x;
	yratio = 1/ratio;

	//printf("XRATIO: %.3f, YRATIO: %.3f\n",xratio,yratio);

	GLfloat vertexData[16] = {	-xratio/2, 0, 0,
								xratio/2, 0, 0,
								xratio/2, yratio, 0,
								-xratio/2, yratio, 0};

	kuhl_geometry_attrib(geom, vertexData, 3, "in_Position", KG_WARN);
	kuhl_errorcheck();
	// The 3 parameter above means that each vertex position is a 3D coordinate.

	
	
	/* Tell this piece of geometry to use the texture we just loaded. */
	kuhl_geometry_texture(geom, texId, "tex", KG_WARN);
	kuhl_errorcheck();

	GLuint indexData[] = {
		0,1,2,
		0,3,2
	};
	kuhl_geometry_indices(geom, indexData, 6);

	kuhl_errorcheck();
	//printf("Finished a quad\n");
}

int main(int argc, char** argv){
	/* Initialize GLFW and GLEW */
	
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);
	char** images;
	num_images = 0;
	//char def[] = "../images/kitten.jpg";
	if(argc < 2){
		printf("Usage: ./carousel <img 1> <img 2>...\n");
		exit(0);
	}else{
		images = (char**)malloc(sizeof(char*) * (argc-1));
		printf("%d  ", argc);
		for (int i = 1; i < argc; i++){
			printf("%s  ", argv[i]);
			images[i-1] = argv[i];
		}
		printf("\n");
		num_images = argc-1;
	}
	
	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program("carousel.vert", "carousel.frag");
	
	kuhl_errorcheck();

	t_matrix = (float**)malloc(sizeof(float*) * (num_images));
	for(int i = 0; i < num_images; i++)
		t_matrix[i] = (float*)malloc(sizeof(float)*16);
	printf("Translations Malloced\n");
	quads = (kuhl_geometry*)malloc(sizeof(kuhl_geometry) * (num_images));
	angles = (float*)malloc(sizeof(float) * (num_images));
	printf("Quads Malloced\n");
	std_angle = 360/num_images;

	glUseProgram(program);
	for (int j = 0; j < num_images/2; j+=1){		
		int h = (num_images-1)-j;
		int l = j;
		init_geometryQuad(&quads[2*j], program, images[l]);
		mat4f_translate_new(t_matrix[2*j],0,-3,-4);
		angles[j] = std_angle * l;
		init_geometryQuad(&quads[2*j+1], program, images[h]);
		mat4f_translate_new(t_matrix[2*j+1],0,-3,-4);
		angles[2*j+1] = std_angle * h;

		printf("%d  %d  ",l,h);
	}
	if(num_images%2 == 1){
		int i = num_images/2;
		printf("%d",i);
		init_geometryQuad(&quads[num_images-1], program, images[i]);
		mat4f_translate_new(t_matrix[num_images-1],0,-3,-3);
		angles[num_images-1] = std_angle * i;
	}
	printf("\n");
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);
	printf("Quads made\n");
	
	

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

	free(quads);
	for (int i = 0; i < num_images;i++){
		free(t_matrix[i]);
	}
	free(t_matrix);
	free(images);
	free(angles);

	exit(EXIT_SUCCESS);
}
