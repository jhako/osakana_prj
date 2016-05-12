

#include <vector>
#include "vec2d.h"

class Fish;
class World;
class Noise;

//--Fishの振る舞いを計算するクラス--
class Behavior
{
	//randomwalkにおける円周上の点の以前の位置
	double rw_rad;

public:
	Behavior() : rw_rad(0.0){}

	//分離行動
	vec2d separation(Fish* self, World* p_world, std::vector<Fish*>& neighbors);
	//整列行動
	vec2d alignment(Fish* self, World* p_world, std::vector<Fish*>& neighbors);
	//結合行動
	vec2d cohesion(Fish* self, World* p_world, std::vector<Fish*>& neighbors);
	//追従行動
	vec2d seek(Fish* self, World* p_world, Fish* target);
	//逃避行動
	vec2d flee(Fish* self, World* p_world, Fish* enemy);
	//放浪行動
	vec2d randomwalk(Fish* self, World* p_world);
	//到着行動
	vec2d arrive(Fish* self, World* p_world, vec2d target_pos);
	vec2d arrive(Fish * self, vec2d aj_pos, vec2d target_pos);
	//ノイズ対比
	vec2d flee_from_noise(Fish* self, World* p_world, std::vector<Noise*>& neighbors);
};
