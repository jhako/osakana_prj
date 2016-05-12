


#include "behavior.h"
#include "world.h"
#include "fish.h"
#include "noise.h"

//重みづけ係数
const double KS = 0.34;  //分離
const double KA = 0.025; //整列
const double KC = 0.06;  //結合
const double KR = 0.1;   //放浪
const double KSe = 0.3;  //追従
const double KF = 0.3;   //逃避
const double KArr = 0.05;   //到着
const double KFN = 0.3; //ノイズ逃避

vec2d Behavior::separation(Fish* self, World* p_world, std::vector<Fish*>& neighbors)
{
	if (neighbors.size() == 0)
		return vec2d();

	//近傍の各対象に対し、距離に反比例する力を計算
	vec2d force;
	for (auto& neighbor : neighbors)
	{
		vec2d tovec = self->get_pos() - neighbor->get_pos();
		force += tovec.norm() / tovec.length();
	}
	return force*KS;
}



vec2d Behavior::alignment(Fish* self, World* p_world, std::vector<Fish*>& neighbors)
{
	if (neighbors.size() == 0) return vec2d();

	//近傍の全対象の向きの平均を計算し、それに合わせるような力を計算
	vec2d ave;
	for(auto& neighbor : neighbors)
	{
		ave += neighbor->get_dire();
	}
	ave /= neighbors.size();
	return (ave - self->get_dire())*KA;

}

vec2d Behavior::cohesion(Fish* self, World* p_world, std::vector<Fish*>& neighbors)
{
	if (neighbors.size() == 0) return vec2d();

	//近傍全対象の中心を計算し、そこに向かう力を与える
	vec2d center;
	int num = 0;
	for(auto& neighbor : neighbors)
	{
		center += neighbor->get_pos();
		++num;
	}
	center /= num;
	return (center - self->get_pos()).norm()*self->get_maxspeed()*KC;
}

vec2d Behavior::randomwalk(Fish* self, World* p_world)
{
	//前方に半径rの円を仮定し、その円上にあるランダムな点に
	//向かう力を計算する（振動をさけ、滑らかな動きを実現するため）
	vec2d center = self->get_pos() + self->get_dire() * 0.5;
	double r = 0.2;
	double rd = 0.4;
	rw_rad += (double)(rand()) / RAND_MAX * rd - rd / 2.0;
	return (center + vec2d(0, r).rotate(rw_rad) - self->get_pos())*KR;
}

vec2d Behavior::seek(Fish* self, World* p_world, Fish* target)
{
	//対象の移動先を計算し、そこに向かうような力を与える
	//ただし、滑らかな回転を行うように移動先を考慮する（係数lookahead）
	vec2d totarget = target->get_pos() - self->get_pos();
	double lookahead = totarget.length() / (self->get_maxspeed() + target->get_velocity().length());
	vec2d tovec = target->get_pos() + target->get_velocity() * lookahead;
	return (tovec - self->get_pos()).norm()*self->get_maxspeed() * KSe;
}

vec2d Behavior::flee(Fish* self, World* p_world, Fish* enemy)
{
	//対象から逃避するような力を計算（Behavior::seekと同様の原理）
	vec2d toenemy = enemy->get_pos() - self->get_pos();
	double lookahead = toenemy.length() / (self->get_maxspeed() + enemy->get_velocity().length());
	vec2d tovec = enemy->get_pos() + enemy->get_velocity() * lookahead;
	return (self->get_pos() - enemy->get_pos()).norm()*self->get_maxspeed() * KF;
}

vec2d Behavior::arrive(Fish * self, World * p_world, vec2d target_pos)
{
	//減速をはじめる半径
	constexpr auto SlowingRadius = 30;
	vec2d tovec = target_pos - self->get_pos();
	auto length = tovec.length();
	if (length < SlowingRadius)
	{
		return tovec.norm() * self->get_maxspeed() * (length / SlowingRadius) * KArr;
	}
	else
	{
		return tovec.norm() * self->get_maxspeed() * KArr;
	}
}

vec2d Behavior::arrive(Fish * self, vec2d aj_pos, vec2d target_pos)
{
	constexpr auto SlowingRadius = 10;
	vec2d tovec = target_pos - aj_pos;
	auto length = tovec.length();
	if(length < SlowingRadius)
	{
		return tovec.norm() * self->get_maxspeed() * (length / SlowingRadius) * KArr;
	}
	else
	{
		return tovec.norm() * self->get_maxspeed() * KArr;
	}
}


vec2d Behavior::flee_from_noise(Fish* self, World* p_world, std::vector<Noise*>& neighbors)
{
	if(neighbors.empty()) return vec2d();

	//近傍の各対象に対し、距離に反比例する力を計算
	vec2d force;
	for(auto& neighbor : neighbors)
	{
		vec2d tovec = self->get_pos() - neighbor->get_pos();
		force += tovec.norm() / tovec.length();
	}
	return force.norm() * self->get_maxspeed() * KFN;
}

