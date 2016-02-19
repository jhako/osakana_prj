

#ifndef TARGET_H
#define TARGET_H

#include "vec2d.h"

class World;

class Target
{
	vec2d pos;

public:
	Target(vec2d pos_)
		:pos(pos_)
	{}

	void update(World* p_world);

	void render(World* p_world);

	vec2d get_pos(){ return pos; }
};

#endif
