
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <random>
#include "noise.h"
#include "world.h"

Noise::Noise(vec2d pos_)
	: pos(pos_)
{
	static int unique_id = 0;
	id = unique_id++;
}

void Noise::update(World * p_world)
{
	life -= 1;
}

void Noise::render(World * p_world)
{
	if(death()) return;
	//--‰~‚ð•`‰æ--
	glBegin(GL_POLYGON);
	glColor4f(1.0, 0, 0, 0.9);
	const int N = 100;
	for(int i = 0; i < N; i++)
	{
		auto x = pos.x + 0.3 * life * cos(2.0 * 3.141592 * ((double)i / N));
		auto y = pos.y + 0.3 * life * sin(2.0 * 3.141592 * ((double)i / N));
		glVertex3f(x, y, 0.0);
	}
	glEnd();
}
