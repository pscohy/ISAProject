/************************************************************************/
/*          Draw a cube or sphere with texture images.                  */
/*          The object can be rotated by pressing keys                  */
/*                 'x', 'X', 'y', 'Y', 'z', 'Z'.                        */
/*  Code inspired from:                                                 */
/*  - Fore June, "An Introduction to 3D Computer Graphics,              */
/*                Stereoscopic Image, and Animation in OpenGL and C/C++ */
/************************************************************************/

// Traditional includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
using namespace std;

// Image saving/reading include
#include <FreeImage.h>
#include "makeTexImage.h"

// OpenGL includes
#include <GL/glew.h>
#include <GL/glut.h>
//#include "shadersC.h"
//#include "shadersCPP.h"
//#include "arb_program.h"



const double PI = 3.141592654;
const double TWOPI = 6.283185308;

static int mouseX = 320;
static int mouseY = 240;
float posX;
float posY;

int texImageWidth;
int texImageHeight;
int window;
static GLuint handles[6];					//texture names
int angleX= 30, angleY = 40, angleZ = 0;	//rotation angles

//images for texture maps for 6 faces of cube, and 1 texture for the sphere
char maps[][40] = {"data/front.png",  "data/back.png",  "data/right.png", "data/left.png",
		 "data/up.png", "data/down.png", "data/earth.png"};

// Number of textures in the texture mapping
//**** [1] ****
int NmapHandles = 1; //6;
int startMapHandle = 6;

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

/*********************************************************************************/
/*               Some image read/write functions for window snapshot             */
/*********************************************************************************/
void saveScreenshot(string fname, int width, int height) {
	int pix = width * height;
	BYTE *pixels = new BYTE[3*pix];	
	glReadBuffer(GL_FRONT);
	glReadPixels(0,0,width,height,GL_BGR,GL_UNSIGNED_BYTE, pixels);

	FIBITMAP *img = FreeImage_ConvertFromRawBits(pixels, width, height, width * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);

	cout << "Saving screenshot: " << fname << "\n";
	FreeImage_Save(FIF_PNG, img, fname.c_str(), 0);
	delete pixels;
}


/***********************************************************************/
/*                       Define lights and materials                   */
/***********************************************************************/

void addLightAndMaterials() 
{
	//Material properties: reflectivity coefficients
   GLfloat mat_specular[] = { 0.6, .8, 1, 1.0 };
   GLfloat mat_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
   GLfloat mat_diffuse[] = { 0.6, 0.4, 0.2, 1.0 };
   GLfloat mat_shininess0[] = { 20.0}; 
   GLfloat mat_shininess1[] = { 100.0}; 

   //Light properties
   GLfloat light_position0[] = { 1.0, 1.0, 1.0, 0.0 };  //directional light
   GLfloat light_position1[] = { 1.0, 1.0, 0.0, 0.0 };  //directional light
   GLfloat light0[] = { 1, 1, 1 };  //light source 0
   GLfloat light1[] = {1, 1, 1 };   //light source 1

   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess0);
 
//**** [2] ****
   glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
   glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0 );
   glLightfv(GL_LIGHT0, GL_AMBIENT,  light0 );
   glLightfv(GL_LIGHT0, GL_SPECULAR, light0 );

   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess1);
   glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, light1 );
   glLightfv(GL_LIGHT1, GL_AMBIENT, light1 );
   glLightfv(GL_LIGHT1, GL_SPECULAR, light1 );

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHT1);

}

/***********************************************************************/
/*           Basic OpenGL initialisation and Texture loading           */
/***********************************************************************/
void init(void)
{    
   glClearColor (1, 1, 1, 0.0);

//**** [3] ****
   //glShadeModel(GL_FLAT);
   glShadeModel(GL_SMOOTH);

   // Add lights and materials
//**** [4] ****
   addLightAndMaterials();

   glEnable(GL_DEPTH_TEST);

   // Load textures
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glGenTextures(NmapHandles, handles);			//handles is global
//**** [5] ****
   for ( int i = startMapHandle; i < (startMapHandle + NmapHandles); ++i ) {	
     GLubyte *texImage = makeTexImage( maps[i], texImageWidth, texImageHeight); // slightly changed to have also the width and height included
     if ( !texImage ) {
       printf("\nError reading %s \n", maps[i] );
       continue;
     }
     glBindTexture(GL_TEXTURE_2D, handles[i]);	//works on handles
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//**** [6] ****
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texImageWidth, 
                texImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage);

     delete texImage;					
   }
   printf("\nPress x, y, z or X, Y, Z to rotate the object\n");
}


