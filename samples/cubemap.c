/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Loads 3D model files displays them.
 *
 * @author Scott Kuhl
 */

#include "libkuhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static GLuint program = 0; /**< id value for the GLSL program */
static GLuint texProg = 0;

static kuhl_geometry *fpsgeom = NULL;
static int num_tex = 2;
static kuhl_geometry *modelgeom = NULL;
static kuhl_geometry *origingeom = NULL;
static float texModel[2][16];
static kuhl_geometry texQuads[2];
static float bbox[6];
static GLuint frame_buf[6]; //one for each direction
static GLuint frame_texid[6];
static int saved = 0;
static char *frame_buf_name[6] = {
	"NegXTex",
	"PosXTex",
	"NegYTex",
	"PosYTex",
	"NegZTex",
	"PosZTex"};
static float look_pos[] = {
	-.4,0,0,
	.4,0,0,
	0,-.4,0,
	0,.4,0,
	0,0,-.4,
	0,0,.4
};

static int fitToView = 0; // was --fit option used?

/** The following variable toggles the display an "origin+axis" marker
 * which draws a small box at the origin and draws lines of length 1
 * on each axis. Depending on which matrices are applied to the
 * marker, the marker will be in object, world, etc coordinates. */
static int showOrigin = 0; // was --origin option used?

/** Initial position of the camera. 1.55 is a good approximate
 * eyeheight in meters.*/
static const float initCamPos[3] = {0, 0, 0};

/** A point that the camera should initially be looking at. If
 * fitToView is set, this will also be the position that model will be
 * translated to. */
static const float initCamLook[3] = {0.0f, 0.0f, -5.0f};

/** A vector indicating which direction is up. */
static const float initCamUp[3] = {0.0f, 1.0f, 0.0f};
void drawDuck(float *modelMat, float *viewMat, float *perspective);
void drawQuad(kuhl_geometry *quad, float *modelview, float *perspective);

