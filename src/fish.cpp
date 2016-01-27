
#include "fish.h"
#include "teximage.h"
#include "world.h"
#include "behavior.h"
#include "shark.h"
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

//近傍とみなす限界距離
const int RADIUS = 50;

void Fish::init(vec2d pos_, vec2d velo_)
{
    pos = pos_; velo = velo_; dire = vec2d(1.0,1.0);
    behavior = new Behavior();
    static int staticID = 0;
    id = staticID++;
    partidx = -99;
}

Fish::Fish(vec2d pos_, vec2d velo_)
    : max_speed(2.3)
{
    init(pos_, velo_);
    static TexImage img("osakana.png");
    tex = &img;
}

Fish::Fish(vec2d pos_, vec2d velo_, TexImage* tex_, double maxspd)
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
    std::list<Fish*> neighbors;
    std::vector<Fish*> lfish = p_world->get_neighborfishes(partidx);
    for(int i = 0; i < lfish.size(); ++i)
    {
	double lsq = (pos - lfish.at(i)->get_pos()).lengthsq();

	if(lsq < 0.0001)
	    continue; //自分自身
	if(lsq < RADIUS*RADIUS)
	{
	    neighbors.push_back(lfish.at(i));
	}
    }

    //最も近傍にいる敵を探す
    Fish* nearest_enemy = NULL;
    std::vector<Shark*>& lshark = p_world->get_sharks();
    for(int i = 0; i < lshark.size(); ++i)
    {
	double lsq = (pos - lshark.at(i)->get_pos()).lengthsq();
	if(lsq < RADIUS*RADIUS)
	{
	    if(!nearest_enemy)
	    {
	  	nearest_enemy = lshark.at(i);
		continue;
	    }
	    if(lsq < (nearest_enemy->get_pos() - pos).lengthsq())
	    {
		nearest_enemy = lshark.at(i);
	    }
	}
    }

    vec2d force;
    double fmax = 6.65;

    //敵からの逃避行動、分離・整列・結合行動（群れ）、放浪行動を行う
    //上から順に優先度が高く、作用する力がfmaxを超えた時点で計算終了
    for(;;)
    {
	if(nearest_enemy)
	    force += behavior->flee(this, p_world, nearest_enemy);
	if(force.lengthsq() > fmax*fmax) break;
	force += behavior->separation(this, p_world, neighbors);
	if(force.lengthsq() > fmax*fmax) break;
	force += behavior->alignment(this, p_world, neighbors);
	if(force.lengthsq() > fmax*fmax) break;
	force += behavior->cohesion(this, p_world, neighbors);
	if(force.lengthsq() > fmax*fmax) break;
	force += behavior->randomwalk(this, p_world);
	break;
    }

    //作用する力をfmaxに丸める
    if(force.lengthsq() > fmax*fmax) force = force.norm()*fmax;

    double M = 1.20;    //魚の質量

    velo += force / M;

    //向きを更新
    if(velo.lengthsq() > 0.001)
	dire = velo.norm();

    //速度を最大速度で丸める
    if(velo.lengthsq() > max_speed*max_speed)
	velo = velo.norm()*max_speed;

    //位置の更新
    pos += velo;
  
    //端はない（上下左右端で接続されている）
    if(pos.x < 0)
	pos.x += p_world->get_width(); 
    if(pos.x > p_world->get_width())
	pos.x -= p_world->get_width();
    if(pos.y < 0)
	pos.y += p_world->get_height();
    if(pos.y > p_world->get_height())
	pos.y -= p_world->get_height();
}


void Fish::render(World* p_world)
{
    //向きに合わせて画像を回転させ、描画
    tex->render(pos.x, pos.y,
		atan2(dire.y, dire.x)+3.14159265358979323846/2.0);
}