/***********************************************************************/
/*    Create a cube centered at 0, with side euqual to 2x halfSize     */
/***********************************************************************/
void createCube(double halfSize){

   float x0 = -halfSize, y0 = -halfSize, z0 = -halfSize;
   float x1 = halfSize, y1 = halfSize, z1 = halfSize;
   float face[6][4][3] =  { 
	    {{x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}},	//front
    	{{x0, y1, z0}, {x1, y1, z0}, {x1, y0, z0}, {x0, y0, z0}},	//back
		{{x1, y0, z1}, {x1, y0, z0}, {x1, y1, z0}, {x1, y1, z1}},	//right 
		{{x0, y0, z1}, {x0, y1, z1}, {x0, y1, z0}, {x0, y0, z0}},	//left 
		{{x0, y1, z1}, {x1, y1, z1}, {x1, y1, z0}, {x0, y1, z0}},	//top 
		{{x0, y0, z1}, {x0, y0, z0}, {x1, y0, z0}, {x1, y0, z1}}	//bottom 
		};

//**** [7] ****
   for ( int i = startMapHandle; i < (startMapHandle + NmapHandles); ++i ) { //draw object with texture images
     glBindTexture(GL_TEXTURE_2D, handles[i]);
     glBegin(GL_QUADS);
       glTexCoord2f(0.0, 0.0); glVertex3fv ( face[i][0] ); 	
       glTexCoord2f(1.0, 0.0); glVertex3fv ( face[i][1] );	
       glTexCoord2f(1.0, 1.0); glVertex3fv ( face[i][2] );
       glTexCoord2f(0.0, 1.0); glVertex3fv ( face[i][3] );
     glEnd();
   }

}

/***********************************************************************/
/*    Create a sphere centered at 0, with radius r, and precision n    */
/*    Draw a point for zero radius spheres                             */
/***********************************************************************/
void createSphere(double r,int n)
{
   int i,j;
   double phi1, phi2, theta, u, v;
   Point3 c;
   Point3 p, q;
   
   if ( r < 0 || n < 0 )
      return;
   if (n < 4 || r <= 0) {
      glBegin(GL_POINTS);
      glVertex3f(c.x,c.y,c.z);
      glEnd();
      return;
   }

   for ( j=0; j < n; j++ ) {
      phi1 = j * TWOPI / n;
      phi2 = (j + 1) * TWOPI / n;	//next phi

      glBegin(GL_QUAD_STRIP);
      for ( i=0; i <= n; i++ ) {
         theta = i * PI / n;

        q.x = sin ( theta ) * cos ( phi2 );
		q.y = sin ( theta ) * sin ( phi2 );
        q.z = cos ( theta );
        p.x = c.x + r * q.x;
        p.y = c.y + r * q.y;
        p.z = c.z + r * q.z;

        glNormal3f ( q.x, q.y, q.z );
        u = (double)(j+1) / n;		// column
	 	v = 1 - (double) i / n;		// row
        glTexCoord2f( u, v );
        glVertex3f( p.x, p.y, p.z );

        q.x = sin ( theta ) * cos ( phi1 );
		q.y = sin ( theta ) * sin ( phi1 );
        q.z = cos ( theta );
        p.x = c.x + r * q.x;
        p.y = c.y + r * q.y;
        p.z = c.z + r * q.z;

         glNormal3f ( q.x, q.y, q.z );
         u = (double) j / n;		// column
		 v = 1 - (double) i / n;	// row
         glTexCoord2f( j / (double) n, 1 - i / (double) n );
         glVertex3f ( p.x, p.y, p.z );
      }
      glEnd();
   }
}

