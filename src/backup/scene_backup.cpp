/*
 * Copyright (C) 2010 Josh A. Beam
 * All rights reserved.
 * http://joshbeam.com/articles/getting_started_with_glsl/
 *
 * Slightly modified by Gauthier Lafruit for INFO-H502
 *
 * Good GLSL tutorial:
 * http://nehe.gamedev.net/article/glsl_an_introduction/25007/
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef _WIN32
	#include <windows.h>
	#include <wingdi.h>
	#define GLUT_DISABLE_ATEXIT_HACK
#else
	#include <sys/time.h>
#endif /* _WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
using namespace std;

// OpenGL includes
#include <GL/glew.h>
#include <GL/glut.h>

#include "variables.h"
#include "shaders.h"
#include "makeTexImage.h"

const double PI = 3.141592654;
const double TWOPI = 6.283185308;

GLuint vertexshader, fragmentshader, g_program ; // shaders

/* shader functions defined in shader.c */
extern void shaderAttachFromFile(GLuint, GLenum, const char *);

//static GLuint g_program;
static GLuint g_programCameraPositionLocation;
static GLuint g_programLightPositionLocation;
static GLuint g_programLightColorLocation;
static GLuint g_programBaseTextureLocation;
static GLuint g_programBumpTextureLocation;

static GLuint g_cylinderBufferId;
static unsigned int g_cylinderNumVertices;

static float g_cameraPosition[3];

#define NUM_LIGHTS 3
static float g_lightPosition[NUM_LIGHTS * 3];
static float g_lightColor[NUM_LIGHTS * 3];
static float g_lightRotation;

int texImageWidth;
int texImageHeight;
static GLuint texturePointer[6];

//images for texture maps for 6 faces of cube, and 1 texture for the sphere
char maps[][40] = {"data/front.png",  "data/back.png",  "data/right.png", "data/left.png",
		 "data/up.png", "data/down.png", "data/earth.png"};
int NmapHandles = 6;    // 6 for cube, 1 for sphere
int startMapHandle = 0; // 0 for cube, 6 for sphere

void createCube(double halfSize){

   // Load textures
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glGenTextures(NmapHandles, texturePointer);			

   for ( int i = startMapHandle; i < (startMapHandle + NmapHandles); ++i ) {	
     GLubyte *texImage = makeTexImage( maps[i], texImageWidth, texImageHeight); 
     if ( !texImage ) {
       printf("\nError reading %s \n", maps[i] );
       continue;
     }
     glBindTexture(GL_TEXTURE_2D, texturePointer[i]);	
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texImageWidth, 
                texImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage);

     delete texImage;					
   }

   float x0 = -halfSize, y0 = -halfSize, z0 = -halfSize;
   float x1 = halfSize, y1 = halfSize, z1 = halfSize;
   // Vertices
   float faceVertex[6][4][3] =  {   // 6 faces, 4 vertices each, with 3 coordinates per vertex
	    {{x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}},	//front
    	{{x0, y1, z0}, {x1, y1, z0}, {x1, y0, z0}, {x0, y0, z0}},	//back
		{{x1, y0, z1}, {x1, y0, z0}, {x1, y1, z0}, {x1, y1, z1}},	//right 
		{{x0, y0, z1}, {x0, y1, z1}, {x0, y1, z0}, {x0, y0, z0}},	//left 
		{{x0, y1, z1}, {x1, y1, z1}, {x1, y1, z0}, {x0, y1, z0}},	//top 
		{{x0, y0, z1}, {x0, y0, z0}, {x1, y0, z0}, {x1, y0, z1}}	//bottom 
		};
   // Normals (not normalized, since this will be done in the shaders anyway)
