

#pragma once

#include "vec2d.h"

class World;

class Noise
{
	vec2d pos;
	int life = 60 * 3;
	int id;
	int partidx;

public:
	Noise(vec2d pos_);

	void update(World* p_world);

	void render(World* p_world);

	void kill(){ life = -1; }

	vec2d get_pos(){ return pos; }
	int get_id(){ return id; }
	int get_pidx(){ return partidx; }
	void set_pidx(int idx){ partidx = idx; }

	bool death(){ return life < 0; }
};