/***********************************************************************/
/*    Create a sphere centered at 0, with radius r, and precision n    */
/*                       using the GLUT libray                         */
/***********************************************************************/
void createGlutSphere(GLdouble radius, GLint Ndivisions){

	glutSolidSphere (radius, Ndivisions, Ndivisions);

}


/***********************************************************************/
/*             Create a teapot using the GLUT libray                   */
/*          There is something special in the face culling             */
/***********************************************************************/
void createGlutTeapot(GLdouble size){

//**** [8] ****
   glFrontFace(GL_CW); 
   glutSolidTeapot(2.0);		
   glFrontFace(GL_CCW);

}


/***********************************************************************/
/*    Displays an object with starting rotations and translations      */
/***********************************************************************/
void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//**** [9] ****
   glEnable(GL_TEXTURE_2D);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

   glEnable( GL_CULL_FACE );
   glCullFace ( GL_BACK );
  
   glPushMatrix(); 
   glRotatef( angleX, 1.0, 0.0, 0.0);			//rotate the cube along x-axis
   glRotatef( angleY, 0.0, 1.0, 0.0);			//rotate along y-axis	
   glRotatef( angleZ, 0.0, 0.0, 1.0);			//rotate along z-axis

   glPointSize(6);

//**** [10] ****
   // create the object and map its textures
   //createCube(1.0);			// cube with 6 textures
   createSphere(3.0, 40);		// sphere with a texture
   //createGlutSphere(3.0, 40); // sphere without texture
   //createGlutTeapot(2.0);		// GLUT teapot

   glPopMatrix();
   glFlush();
   glDisable(GL_TEXTURE_2D);
}

/***********************************************************************/
/*                       Basic keyboard interaction                    */
/***********************************************************************/
void keyboard(unsigned char key, int x, int y)
{
  switch(key) {
    case 'x':
      angleX = ( angleX + 3 ) % 360;
      break;
    case 'X':
      angleX = ( angleX - 3 ) % 360;
      break;
    case 'y':
      angleY = ( angleY + 3 ) % 360;
      break;
    case 'Y':
      angleY = ( angleY - 3 ) % 360;
      break;
    case 'z':
      angleZ = ( angleZ + 3 ) % 360;
      break;
    case 'Z':
      angleZ = ( angleZ - 3 ) % 360;
      break;
    case 'r':
      angleX = angleY = angleZ = 0;
      break;
    case 27: /* escape */
        glutDestroyWindow(window);
        exit(0);
  }
  glutPostRedisplay();
}

void specialKey(int key,int x,int y) {
	switch(key) {
	case 100: //left arrow
		angleY = ( angleY - 3 ) % 360;
		break;
	case 101: //up arrow
		angleX = ( angleX + 3 ) % 360;
		break;
	case 102: //right arrow
		angleY = ( angleY + 3 ) % 360;
		break;
	case 103: //down arrow
		angleX = ( angleX - 3 ) % 360;
		break;
	}
	glutPostRedisplay();
}

/***********************************************************************/
/*                              Mouse actions                          */
/***********************************************************************/

static void
mouseMotion (int x, int y)
{
    // mouse rotation
	posX += (float)(x - mouseX);
	posY += (float)(y - mouseY);

	mouseX = x;
	mouseY = y;

	glutPostRedisplay ();
}


static void
mouseButton (int button, int state, int x, int y)
{
  mouseX = x;
  mouseY = y;
}

/***********************************************************************/
/*             Actions to display and resize the window                */
/***********************************************************************/
void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
//**** [11] ****
   //gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
   glOrtho(-4.0, 4.0, -4.0 * (GLfloat) h / (GLfloat) w,
            4.0 * (GLfloat) h / (GLfloat) w, -10.0, 10.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
//**** [12] ****
   //gluLookAt ( 0, 0, 5, 0, 0, 0, 0, 1, 0 );
   gluLookAt ( 5, 0, 0, 0, 0, 0, 0, 0, 1 );
}

/***********************************************************************/
/*                Window and Interaction initialization                */
/***********************************************************************/
int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100);
   window = glutCreateWindow("OpenGL Output");
   init();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(specialKey);
   glutMotionFunc (mouseMotion);
   glutMouseFunc (mouseButton);
   glutMainLoop();
   return 0; 
}
