
#include "fish.h"
#include "vec2d.h"

class World;

//--Sharkクラス（Fishの敵）：Fishクラスを継承
class Shark :public Fish
{
public:
	Shark(vec2d pos_, vec2d velo_);

	//update関数のオーバーライド
	void update(World* p_world);
};
