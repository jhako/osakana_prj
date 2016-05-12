

#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <random>
#include "food.h"
#include "world.h"


static std::random_device rd;
static std::mt19937 mt(rd());

Food::Food(vec2d pos_)
	:pos(pos_)
{
	static int unique_id = 0;
	id = unique_id++;
	rad = (double)mt() / std::mt19937::max() * 2.0 * 3.141592;
}

void Food::update(World * p_world)
{
	//ƒ‰ƒ“ƒ_ƒ€‚È‰^“®
	if(life > 0)
	{
		const double D = 0.5;
		const double K = 0.5;
		rad += ((double)mt() / std::mt19937::max() - 0.5) * D * 2.0;
		pos.x += K * cos(rad);
		pos.y += K * sin(rad);
		if(pos.x < 0)
			pos.x += p_world->get_width();
		if(pos.x > p_world->get_width())
			pos.x -= p_world->get_width();
		if(pos.y < 0)
			pos.y += p_world->get_height();
		if(pos.y > p_world->get_height())
			pos.y -= p_world->get_height();
		life -= 1;
	}
}

void Food::render(World * p_world)
{
	if(death()) return;
	//--‰~‚ð•`‰æ--
	glBegin(GL_POLYGON);
	glColor3f(1.0, 0, 0);
	const int R = 3, N = 100;
	for(int i = 0; i < N; i++)
	{
		auto x = pos.x + R * cos(2.0 * 3.141592 * ((double)i / N));
		auto y = pos.y + R * sin(2.0 * 3.141592 * ((double)i / N));
		glVertex3f(x, y, 0.0);
	}
	glEnd();
}