#define GLSL_VERT_FILE "cubemap.vert"
#define GLSL_FRAG_FILE "cubemap.frag"

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	switch (key)
	{
	case GLFW_KEY_Q:
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_R:
	{
		// Reload GLSL program from disk
		kuhl_delete_program(program);
		program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);
		/* Apply the program to the model geometry */
		kuhl_geometry_program(modelgeom, program, KG_FULL_LIST);
		/* and the fps label*/
		kuhl_geometry_program(fpsgeom, program, KG_FULL_LIST);

		break;
	}
	case GLFW_KEY_W:
	{
		// Toggle between wireframe and solid
		int polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
		if (polygonMode == GL_LINE)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	}
	case GLFW_KEY_P:
	{
		// Toggle between points and solid
		int polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
		if (polygonMode == GL_POINT)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	}
	case GLFW_KEY_C:
	{
		// Toggle front, back, and no culling
		int cullMode;
		glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
		if (glIsEnabled(GL_CULL_FACE))
		{
			if (cullMode == GL_FRONT)
			{
				glCullFace(GL_BACK);
				printf("Culling: Culling back faces; drawing front faces\n");
			}
			else
			{
				glDisable(GL_CULL_FACE);
				printf("Culling: No culling; drawing all faces.\n");
			}
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			printf("Culling: Culling front faces; drawing back faces\n");
		}
		kuhl_errorcheck();
		break;
	}
	case GLFW_KEY_D: // toggle depth clamping
	{
		if (glIsEnabled(GL_DEPTH_CLAMP))
		{
			printf("Depth clamping disabled\n");
			glDisable(GL_DEPTH_CLAMP); // default
			glDepthFunc(GL_LESS);	  // default
		}
		else
		{
			/* With depth clamping, vertices beyond the near and
				   far planes are clamped to the near and far
				   planes. Since multiple layers of vertices will be
				   clamped to the same depth value, depth testing
				   beyond the near and far planes won't work. */
			printf("Depth clamping enabled\n");
			glEnable(GL_DEPTH_CLAMP);
			glDepthFunc(GL_LEQUAL); // makes far clamping work.
		}
		break;
	}
	case GLFW_KEY_EQUAL:  // The = and + key on most keyboards
	case GLFW_KEY_KP_ADD: // increase size of points and width of lines
	{
		// How can we distinguish between '=' and '+'? The 'mods'
		// variable should contain GLFW_MOD_SHIFT if the shift key
		// is pressed along with the '=' key. However, we accept
		// both versions.

		GLfloat currentPtSize;
		GLfloat sizeRange[2] = {-1.0f, -1.0f};
		glGetFloatv(GL_POINT_SIZE, &currentPtSize);
		glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, sizeRange);
		GLfloat temp = currentPtSize + 1;
		if (temp > sizeRange[1])
			temp = sizeRange[1];
		glPointSize(temp);
		printf("Point size is %f (can be between %f and %f)\n", temp, sizeRange[0], sizeRange[1]);
		kuhl_errorcheck();

		// The only line width guaranteed to be available is
		// 1. Larger sizes will be available if your OpenGL
		// implementation or graphics card supports it.
		GLfloat currentLineWidth;
		GLfloat widthRange[2] = {-1.0f, -1.0f};
		glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
		glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, widthRange);
		temp = currentLineWidth + 1;
		if (temp > widthRange[1])
			temp = widthRange[1];
		glLineWidth(temp);
		printf("Line width is %f (can be between %f and %f)\n", temp, widthRange[0], widthRange[1]);
		kuhl_errorcheck();
		break;
	}
	case GLFW_KEY_MINUS: // decrease size of points and width of lines
	case GLFW_KEY_KP_SUBTRACT:
	{
		GLfloat currentPtSize;
		GLfloat sizeRange[2] = {-1.0f, -1.0f};
		glGetFloatv(GL_POINT_SIZE, &currentPtSize);
		glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, sizeRange);
		GLfloat temp = currentPtSize - 1;
		if (temp < sizeRange[0])
			temp = sizeRange[0];
		glPointSize(temp);
		printf("Point size is %f (can be between %f and %f)\n", temp, sizeRange[0], sizeRange[1]);
		kuhl_errorcheck();

		GLfloat currentLineWidth;
		GLfloat widthRange[2] = {-1.0f, -1.0f};
		glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
		glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, widthRange);
		temp = currentLineWidth - 1;
		if (temp < widthRange[0])
			temp = widthRange[0];
		glLineWidth(temp);
		printf("Line width is %f (can be between %f and %f)\n", temp, widthRange[0], widthRange[1]);
		kuhl_errorcheck();
		break;
	}
	// Toggle different sections of the GLSL fragment shader
	case GLFW_KEY_SPACE:
	case GLFW_KEY_PERIOD:
		break;
	}
}

/** Gets a model matrix which is appropriate for the model that we have loaded. */
void get_model_matrix(float result[16])
{
	mat4f_identity(result);

	if (fitToView == 1) // if --fit option was provided.
	{
		float fitMat[16];
		float transMat[16];

		/* Get a matrix to scale+translate the model based on the bounding
		 * box. If the last parameter is 1, the bounding box will sit on
		 * the XZ plane. If it is set to 0, the bounding box will be
		 * centered at the specified point. */
		kuhl_bbox_fit(fitMat, bbox, 1);

		/* Translate the model to the point the camera is looking at. */
		mat4f_translateVec_new(transMat, initCamLook);

		mat4f_mult_mat4f_new(result, transMat, fitMat);
		return;
	}
}

