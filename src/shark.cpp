﻿
#include "shark.h"
#include "teximage.h"
#include "world.h"
#include "behavior.h"
#include <list>
#include <stdio.h>
#include <stdlib.h>

//近傍の限界距離
const int RADIUS = 50;

Shark::Shark(vec2d pos_, vec2d velo_, GLuint shader)
	: Fish(pos_, velo_, NULL, 0.9)     //最高速度を遅めに設定
{
	//static TexImageWithShader img("shark.png", 19, 37, shader);
	static TexImageWithShader img("newshark.jpg", 26, 60, shader);
	tex = &img;
}

void Shark::update(World* p_world)
{
	//最近傍のfishを探す
	Fish* nearest_fish = NULL;
	auto lfishes = p_world->get_neighborfishes(partidx);
	for (auto& lfish : lfishes)
	{
		double lsq = (pos - lfish->get_pos()).lengthsq();
		if (lsq < RADIUS*RADIUS)
		{
			if (!nearest_fish)
			{
				nearest_fish = lfish;
				continue;
			}
			if (lsq < (nearest_fish->get_pos() - pos).lengthsq())
			{
				nearest_fish = lfish;
			}
		}
	}

	//追従行動および放浪行動を行う
	vec2d force;
	double fmax = 6.65;
	for (;;)
	{
		if (nearest_fish)
			force += behavior->seek(this, p_world, nearest_fish);
		if (force.lengthsq() > fmax*fmax) break;
		force += behavior->randomwalk(this, p_world);
		break;
	}

	if (force.lengthsq() > fmax*fmax) force = force.norm()*fmax;

	double M = 1.80;

	velo += force / M;

	if (velo.lengthsq() > 0.001)
		dire = velo.norm();

	if (velo.lengthsq() > max_speed*max_speed)
		velo = velo.norm()*max_speed;

	pos += velo;

	if (pos.x < 0) pos.x += p_world->get_width();
	if (pos.x > p_world->get_width()) pos.x -= p_world->get_width();
	if (pos.y < 0) pos.y += p_world->get_height();
	if (pos.y > p_world->get_height()) pos.y -= p_world->get_height();
}
