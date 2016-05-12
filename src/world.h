
#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <array>
#include <map>
#include <memory>
#include <deque>
#include <GLFW/glfw3.h>
#include "vec2d.h"

//--プロトタイプ宣言--
class Fish;
class Shark;
class TexImage;
class FishLoader;
class WaterSurface;
class IPCom;
class MyShader;
class Food;
class Noise;


//--魚等の管理クラス--
class World
{
	//画面の幅・高さ
	int width, height;

	//魚を保管するvector
	std::vector<Fish*> fishes;
	std::vector<Shark*> sharks;
	//領域の分割（動作負荷軽減のため）
	std::vector<std::map<int, Fish*>>  partitons;

	//partitions
	const int	parti_w = 50, //分割領域の大きさ
				parti_h = 50;
	const int	parti_nx, parti_ny; //分割数（大きさにより決定）

	//エサ
	std::deque<Food*> foods;
	std::vector<std::map<int, Food*>>  parti_foods;

	//騒音
	std::deque<Noise*> noises;
	std::vector<std::map<int, Noise*>>  parti_noises;
	
	//マウス用
	vec2d	mouse_pos;
	bool	mouse_state = false; //押されているかどうか
	bool	mouse_state_right = false; //押されているかどうか
	bool	click_right = false; //レフトクリック

	//タッチ用
	std::array<vec2d, 10>	touch_pos; //正規化座標
	int		touch_maxidx = 0;

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
	
	//通信用クラス
	std::unique_ptr<IPCom> ip_com;

	//描画用シェーダ
	std::unique_ptr<MyShader> simple_shd;

	//デバッグモード
	bool debug_mode = true;

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
	//std::vector<Target*>& get_targets(){ return targets; }
	//分割された領域周辺にいるfishの配列を得る
	std::vector<Fish*> get_neighborfishes(int idx);
	std::vector<Food*> get_neighborfoods(int idx);
	std::vector<Noise*> get_neighbornoises(int idx);
	int get_width(){ return width; }
	int get_height(){ return height; }
	void	set_mouse_pos(vec2d mp){ mouse_pos = mp; }
	void	set_mouse_state(bool ms){ mouse_state = ms; }
	void	set_click_state_right(bool ms){ click_right = ms; }
	void	add_fish_to_world(Fish* fish);
	void	add_food_to_world(Food* food);
	void	add_noise_to_world(Noise * food);
	void	set_touch_data(std::array<vec2d, 10>& pos_arr, int midx){ touch_pos = pos_arr; touch_maxidx = midx; }
	GLuint	get_shader();
};


#endif

