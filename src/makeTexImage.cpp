// Traditional includes
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
using namespace std;

// Image saving/reading include
#include <FreeImage.h>

// OpenGL includes
#include <GL/glew.h>
#include <GL/glut.h>

//load texture image
GLubyte *makeTexImage(char *loadfile, int &texImageWidth, int &texImageHeight)
{   
// GLubyte *texImage;
// texImage = loadImageRGBA( (char *) loadfile, &width, &height);	
    // replaced by a Free_Image equivalent

    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(loadfile, 0);//Automatocally detects the format(from over 20 formats!)
	FIBITMAP* imagen = FreeImage_Load(formato, loadfile);
 
	FIBITMAP* temp = imagen;
	imagen = FreeImage_ConvertTo32Bits(imagen);
	FreeImage_Unload(temp);
 
	texImageWidth = FreeImage_GetWidth(imagen);
	texImageHeight = FreeImage_GetHeight(imagen);

	GLubyte* textura = new GLubyte[4*texImageWidth*texImageHeight];
	char* pixeles = (char*)FreeImage_GetBits(imagen);
	//FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).
 
	for(int j= 0; j<texImageWidth*texImageHeight; j++){
		textura[j*4+0]= pixeles[j*4+2];
		textura[j*4+1]= pixeles[j*4+1];
		textura[j*4+2]= pixeles[j*4+0];
		textura[j*4+3]= pixeles[j*4+3];
	}

   return textura;
}