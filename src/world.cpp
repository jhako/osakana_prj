


#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <future>
#include "world.h"
#include "teximage.h"
#include "fish.h"
#include "shark.h"
#include "shader.h"
#include "vec3d.h"
#include "fish_loader.h"
#include "water_surface.h"
#include "ipcom.h"
#include "food.h"
#include "noise.h"


static TexImage bottom_img;
const int NUM_MAX_FOODS = 5;
const int NUM_MAX_NOISES = 100;


World::World(int w, int h): 
	width(w), height(h),
	parti_nx((w + parti_w - 1) / parti_w),
	parti_ny((h + parti_h - 1) / parti_h)
{
	//メモリ確保
	partitons.resize(parti_nx * parti_ny);
	parti_foods.resize(parti_nx * parti_ny);
	parti_noises.resize(parti_nx * parti_ny);
	fishes.reserve(500);

	//描画用シェーダ
	simple_shd = std::make_unique<MyShader>("shader/simple_shd.vs", "shader/simple_shd.fs");

	//fishの追加
	for (int i = 0; i < 200; ++i)
	{
		//位置はランダム、初期速度はゼロ
		Fish* fish = new Fish(vec2d(100 + (double)rand() / RAND_MAX * (width - 200),
			100 + (double)rand() / RAND_MAX * (height - 200)),
			vec2d(), simple_shd->get_prog());
		add_fish_to_world(fish);
	}
	
	/*
	//三匹sharkを追加
	sharks.push_back(new Shark(vec2d(100, 100), vec2d(0, 0), simple_shd->get_prog()));
	sharks.push_back(new Shark(vec2d(250, 250), vec2d(0, 0), simple_shd->get_prog()));
	sharks.push_back(new Shark(vec2d(450, 450), vec2d(0, 0), simple_shd->get_prog()));
	*/

	//ダミーフード
	for(int i = 0; i < NUM_MAX_FOODS; ++i)
	{
		foods.push_back(new Food(vec2d(120 + i * 30, 100)));
		foods.back()->kill(); //dummy
		foods.back()->set_pidx(-1);
	}
	//add_food_to_world(new Food(vec2d(120, 100)));
	//add_food_to_world(new Food(vec2d(270, 200)));
	//add_food_to_world(new Food(vec2d(320, 300)));
	//add_food_to_world(new Food(vec2d(420, 400)));

	//ダミーノイズ
	for(int i = 0; i < NUM_MAX_NOISES; ++i)
	{
		noises.push_back(new Noise(vec2d(120 + i * 30, 100)));
		noises.back()->kill(); //dummy
		noises.back()->set_pidx(-1);
	}

	//海底イメージ
	bottom_img.load("newbottom.png");

	//魚ローダー
	fish_loader = std::make_unique<FishLoader>("newfish", this);

	//水中バッファテクスチャの割り当て
	glGenTextures(1, &underwater_tex);
	glBindTexture(GL_TEXTURE_2D, underwater_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	//フレームバッファの作成
	glGenFramebuffersEXT(1, &frame_buf);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buf);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, underwater_tex, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	//water_surface = std::make_unique<WaterSurfaceCPU>(width, height);   //CPU上の計算
	water_surface = std::make_unique<WaterSurfaceGPU_SWE>(width, height); //GPU上の計算

	ip_com = std::make_unique<IPCom>(width, height, 400, 400);
}

World::~World()
{
	for (int i = 0; i < fishes.size(); ++i)
	{
		Fish* fish = fishes.back();
		delete fish;
		fishes.pop_back();
	}
	for (int i = 0; i < sharks.size(); ++i)
	{
		Shark* shark = sharks.back();
		delete shark;
		sharks.pop_back();
	}
	for(int i = 0; i < foods.size(); ++i)
	{
		Food* food = foods.back();
		delete food;
		foods.pop_back();
	}
	for(int i = 0; i < noises.size(); ++i)
	{
		Food* food = foods.back();
		delete food;
		foods.pop_back();
	}
}

