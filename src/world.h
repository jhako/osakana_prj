
#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <list>
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
class MyShader;


//--魚等の管理クラス--
class World
{
	//画面の幅・高さ
	int width, height;

	//魚を保管するvector
	std::list<Fish*> fishes;
	std::list<Shark*> sharks;
	//領域の分割（動作負荷軽減のため）
	std::vector< std::map<int, Fish*> >  partitons;

	//魚が集まる場所
	std::list<Target*> targets;

	//水面を表現する頂点データ（z値）
	std::vector<std::vector<double>>		surf_vertex_z[3];

	//シェーダ
	std::unique_ptr<MyShader>	water_shader;
	std::unique_ptr<MyShader>	caustics_shader;

	//キューブマッピング用テクスチャ
	GLuint		cube_tex;
	//フレームバッファ
	GLuint		frame_buf;
	//水中バッファ記録用
	std::vector<unsigned char>	buffer_data;
	GLuint		buf_tex;
	//法線マップ
	std::vector<unsigned char>	normal_data;
	GLuint		normal_map;
	//コースティクスマップ
	GLuint	caustics_map;

	//マウス用
	vec2d	mouse_pos;
	bool	mouse_state = false; //押されているかどうか

public:
	World(int w, int h);
	~World();

	void update();
	void render();

	void draw_water_surface();

	//アクセサ
	std::list<Shark*>& get_sharks(){ return sharks; }
	std::list<Target*>& get_targets(){ return targets; }
	//分割された領域周辺にいるfishの配列を得る
	std::list<Fish*> get_neighborfishes(int idx);
	int get_width(){ return width; }
	int get_height(){ return height; }
	void	set_mouse_pos(vec2d mp){ mouse_pos = mp; }
	void	set_mouse_state(bool ms){ mouse_state = ms; }
};


#endif

