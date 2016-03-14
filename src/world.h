
#ifndef WORLD_H
#define WORLD_H

#include <vector>
//#include <list>
#include <map>
#include <memory>
#include <GL/glut.h>
#include "vec2d.h"

//--プロトタイプ宣言--
class Fish;
class Shark;
class Target;
class MyShader;
class TexImage;
class FishLoader;
class WaterSurface;


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
	
	//マウス用
	vec2d	mouse_pos;
	bool	mouse_state = false; //押されているかどうか

	//ステップカウンタ
	unsigned int step = 0;

	//魚ローダー
	std::unique_ptr<FishLoader>	fish_loader;

	//フレームバッファ
	GLuint		frame_buf;
	//水中テクスチャ
	GLuint		underwater_tex;

	//水面
	std::unique_ptr<WaterSurface> water_surface;

public:
	World(int w, int h);
	~World();

	void update();
	void opengl_update(); //OPENGL関連の処理
	void render();

	//エンティティ(魚・ターゲット)のアップデート
	void update_entities();

	//アクセサ
	std::vector<Shark*>& get_sharks(){ return sharks; }
	std::vector<Target*>& get_targets(){ return targets; }
	//分割された領域周辺にいるfishの配列を得る
	std::vector<Fish*> get_neighborfishes(int idx);
	int get_width(){ return width; }
	int get_height(){ return height; }
	void	set_mouse_pos(vec2d mp){ mouse_pos = mp; }
	void	set_mouse_state(bool ms){ mouse_state = ms; }
	void	add_fish_to_world(Fish* fish);
};


#endif

