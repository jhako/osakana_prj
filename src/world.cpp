


#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include <chrono>
#include <future>
#include "world.h"
#include "teximage.h"
#include "fish.h"
#include "shark.h"
#include "target.h"
#include "shader.h"
#include "vec3d.h"
#include "fish_loader.h"
#include "water_surface.h"


//X・Y方向にどれだけ分割するか
const int PARTITION_X = 10;
const int PARTITION_Y = 10;

static TexImage bottom_img;


World::World(int w, int h) : width(w), height(h)
{
	//メモリ確保
	partitons.resize(PARTITION_X*PARTITION_Y);
	fishes.reserve(500);
	targets.reserve(50);

	//fishの追加
	for (int i = 0; i < 200; ++i)
	{
		//位置はランダム、初期速度はゼロ
		Fish* fish = new Fish(vec2d(100 + (double)rand() / RAND_MAX * 400,
			100 + (double)rand() / RAND_MAX * 400),
			vec2d());
		add_fish_to_world(fish);
	}

	//三匹sharkを追加
	sharks.push_back(new Shark(vec2d(100, 100), vec2d(0, 0)));
	sharks.push_back(new Shark(vec2d(250, 250), vec2d(0, 0)));
	sharks.push_back(new Shark(vec2d(450, 450), vec2d(0, 0)));

	//テストターゲット
	targets.push_back(new Target(vec2d(320, 200)));

	//海底イメージ
	bottom_img.load("bottom.png");

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
	for (int i = 0; i < targets.size(); ++i)
	{
		Target* target = targets.back();
		delete target;
		targets.pop_back();
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

	if(if_load_fish)
	{
		//ロードタスクを設定
		fut_load = std::async([&]{ return fish_loader->load_fish(); });
	}

	//エンティティのアップデート
//	update_entities();
	auto fut_ue = std::async([&]{ update_entities(); });

	//水面サーフェスの計算
	water_surface->update();
	//calc_water_surface();
//	auto fut_cw = std::async([&]{ calc_water_surface(); }); //並列化


	fut_ue.get();
//	fut_cw.get();

	if(if_load_fish)
	{
		//タスクの終了を待機
		auto additional_fishes = fut_load.get();
		//魚の追加
		while(!additional_fishes.empty())
		{
			add_fish_to_world(additional_fishes.back());
			additional_fishes.pop_back();
		}
	}

	//ステップの更新
	step += 1;

//	//for debug
//	auto end_t = std::chrono::system_clock::now();
//	std::cout << "TS : " << std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count() << "\n";
}

void World::opengl_update()
{
	//山波を作る
	if(mouse_state == true)
	{
		water_surface->mouse_action(mouse_pos.x, mouse_pos.y);
	}
}

void World::update_entities()
{
	for(auto& tar : targets)
	{
		tar->update(this);
	}
	for(auto& fish : fishes)
	{
		//各fishがどこの領域に属するかを更新
		vec2d pos = fish->get_pos();
		int pre_idx = fish->get_pidx();
		int idx = pos.x / (int)(width / PARTITION_X)
			+ (int)(pos.y / (int)(height / PARTITION_Y))*PARTITION_X;
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
		int idx = shark->get_pos().x / (int)(width / PARTITION_X)
			+ (int)(shark->get_pos().y / (int)(height / PARTITION_Y))*PARTITION_X;
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
	for(auto& tar : targets)
	{
		tar->render(this);
	}
	for(auto& fish : fishes)
	{
		fish->render(this);
	}
	for(auto& shark : sharks)
	{
		shark->render(this);
	}

	glFlush();
	glDrawBuffer(GL_FRONT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glUseProgram(0);


	glDisable(GL_ALPHA_TEST);
//	glEnable(GL_DEPTH_TEST);

//	glClearColor(0.5, 0.5, 0.5, 1.0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	water_surface->render(underwater_tex);

	glutSwapBuffers();
}

std::vector<Fish*> World::get_neighborfishes(int idx)
{
	//idxの領域の周辺８つの領域を含めて、属するfishを返す
	std::vector<Fish*> nfishes;
	nfishes.reserve(100);
	int x = idx%PARTITION_X;
	int y = idx / PARTITION_X;
	for (int i = x - 1; i <= x + 1; ++i)
	{
		for (int j = y - 1; j <= y + 1; ++j)
		{
			if (i < 0 || i >= PARTITION_X || j < 0 || j >= PARTITION_Y)
				continue;
			int nidx = i + j*PARTITION_X;
			for (std::map<int, Fish*>::iterator it = partitons.at(nidx).begin();
			it != partitons.at(nidx).end(); ++it)
			{
				nfishes.push_back(it->second);
			}
		}
	}
	return nfishes; //NRVO
}

void World::add_fish_to_world(Fish* fish)
{
	//リストに追加
	fishes.push_back(fish);

	//どこの領域にいるかを計算・更新
	int idx = fish->get_pos().x / (int)(width / PARTITION_X)
		+ (int)(fish->get_pos().y / (int)(height / PARTITION_Y))*PARTITION_X;
	partitons.at(idx).insert(std::make_pair(fish->get_id(), fish));
	fish->set_pidx(idx);
}
