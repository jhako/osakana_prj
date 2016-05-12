

#include <stdlib.h>
#include <GLFW/glfw3.h>
#include "target.h"
#include "world.h"


void Target::update(World * p_world)
{
	// nothing to do
}

void Target::render(World * p_world)
{
	//--‰~‚ð•`‰æ--
	glBegin(GL_POLYGON);
	glColor3f(1.0, 0, 0);
	const int R = 3, N = 100;
	for (int i = 0; i < N; i++)
	{
		auto x = pos.x + R * cos(2.0 * 3.141592 * ((double)i / N));
		auto y = pos.y + R * sin(2.0 * 3.141592 * ((double)i / N));
		glVertex3f(x, y, 0.0);
	}
	glEnd();
}