#define N_FLB {-1,-1,+1} // Normal at Front Left  Bottom corner
#define N_FRB {+1,-1,+1} // Normal at Front Right Bottom corner
#define N_FLT {-1,+1,+1} // Normal at Front Left  Top corner
#define N_FRT {+1,+1,+1} // Normal at Front Right Top corner
#define N_BLB {-1,-1,-1} // Normal at Back  Left  Bottom corner
#define N_BRB {+1,-1,-1} // Normal at Back  Right Bottom corner
#define N_BLT {-1,+1,-1} // Normal at Back  Left  Top corner
#define N_BRT {+1,+1,-1} // Normal at Back  Right Top corner
   float faceNormal[6][4][3] =  {   // 6 faces, 4 normals in each vertex, with 3 coordinates per normal
	    {N_FLB, N_FRB, N_FRT, N_FLT},	//front
    	{N_BLT, N_BRT, N_BRB, N_BLB},	//back
		{N_FRB, N_BRB, N_BRT, N_FRT},	//right 
		{N_FLB, N_FLT, N_BLT, N_BLB},	//left 
		{N_FLT, N_FRT, N_BRT, N_BLT},	//top 
		{N_FLB, N_BLB, N_BRB, N_FRB}	//bottom 
		};

   for ( int i = startMapHandle; i < (startMapHandle + NmapHandles); ++i ) { //draw object with texture images
     glBindTexture(GL_TEXTURE_2D, texturePointer[i]);
     glBegin(GL_QUADS);
       glTexCoord2f(0.0, 0.0); glVertex3fv ( faceVertex[i][0] ); glNormal3fv(faceNormal[i][0]);	
       glTexCoord2f(1.0, 0.0); glVertex3fv ( faceVertex[i][1] ); glNormal3fv(faceNormal[i][1]);	
       glTexCoord2f(1.0, 1.0); glVertex3fv ( faceVertex[i][2] ); glNormal3fv(faceNormal[i][2]);
       glTexCoord2f(0.0, 1.0); glVertex3fv ( faceVertex[i][3] ); glNormal3fv(faceNormal[i][3]);
     glEnd();
   }

}

static void
createCylinder(unsigned int divisions)
{
	const int floatsPerVertex = 6;
	unsigned int i, size;
	float *v;

	g_cylinderNumVertices = (divisions + 1) * 2;
	size = floatsPerVertex * g_cylinderNumVertices;

	/* generate vertex data */
	v = (float *) malloc(sizeof(float) * size);
	for(i = 0; i <= divisions; ++i) {
		float r = ((M_PI * 2.0f) / (float)divisions) * (float)i;
		unsigned int index1 = i * 2 * floatsPerVertex;
		unsigned int index2 = index1 + floatsPerVertex;

		/* vertex positions */
		v[index1 + 0] = cosf(r);
		v[index1 + 1] = 1.0f;
		v[index1 + 2] = -sinf(r);
		v[index2 + 0] = cosf(r);
		v[index2 + 1] = -1.0f;
		v[index2 + 2] = -sinf(r);

		/* normals */
		v[index1 + 3] = cosf(r);
		v[index1 + 4] = 0.0f;
		v[index1 + 5] = -sinf(r);
		v[index2 + 3] = v[index1 + 3];
		v[index2 + 4] = v[index1 + 4];
		v[index2 + 5] = v[index1 + 5];
	}

	/* create vertex buffer */
	glGenBuffers(1, &g_cylinderBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, g_cylinderBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, v, GL_STATIC_DRAW);
	free(v);

	/* enable arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	/* set pointers */
	glVertexPointer(3, GL_FLOAT, sizeof(float) * floatsPerVertex, 0);
	glNormalPointer(GL_FLOAT, sizeof(float) * floatsPerVertex, (const GLvoid *)(sizeof(float) * 3));
}

// simple Point class
class Point3 {
public:
  double x;
  double y;
  double z;
  Point3()
  {
    x = y = z = 0.0;
  }

  Point3 ( double x0, double y0, double z0 )
  {
    x = x0;	y = y0;	   z = z0;
  }
};



void sceneCycle(void);

void loadTextures(){
   // Load textures
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glGenTextures(NmapHandles, texturePointer);			

   for ( int i = startMapHandle; i < (startMapHandle + NmapHandles); ++i ) {	
     GLubyte *texImage = makeTexImage( maps[i], texImageWidth, texImageHeight); 
     if ( !texImage ) {
       printf("\nError reading %s \n", maps[i] );
       continue;
     }
     
	glBindTexture(GL_TEXTURE_2D, texturePointer[i]);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texImageWidth, 
                texImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage);
     

     delete texImage;					
   }
}



