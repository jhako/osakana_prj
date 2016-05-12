

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include "fish.h"
#include "teximage.h"
#include "world.h"
#include "behavior.h"
#include "shark.h"
#include "food.h"
#include "noise.h"

//近傍とみなす限界距離
const int RadiusForNeighbors = 50;
const int RadiusForEnemies = 70;
const int RadiusForTargets = 90;
const int RadiusForNoises = 90;

//最大F
const double force_max = 6.65;
const double M = 1.20;    //魚の質量

void Fish::init(vec2d pos_, vec2d velo_)
{
	pos = pos_; velo = velo_; dire = vec2d(1.0, 1.0);
	behavior = new Behavior();
	static int staticID = 0;
	id = staticID++;
	partidx = -99;
}

Fish::Fish(vec2d pos_, vec2d velo_, GLuint shader)
	: max_speed(1.3)
{
	init(pos_, velo_);
	//static TexImageWithShader img("osakana.png", 8, 20, shader);
	static TexImageWithShader img("newosakana.jpg", 22, 50, shader);
	tex = &img;
}

Fish::Fish(vec2d pos_, vec2d velo_, TexImageWithShader* tex_, double maxspd)
	: tex(tex_), max_speed(maxspd)
{
	init(pos_, velo_);
}

Fish::~Fish()
{
	delete behavior;
}

void Fish::update(World* p_world)
{
	//近傍にいる魚を調べ、neighborsに追加する
	std::vector<Fish*> neighbors;
	neighbors.reserve(100);
	auto lfishes = p_world->get_neighborfishes(partidx);
	for (auto& lfish : lfishes)
	{
		double lsq = (pos - lfish->get_pos()).lengthsq();

		if (lsq < 0.0001)
			continue; //自分自身
		if (lsq < RadiusForNeighbors*RadiusForNeighbors)
		{
			neighbors.push_back(lfish);
		}
	}

	//最も近傍にいる敵を探す
	Fish* nearest_enemy = NULL;
	auto& lsharks = p_world->get_sharks();
	for (auto& lshark : lsharks)
	{
		double lsq = (pos - lshark->get_pos()).lengthsq();
		if (lsq < RadiusForEnemies*RadiusForEnemies)
		{
			if (!nearest_enemy)
			{
				nearest_enemy = lshark;
				continue;
			}
			if (lsq < (nearest_enemy->get_pos() - pos).lengthsq())
			{
				nearest_enemy = lshark;
			}
		}
	}

	//最も近傍のターゲットを探す
	Food* nearest_food = NULL;
	auto& lfoods = p_world->get_neighborfoods(partidx);
	for(auto& food : lfoods)
	{
		double lsq = (pos - food->get_pos()).lengthsq();
		if (lsq < RadiusForTargets*RadiusForTargets)
		{
			if (!nearest_food)
			{
				nearest_food = food;
				continue;
			}
			if (lsq < (nearest_food->get_pos() - pos).lengthsq())
			{
				nearest_food = food;
			}
		}
	}
	//食べる
	if(nearest_food && nearest_food->eatable())
	{
		if((pos + dire * tex->get_h() / 2.1 - nearest_food->get_pos()).lengthsq() < 9.0)
		{
			nearest_food->kill();
		}
	}

	//近くの騒音を調べる
	std::vector<Noise*> nb_noises;
	neighbors.reserve(32);
	auto lnoises = p_world->get_neighbornoises(partidx);
	for(auto& lnoise : lnoises)
	{
		double lsq = (pos - lnoise->get_pos()).lengthsq();
		if(lsq < RadiusForNoises*RadiusForNoises)
		{
			nb_noises.push_back(lnoise);
		}
	}


	vec2d force;

	//敵からの逃避行動、分離・整列・結合行動（群れ）、放浪行動を行う
	//上から順に優先度が高く、作用する力がforce_maxを超えた時点で計算終了
	for (;;)
	{
		force += behavior->flee_from_noise(this, p_world, nb_noises);
		if(force.lengthsq() > force_max*force_max) break;
		if (nearest_enemy)
		{
			force += behavior->flee(this, p_world, nearest_enemy);
		}
		if (force.lengthsq() > force_max*force_max) break;
		force += behavior->separation(this, p_world, neighbors);
		if (force.lengthsq() > force_max*force_max) break;
		force += behavior->alignment(this, p_world, neighbors);
		if (force.lengthsq() > force_max*force_max) break;
		force += behavior->cohesion(this, p_world, neighbors);
		if(force.lengthsq() > force_max*force_max) break;
		if(nearest_food)
		{
			//force += behavior->arrive(this, p_world, nearest_food->get_pos());
			force += behavior->arrive(this, pos + dire * tex->get_h() / 2.1, nearest_food->get_pos());
			if(force.lengthsq() > force_max*force_max) break;
		}
		//else
		{
			force += behavior->randomwalk(this, p_world);
			if(force.lengthsq() > force_max*force_max) break;
		}
		break;
	}

	//作用する力をforce_maxに丸める
	if (force.lengthsq() > force_max*force_max) force = force.norm()*force_max;


	velo += force / M;

	//向きを更新
	if (velo.lengthsq() > 0.001)
		dire = velo.norm();

	//速度を最大速度で丸める
	if (velo.lengthsq() > max_speed*max_speed)
		velo = velo.norm()*max_speed;

	//位置の更新
	pos += velo;

	//端はない（上下左右端で接続されている）
	if (pos.x < 0)
		pos.x += p_world->get_width();
	if (pos.x > p_world->get_width())
		pos.x -= p_world->get_width();
	if (pos.y < 0)
		pos.y += p_world->get_height();
	if (pos.y > p_world->get_height())
		pos.y -= p_world->get_height();
}


void Fish::render(World* p_world)
{
	//向きに合わせて画像を回転させ、描画
	tex->render(pos.x, pos.y,
		atan2(dire.y, dire.x) + 3.14159265358979323846 / 2.0);

	/*
	glBegin(GL_POLYGON);
	glColor3f(0.0, 0, 1.0);
	const int R = 3, N = 100;
	for(int i = 0; i < N; i++)
	{
		auto x = pos.x + R * cos(2.0 * 3.141592 * ((double)i / N));
		auto y = pos.y + R * sin(2.0 * 3.141592 * ((double)i / N));
		glVertex3f(x, y, 0.0);
	}
	glEnd();
	vec2d p = pos + dire * tex->get_h() / 2.1;
	glBegin(GL_POLYGON);
	glColor3f(0.0, 1.0, 0);
	for(int i = 0; i < N; i++)
	{
		auto x = p.x + R * cos(2.0 * 3.141592 * ((double)i / N));
		auto y = p.y + R * sin(2.0 * 3.141592 * ((double)i / N));
		glVertex3f(x, y, 0.0);
	}
	glEnd();
	*/
}