/** Draws the 3D scene. */
void display()
{
	/* Display FPS if we are a DGR master OR if we are running without DGR. */
	if (dgr_is_master())
	{
		static long lasttime = 0;
		long now = kuhl_milliseconds();
		if (now - lasttime > 200) // reduce number to increase frequency of FPS label updates.
		{
			lasttime = now;

			float fps = bufferswap_fps(); // get current fps
			char message[1024];
			snprintf(message, 1024, "FPS: %0.2f", fps); // make a string with fps in it
			float labelColor[3] = {1.0f, 1.0f, 1.0f};
			float labelBg[4] = {0.0f, 0.0f, 0.0f, .3f};

			// If DGR is being used, only display FPS info if we are
			// the master process.
			fpsgeom = kuhl_label_geom(fpsgeom, program, NULL, message, labelColor, labelBg, 24);
		}
	}

	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right). */
	viewmat_begin_frame();
	for (int viewportID = 0; viewportID < viewmat_num_viewports(); viewportID++)
	{
		viewmat_begin_eye(viewportID);

		// Save viewport
		int origViewport[4];
		glGetIntegerv(GL_VIEWPORT, origViewport);
		if(saved == 0){
			for (int i = 0; i < 6; i++)
			{
				frame_buf[i] = kuhl_gen_framebuffer(512, 512, &frame_texid[i], NULL);
				// Switch to the framebuffer object
				glBindFramebuffer(GL_FRAMEBUFFER, frame_buf[i]);
				glViewport(0, 0, 512, 512);
				glClearColor(.2, .2, .2, 0);						// set the color to use in the background, try changing it to a bright color for debugging.
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen
				/* Get the view or camera matrix; update the frustum values if needed. */
				float viewMat[16], perspective[16], modelMat[16];
				int shift = 3 * i;
				//if it is looking up or down the Y - Axis
				if(i == 2 || i == 3)
					mat4f_lookat_new(viewMat, 
						0, 0, 0, 
						look_pos[shift + 0], look_pos[shift + 1], look_pos[shift + 2], 
						0, 0, 1);
				else
					mat4f_lookat_new(viewMat, look_pos[shift + 0], look_pos[shift + 1], look_pos[shift + 2], 0, 0, 0, 0, 1, 0);
				mat4f_perspective_new(perspective, 90, 1, .1, 100);
				get_model_matrix(modelMat);
				
				for (int j = 0; j < num_tex; j++)
				{
					float modelView[16];
					mat4f_mult_mat4f_new(modelView, viewMat, texModel[j]);
					drawQuad(&texQuads[j], modelView, perspective);
					kuhl_errorcheck();
				}
				kuhl_errorcheck();
				
			}
			saved = 1;
			// Go back to rendering on the screen:
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// Restore viewport
			glViewport(origViewport[0], origViewport[1], origViewport[2], origViewport[3]);
		}

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
		glClearColor(.2f, .2f, .2f, 0.0f); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		/* Turn on blending (note, if you are using transparent textures,
		   the transparency may not look correct unless you draw further
		   items before closer items.). */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		/* Get the view or camera matrix; update the frustum values if needed. */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);
		float modelMat[16];
		get_model_matrix(modelMat);
		for (int i = 0; i < num_tex; i++)
		{
			float modelView[16];
			mat4f_mult_mat4f_new(modelView, viewMat, texModel[i]);
			drawQuad(&texQuads[i], modelView, perspective);
			kuhl_errorcheck();
		}
		glUseProgram(program);
		kuhl_errorcheck();
		drawDuck(modelMat, viewMat, perspective);

		kuhl_errorcheck();

		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, modelMat); // modelview = view * model

		if (showOrigin && origingeom != NULL && 0)
		{
			/* Save current line width */
			GLfloat origLineWidth;
			glGetFloatv(GL_LINE_WIDTH, &origLineWidth);
			glLineWidth(4); // make lines thick

			/* Object coordinate system origin */
			kuhl_geometry_draw(origingeom); /* Draw the origin marker */

			/* World coordinate origin */
			mat4f_copy(modelview, viewMat);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
							   1,			// number of 4x4 float matrices
							   0,			// transpose
							   modelview);  // value
			kuhl_geometry_draw(origingeom); /* Draw the origin marker */

			/* Restore line width */
			glLineWidth(origLineWidth);
		}

		//frame buffs
		for (int i = 0; i < 6; i++)
		{
		}

		// aspect ratio will be zero when the program starts (and FPS hasn't been computed yet)
		if (dgr_is_master())
		{
			float stretchLabel[16];
			mat4f_scale_new(stretchLabel, 1 / 8.0f / viewmat_window_aspect_ratio(), 1 / 8.0f, 1.0f);

			/* Position label in the upper left corner of the screen */
			float transLabel[16];
			mat4f_translate_new(transLabel, -.9f, .8f, 0.0f);
			mat4f_mult_mat4f_new(modelview, transLabel, stretchLabel);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, 0, modelview);

			/* Make sure we don't use a projection matrix */
			float identity[16];
			mat4f_identity(identity);
			glUniformMatrix4fv(kuhl_get_uniform("Projection"), 1, 0, identity);

			/* Don't use depth testing and make sure we use the texture
			 * rendering style */
			glDisable(GL_DEPTH_TEST);

			kuhl_geometry_draw(fpsgeom); /* Draw the quad */
			glEnable(GL_DEPTH_TEST);
			kuhl_errorcheck();
		}

		glUseProgram(0); // stop using a GLSL program.

		viewmat_end_eye(viewportID);
	} // finish viewport loop

	/* Update the model for the next frame based on the time. We
	 * convert the time to seconds and then use mod to cause the
	 * animation to repeat. */
	double time = glfwGetTime();
	dgr_setget("time", &time, sizeof(double));
	kuhl_update_model(modelgeom, 0, fmodf((float)time, 10.0f));

	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	//kuhl_video_record("videoout", 30);
}