void
sceneInit(void)
{

	loadTextures();

	GLint result;
	GLuint index;

	// Initialize shaders C++ style
    vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/shader.vp") ;
    fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/shader.fp") ;
    g_program = initprogram(vertexshader, fragmentshader) ; 


	// get uniform locations 
	g_programCameraPositionLocation = glGetUniformLocation(g_program, "cameraPosition");
	g_programLightPositionLocation = glGetUniformLocation(g_program, "lightPosition");
	g_programLightColorLocation = glGetUniformLocation(g_program, "lightColor");
	g_programBaseTextureLocation = glGetUniformLocation(g_program, "baseTexture");
	index = 0;
	glUniform1i(g_programBaseTextureLocation, index);
	glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texturePointer[startMapHandle]);
    //glBindSampler(0, linearFiltering);

	// set up red/green/blue lights 
	g_lightColor[0] = 1.0f; g_lightColor[1] = 0.0f; g_lightColor[2] = 0.0f;
	g_lightColor[3] = 0.0f; g_lightColor[4] = 1.0f; g_lightColor[5] = 0.0f;
	g_lightColor[6] = 0.0f; g_lightColor[7] = 0.0f; g_lightColor[8] = 1.0f;

	// create cylinder 
	createCylinder(36);

	// create cube 
	// createSphere(1.0, 40);

	// create sphere 
	//createCube(1);

	// do the first cycle to initialize positions 
	g_lightRotation = 0.0f;
	sceneCycle();

	// setup camera 
	g_cameraPosition[0] = 0.0f;
	g_cameraPosition[1] = 0.0f;
	g_cameraPosition[2] = 4.0f;
	glLoadIdentity();
	glTranslatef(-g_cameraPosition[0], -g_cameraPosition[1], -g_cameraPosition[2]);

}

void
sceneRender(void)
{
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* enable program and set uniform variables */
	glUseProgram(g_program);
	glUniform3fv(g_programCameraPositionLocation, 1, g_cameraPosition);
	glUniform3fv(g_programLightPositionLocation, NUM_LIGHTS, g_lightPosition);
	glUniform3fv(g_programLightColorLocation, NUM_LIGHTS, g_lightColor);

	/* render the cylinder */
	glDrawArrays(GL_TRIANGLE_STRIP, 0, g_cylinderNumVertices);

	/* disable program */
	glUseProgram(0);

	/* render each light */
	for(i = 0; i < NUM_LIGHTS; ++i) {
		/* render sphere with the light's color/position */
		glPushMatrix();
		glTranslatef(g_lightPosition[i * 3 + 0], g_lightPosition[i * 3 + 1], g_lightPosition[i * 3 + 2]);
		glColor3fv(g_lightColor + (i * 3));
		glutSolidSphere(0.4, 36, 36);
		glPopMatrix();
	}

	glutSwapBuffers();
}

static unsigned int
getTicks(void)
{
#ifdef _WIN32
	return GetTickCount();
#else
	struct timeval t;

	gettimeofday(&t, NULL);

	return (t.tv_sec * 1000) + (t.tv_usec / 1000);
#endif /* _WIN32 */
}

void
sceneCycle(void)
{
	static unsigned int prevTicks = 0;
	unsigned int ticks;
	float secondsElapsed;
	int i;

	/* calculate the number of seconds that have
	 * passed since the last call to this function */
	if(prevTicks == 0)
		prevTicks = getTicks();
	ticks = getTicks();
	secondsElapsed = (float)(ticks - prevTicks) / 1000.0f;
	prevTicks = ticks;

	/* update the light positions */
	g_lightRotation += (M_PI / 4.0f) * secondsElapsed;
	for(i = 0; i < NUM_LIGHTS; ++i) {
		const float radius = 1.75f;
		float r = (((M_PI * 2.0f) / (float)NUM_LIGHTS) * (float)i) + g_lightRotation;

		g_lightPosition[i * 3 + 0] = cosf(r) * radius;
		g_lightPosition[i * 3 + 1] = cosf(r) * sinf(r);
		g_lightPosition[i * 3 + 2] = sinf(r) * radius;
	}

	glutPostRedisplay();
}