void World::update()
{
	//OPENGL関連の処理は禁止

//	//for debug
//	auto start_t = std::chrono::system_clock::now();

	//ロード間隔
	constexpr int FramesPerFishLoading = 300; //大体5秒
	//タスク（並列あるいは遅延処理）
	std::future<std::vector<Fish*>> fut_load;

	//ロードするかのフラグ
	bool if_load_fish = step % FramesPerFishLoading == 0;

	//エンティティのアップデート
	update_entities();
	//auto fut_ue = std::async([&]{ update_entities(); });

	//水面サーフェスの計算
	water_surface->update();
	//calc_water_surface();
//	auto fut_cw = std::async([&]{ calc_water_surface(); }); //並列化


//	fut_ue.get();
//	fut_cw.get();

	/*
	if(if_load_fish)
	{
		//タスクの終了を待機
		//auto additional_fishes = fut_load.get();
		auto additional_fishes = fish_loader->load_fish();
		//魚の追加
		while(!additional_fishes.empty())
		{
			add_fish_to_world(additional_fishes.back());
			additional_fishes.pop_back();
		}
	}
	*/

	//ステップの更新
	step += 1;

	//ノイズの追加
	{
		std::lock_guard<std::mutex> lock(world_mtx);
		while(!future_noise_list.empty())
		{
			add_noise_to_world(new Noise(future_noise_list.back()));
			future_noise_list.pop_back();
		}
	}

//	//for debug
//	auto end_t = std::chrono::system_clock::now();
//	std::cout << "TS : " << std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count() << "\n";
}

void World::opengl_update()
{
	//波を作る
	if(mouse_state == true)
	{
		water_surface->mouse_action(mouse_pos.x, mouse_pos.y);
	}

	//タッチで波を作る
	for(int i = 0; i < touch_maxidx; ++i)
	{
		double tx = touch_pos[i].x * width;
		double ty = (1.0 - touch_pos[i].y) * height;
		water_surface->mouse_action(tx, ty);
	}

	if(click_right)
	{
		//add_food_to_world(new Food(vec2d(mouse_pos.x, height - mouse_pos.y)));
		add_noise_to_world(new Noise(vec2d(mouse_pos.x, height - mouse_pos.y)));
		click_right = false;
	}
}

void World::update_entities()
{
	for(auto& food : foods)
	{
		//パーティションの更新
		vec2d pos = food->get_pos();
		int pre_idx = food->get_pidx();
		int idx = (int)(pos.x / parti_w) + (int)(pos.y / parti_h) * parti_nx;
		if(food->death())
		{
			if(pre_idx != -1)
			{
				parti_foods.at(pre_idx).erase(parti_foods.at(pre_idx).find(food->get_id()));
				food->set_pidx(-1);
			}
			continue;
		}
		if(pre_idx != idx)
		{
			parti_foods.at(pre_idx).erase(parti_foods.at(pre_idx).find(food->get_id()));
			parti_foods.at(idx).insert(std::make_pair(food->get_id(), food));
			food->set_pidx(idx);
		}

		food->update(this);
	}

	for(auto& noise : noises)
	{
		//パーティションの更新
		vec2d pos = noise->get_pos();
		int pre_idx = noise->get_pidx();
		int idx = (int)(pos.x / parti_w) + (int)(pos.y / parti_h) * parti_nx;
		if(noise->death())
		{
			if(pre_idx != -1)
			{
				parti_noises.at(pre_idx).erase(parti_noises.at(pre_idx).find(noise->get_id()));
				noise->set_pidx(-1);
			}
			continue;
		}
		if(pre_idx != idx)
		{
			parti_noises.at(pre_idx).erase(parti_noises.at(pre_idx).find(noise->get_id()));
			parti_noises.at(idx).insert(std::make_pair(noise->get_id(), noise));
			noise->set_pidx(idx);
		}

		noise->update(this);
	}

	for(auto& fish : fishes)
	{
		//各fishがどこの領域に属するかを更新
		vec2d pos = fish->get_pos();
		int pre_idx = fish->get_pidx();
		int idx = (int)(pos.x / parti_w) + (int)(pos.y / parti_h) * parti_nx;
		if(pre_idx != idx)
		{
			partitons.at(pre_idx).erase(partitons.at(pre_idx).find(fish->get_id()));
			partitons.at(idx).insert(std::make_pair(fish->get_id(), fish));
			fish->set_pidx(idx);
		}

		fish->update(this);
	}
	for(auto& shark : sharks)
	{
		int idx = (int)(shark->get_pos().x / parti_w) + (int)(shark->get_pos().y / parti_h) * parti_ny;
		shark->set_pidx(idx);

		shark->update(this);
	}
}


void World::render()
{
	glClearColor(240.0 / 255, 248.0 / 255, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2D視点
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0, height, -1000, 1000);
	//glOrtho(0.0, 100, 0, 100, -1000, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, width, height);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);


	// 水底の描画
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buf);
	const auto fbufs = std::vector<GLenum>{
		GL_COLOR_ATTACHMENT0_EXT
	};
	glDrawBuffers(fbufs.size(), &fbufs[0]);

	bottom_img.render(0, 0, width, height);

	// ターゲット, 魚の描画
	for(auto& food : foods)
	{
		food->render(this);
	}
	for(auto& fish : fishes)
	{
		fish->render(this);
	}
	for(auto& shark : sharks)
	{
		shark->render(this);
	}

	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	for(auto& noise : noises)
	{
		noise->render(this);
	}
	glDisable(GL_BLEND);

	glFlush();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glUseProgram(0);

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);