void drawDuck(float *modelMat, float *viewMat, float *perspective)
{
	//float geomtrans[16] = { 1,0,0,0,0,1,0,0,0,0,1,0,-.081,-.525,.022,1};
	

	for (int i = 0; i < 6; i++)
	{
		kuhl_geometry_texture(modelgeom, frame_texid[i], frame_buf_name[i], KG_WARN);
	}

	/* Send the perspective projection matrix to the vertex program. */
	glUniformMatrix4fv(kuhl_get_uniform("Projection"),
					   1,			 // number of 4x4 float matrices
					   0,			 // transpose
					   perspective); // value

	kuhl_errorcheck();
	glUniformMatrix4fv(kuhl_get_uniform("ViewMat"),
					   1,
					   0,
					   viewMat);
	kuhl_errorcheck();

	float modelview[16];
	mat4f_mult_mat4f_new(modelview, viewMat, modelMat); // modelview = view * model
	kuhl_errorcheck();
	/* Send the modelview matrix to the vertex program. */
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
					   1,		   // number of 4x4 float matrices
					   0,		   // transpose
					   modelview); // value
	kuhl_errorcheck();

	kuhl_errorcheck();
	kuhl_geometry_draw(modelgeom); /* Draw the model */
	kuhl_errorcheck();
}

void drawQuad(kuhl_geometry *quad, float *modelview, float *perspective)
{
	glUseProgram(texProg);
	//printf("Drawing Quad\n");
	/* Send the perspective projection matrix to the vertex program. */
	glUniformMatrix4fv(kuhl_get_uniform("Projection"),
					   1,			 // number of 4x4 float matrices
					   0,			 // transpose
					   perspective); // value

	/* Send the modelview matrix to the vertex program. */
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
					   1,		   // number of 4x4 float matrices
					   0,		   // transpose
					   modelview); // value
	kuhl_errorcheck();

	kuhl_errorcheck();
	kuhl_geometry_draw(quad); /* Draw the model */
	kuhl_errorcheck();
	//printf("Done Drawing Quad\n");
	glUseProgram(0);
}

