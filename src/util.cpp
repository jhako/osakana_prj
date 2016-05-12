

#include "util.h"
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>

void draw_string(const char *str, void *font, float x, float y, float z)
{
	glRasterPos3f(x, y, z);
	while(*str){
		glutBitmapCharacter(font, *str);
		++str;
	}
}