//	glClearColor(0.5, 0.5, 0.5, 1.0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	water_surface->render(underwater_tex);

	ip_com->main_update(this);
}

std::vector<Fish*> World::get_neighborfishes(int idx)
{
	//idxの領域の周辺８つの領域を含めて、属するfishを返す
	std::vector<Fish*> nfishes;
	nfishes.reserve(100);
	int x = idx % parti_nx;
	int y = idx / parti_nx;
	for (int i = x - 1; i <= x + 1; ++i)
	{
		for (int j = y - 1; j <= y + 1; ++j)
		{
			if (i < 0 || i >= parti_nx || j < 0 || j >= parti_ny)
				continue;
			int nidx = i + j * parti_nx;
			for (auto it = partitons.at(nidx).begin(); it != partitons.at(nidx).end(); ++it)
			{
				nfishes.push_back(it->second);
			}
		}
	}
	return nfishes; //NRVO
}


std::vector<Food*> World::get_neighborfoods(int idx)
{
	//idxの領域の周辺８つの領域を含めて、属するfoodsを返す
	std::vector<Food*> nfoods;
	nfoods.reserve(100);
	int x = idx % parti_nx;
	int y = idx / parti_nx;
	for(int i = x - 1; i <= x + 1; ++i)
	{
		for(int j = y - 1; j <= y + 1; ++j)
		{
			if(i < 0 || i >= parti_nx || j < 0 || j >= parti_ny)
				continue;
			int nidx = i + j * parti_nx;
			for(auto it = parti_foods.at(nidx).begin(); it != parti_foods.at(nidx).end(); ++it)
			{
				nfoods.push_back(it->second);
			}
		}
	}
	return nfoods; //NRVO
}

std::vector<Noise*> World::get_neighbornoises(int idx)
{
	//idxの領域の周辺８つの領域を含めて、属するfoodsを返す
	std::vector<Noise*> nnoises;
	nnoises.reserve(100);
	int x = idx % parti_nx;
	int y = idx / parti_nx;
	for(int i = x - 1; i <= x + 1; ++i)
	{
		for(int j = y - 1; j <= y + 1; ++j)
		{
			if(i < 0 || i >= parti_nx || j < 0 || j >= parti_ny)
				continue;
			int nidx = i + j * parti_nx;
			for(auto it = parti_noises.at(nidx).begin(); it != parti_noises.at(nidx).end(); ++it)
			{
				nnoises.push_back(it->second);
			}
		}
	}
	return nnoises; //NRVO
}


void	World::add_fish_to_world(Fish* fish)
{
	//リストに追加
	fishes.push_back(fish);

	//どこの領域にいるかを計算・更新
	int idx = (int)(fish->get_pos().x / parti_w) + (int)(fish->get_pos().y / parti_h) * parti_nx;
	partitons.at(idx).insert(std::make_pair(fish->get_id(), fish));
	fish->set_pidx(idx);
}



void	World::add_food_to_world(Food* food)
{
	//先頭消去
	Food* ffood = foods.front();
	foods.pop_front();
	if(!ffood->death())
	{
		int pre_idx = ffood->get_pidx();
		parti_foods.at(pre_idx).erase(parti_foods.at(pre_idx).find(ffood->get_id()));
	}
	delete ffood;
	//リストに追加
	foods.push_back(food);

	//どこの領域にいるかを計算・更新
	int idx = (int)(food->get_pos().x / parti_w) + (int)(food->get_pos().y / parti_h) * parti_nx;
	parti_foods.at(idx).insert(std::make_pair(food->get_id(), food));
	food->set_pidx(idx);
}

void	World::add_noise_to_world(Noise* noise)
{
		//先頭消去
		Noise* fnoise = noises.front();
		noises.pop_front();
		if(!fnoise->death())
		{
			int pre_idx = fnoise->get_pidx();
			parti_noises.at(pre_idx).erase(parti_noises.at(pre_idx).find(fnoise->get_id()));
		}
		delete fnoise;
		//リストに追加
		noises.push_back(noise);

		//どこの領域にいるかを計算・更新
		int idx = (int)(noise->get_pos().x / parti_w) + (int)(noise->get_pos().y / parti_h) * parti_nx;
		parti_noises.at(idx).insert(std::make_pair(noise->get_id(), noise));
		noise->set_pidx(idx);
}


void World::add_to_noise_list(vec2d pos)
{
	std::lock_guard<std::mutex> lock(world_mtx);
	future_noise_list.push_back(pos);
}


GLuint	World::get_shader(){ return simple_shd->get_prog(); }