void prepQuads()
{
	char *backImg = "../images/pavers.jpg";
	char *floorImg = "../images/blue.png";
	glUseProgram(texProg);

	//modified carousel code for drawing a textured quad
	{
		//drawing the back image
		kuhl_geometry_new(&texQuads[0], texProg, 4, GL_TRIANGLES);
		//printf("Making a quad\n");
		kuhl_errorcheck();
		GLfloat texcoordData[] = {0, 0,
								  1, 0,
								  1, 1,
								  0, 1};
		kuhl_geometry_attrib(&texQuads[0], texcoordData, 2, "in_TexCoord", KG_WARN);
		kuhl_errorcheck();
		// The 2 parameter above means each texture coordinate is a 2D coordinate.

		/* Load the texture. It will be bound to texId */
		//printf("loading a texture: %s\n",filename);
		GLuint bId = 0;
		kuhl_read_texture_file(backImg, &bId);
		/* The data that we want to draw */

		GLfloat vertexData[12] = {-3, 0, -5,
								  3, 0, -5,
								  -3, 50, -5,
								  3, 50, -5};

		kuhl_geometry_attrib(&texQuads[0], vertexData, 3, "in_Position", KG_WARN);
		kuhl_errorcheck();
		// The 3 parameter above means that each vertex position is a 3D coordinate.

		/* Tell this piece of geometry to use the texture we just loaded. */
		kuhl_geometry_texture(&texQuads[0], bId, "tex", KG_WARN);
		kuhl_errorcheck();

		GLuint indexData[] = {
			0, 1, 2,
			1, 2, 3};

		kuhl_geometry_indices(&texQuads[0], indexData, 6);

		kuhl_errorcheck();
		printf("Finished back quad\n");
	}
	//drawing the floor image
	{
		kuhl_geometry_new(&texQuads[1], texProg, 4, GL_TRIANGLES);
		//printf("Making a quad\n");
		kuhl_errorcheck();
		GLfloat texcoordData[] = {0, 0,
								  1, 0,
								  1, 1,
								  0, 1};
		kuhl_geometry_attrib(&texQuads[1], texcoordData, 2, "in_TexCoord", KG_WARN);
		kuhl_errorcheck();
		// The 2 parameter above means each texture coordinate is a 2D coordinate.

		/* Load the texture. It will be bound to texId */
		//printf("loading a texture: %s\n",filename);
		GLuint fId = 0;
		kuhl_read_texture_file(floorImg, &fId);
		/* The data that we want to draw */

		GLfloat vertexData[12] = {-5, 0, -5,
								  -5, 0, 5,
								  5, 0, -5,
								  5, 0, 5};

		kuhl_geometry_attrib(&texQuads[1], vertexData, 3, "in_Position", KG_WARN);
		kuhl_errorcheck();
		// The 3 parameter above means that each vertex position is a 3D coordinate.
		/* Tell this piece of geometry to use the texture we just loaded. */
		kuhl_geometry_texture(&texQuads[1], fId, "tex", KG_WARN);
		kuhl_errorcheck();

		GLuint indexData[] = {
			0, 1, 2,
			1, 2, 3};

		kuhl_geometry_indices(&texQuads[1], indexData, 6);

		kuhl_errorcheck();
		printf("Finished floor quad\n");
	}
	glUseProgram(0);

	//setting up each quad's model matrix
	float temp[16] = {1, 0, 0, 0,
					  0, 1, 0, 0,
					  0, 0, 1, 0,
					  0, 0, 0, 1};
	float transform[16];
	mat4f_translate_new(transform, 0, -.5, -5);
	mat4f_mult_mat4f_new(temp, transform, temp);
	for (int i = 0; i < num_tex; i++)
	{
		for (int j = 0; j < 16; j++)
			texModel[i][j] = temp[j];
	}
}

int main(int argc, char **argv)
{
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);

	char *modelFilename = NULL;
	char *modelTexturePath = NULL;

	int currentArgIndex = 1; // skip program name
	int usageError = 0;
	while (argc > currentArgIndex)
	{
		if (strcmp(argv[currentArgIndex], "--fit") == 0)
			fitToView = 1;
		else if (strcmp(argv[currentArgIndex], "--origin") == 0)
			showOrigin = 1;
		else if (modelFilename == NULL)
		{
			modelFilename = argv[currentArgIndex];
			modelTexturePath = NULL;
		}
		else if (modelTexturePath == NULL)
			modelTexturePath = argv[currentArgIndex];
		else
		{
			usageError = 1;
		}
		currentArgIndex++;
	}

	//initialize frame buffers
	for (int i = 0; i < 6; i++)
	{
		frame_buf[i] = 0;
		frame_texid[i] = 0;
	}

	// If we have no model to load or if there were too many arguments.
	if (modelFilename == NULL || usageError)
	{
		printf("Usage:\n"
			   "%s [--fit] [--origin] modelFile     - Textures are assumed to be in the same directory as the model.\n"
			   "- or -\n"
			   "%s [--fit] [--origin] modelFile texturePath\n"
			   "If the optional --fit parameter is included, the model will be scaled and translated to fit within the approximate view of the camera\n"
			   "If the optional --origin parameter is included, a box will is drawn at the origin and unit-length lines are drawn down each axis.\n",
			   argv[0], argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);
	texProg = kuhl_create_program("cubemap-tex.vert", "cubemap-tex.frag");
	prepQuads();
	dgr_init(); /* Initialize DGR based on environment variables. */

	viewmat_init(initCamPos, initCamLook, initCamUp);

	// Clear the screen while things might be loading
	glClearColor(.2f, .2f, .2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the model from the file
	modelgeom = kuhl_load_model(modelFilename, modelTexturePath, program, bbox);
	origingeom = kuhl_load_model("../models/origin/origin.obj", modelTexturePath, program, NULL);

	while (!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
