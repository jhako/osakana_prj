
#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <map>

//--プロトタイプ宣言--
class Fish;
class Shark;
class Target;


//--魚等の管理クラス--
class World
{
	//画面の幅・高さ
	int width, height;

	//魚を保管するvector
	std::vector<Fish*> fishes;
	std::vector<Shark*> sharks;
	//領域の分割（動作負荷軽減のため）
	std::vector< std::map<int, Fish*> >  partitons;

	//魚が集まる場所
	std::vector<Target*> targets;

public:
	World();
	~World();

	void update();
	void render();

	//アクセサ
	std::vector<Shark*>& get_sharks(){ return sharks; }
	std::vector<Target*>& get_targets(){ return targets; }
	//分割された領域周辺にいるfishの配列を得る
	std::vector<Fish*> get_neighborfishes(int idx);
	int get_width(){ return width; }
	int get_height(){ return height; }
};


#endif

