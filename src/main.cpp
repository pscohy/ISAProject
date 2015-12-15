#include <Windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <FreeImage.h>
#include <iostream>


static float Xangle = 150.0, Yangle = 60.0, Zangle = 0.0; // Angles to rotate the cylinder.

GLuint texture_GL;


void cylinder()
{
	const double PI = 3.14159;

	/* top triangle */
	double i, resolution = 0.1;
	double height = 1;
	double radius = 0.5;

	glPushMatrix();
	glTranslatef(0, -0.5, 0);

	/*glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f( 0.5, 0.5 );
	glVertex3f(0, height, 0);  // center
	for (i = 2 * PI; i >= 0; i -= resolution)

	{
	glTexCoord2f( 0.5f * cos(i) + 0.5f, 0.5f * sin(i) + 0.5f );
	glVertex3f(radius * cos(i), height, radius * sin(i));
	}
	// close the loop back to 0 degrees
	glTexCoord2f( 0.5, 0.5 );
	glVertex3f(radius, height, 0);
	glEnd();*/

	/* bottom triangle: note: for is in reverse order */
	/*glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f( 0.5, 0.5 );
	glVertex3f(0, 0, 0);  //center
	for (i = 0; i <= 2 * PI; i += resolution)
	{
	glTexCoord2f( 0.5f * cos(i) + 0.5f, 0.5f * sin(i) + 0.5f );
	glVertex3f(radius * cos(i), 0, radius * sin(i));
	}
	glEnd();*/

	/* middle tube */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= 2 * PI; i += resolution)
	{
		const float tc = (i / (float)(2 * PI));
		glTexCoord2f(tc, 0.0);
		glVertex3f(radius * cos(i), 0, radius * sin(i));
		glTexCoord2f(tc, 1.0);
		glVertex3f(radius * cos(i), height, radius * sin(i));
	}
	/* close the loop back to zero degrees */
	glTexCoord2f(0.0, 0.0);
	glVertex3f(radius, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(radius, height, 0);
	glEnd();

	glPopMatrix();
}


GLuint LoadTexture(const char *filename)
{
	GLuint tmp_texture;
	int width, height;

	// Read in the texture data from file
	FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(filename, 0);
	FIBITMAP* tmp_image = FreeImage_Load(formato, filename);
	FIBITMAP* image = FreeImage_ConvertTo24Bits(tmp_image);

	// Convert this image in OpenGL data format
	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);
	GLubyte* openGL_image = new GLubyte[3 * width*height];
	char* pixels = (char*)FreeImage_GetBits(image);

	// OpenGL stores image data in reverse order
	for (int j = 0; j<width*height; j++)
	{
		openGL_image[j * 3 + 0] = pixels[j * 3 + 2];
		openGL_image[j * 3 + 1] = pixels[j * 3 + 1];
		openGL_image[j * 3 + 2] = pixels[j * 3 + 0];
	}

	// Fixed OpenGL pipeline texture setup
	glGenTextures(1, &tmp_texture); //generate the texture with the loaded data  
	glBindTexture(GL_TEXTURE_2D, tmp_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)openGL_image);  //the actual texture generation

																												// The texture is now in OpenGL memory. The image can hence be freed
	FreeImage_Unload(tmp_image);
	FreeImage_Unload(image);

	// Pass the texture pointer as result
	return tmp_texture;
}

GLuint tex;
void init()
{
	unsigned char data[] =
	{
		128, 128, 128, 255,
		255, 0, 0, 255,
		0, 255, 0, 255,
		0, 0, 255, 255,
	};

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void initOpenGL() {
	glewInit();
	texture_GL = LoadTexture("data/earth.png");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_GL);
}

/*float angle = 0;
void timer( int value )
{
angle += 6;
glutPostRedisplay();
glutTimerFunc( 16, timer, 0 );
}*/

void display()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1.0, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -5);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//glRotatef( angle, 0.2, 0.3, 0.1 );
	glRotatef(Zangle, 0.0, 0.0, 1.0);
	glRotatef(Yangle, 0.0, 1.0, 0.0);
	glRotatef(Xangle, 1.0, 0.0, 0.0);

	glEnable(GL_TEXTURE_2D);
	//glBindTexture( GL_TEXTURE_2D, tex );
	cylinder();

	glutSwapBuffers();
}

void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 'x':
		Xangle += 5.0;
		if (Xangle > 360.0) Xangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'X':
		Xangle -= 5.0;
		if (Xangle < 0.0) Xangle += 360.0;
		glutPostRedisplay();
		break;
	case 'y':
		Yangle += 5.0;
		if (Yangle > 360.0) Yangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'Y':
		Yangle -= 5.0;
		if (Yangle < 0.0) Yangle += 360.0;
		glutPostRedisplay();
		break;
	case 'z':
		Zangle += 5.0;
		if (Zangle > 360.0) Zangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'Z':
		Zangle -= 5.0;
		if (Zangle < 0.0) Zangle += 360.0;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(600, 600);
	glutCreateWindow("GLUT");
	//init();
	initOpenGL();
	glutDisplayFunc(display);
	//glutTimerFunc( 0, timer, 0 );
	glutKeyboardFunc(keyInput);
	glutMainLoop();
	return 0;
}