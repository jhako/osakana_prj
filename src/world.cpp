
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <GL/glut.h>
#include "world.h"
#include "teximage.h"
#include "fish.h"
#include "shark.h"

//X・Y方向にどれだけ分割するか
const int PARTITION_X = 10;
const int PARTITION_Y = 10;

World::World(): width(640), height(640)
{
    partitons.resize(PARTITION_X*PARTITION_Y);
    for(int i = 0; i < 400; ++i)
    {
	//位置はランダム、初期速度はゼロ
	Fish* fish = new Fish(vec2d(100+(double)rand()/RAND_MAX*400,
				    100+(double)rand()/RAND_MAX*400),
			      vec2d());
	fishes.push_back(fish);

	//どこの領域にいるかを計算・更新
	int idx = fish->get_pos().x / (int)(width / PARTITION_X) 
	    + (int)(fish->get_pos().y / (int)(height / PARTITION_Y))*PARTITION_X;
	partitons.at(idx).insert(std::make_pair(fish->get_id(), fish));
	fish->set_pidx(idx);
    }

    //三匹sharkを追加
    sharks.push_back(new Shark(vec2d(100,100), vec2d(0,0)));
    sharks.push_back(new Shark(vec2d(250,250), vec2d(0,0)));
    sharks.push_back(new Shark(vec2d(450,450), vec2d(0,0)));
}

World::~World()
{
    for(int i=0; i < fishes.size(); ++i)
    {
	Fish* fish = fishes.back();
	delete fish;
	fishes.pop_back();
    }
    for(int i=0; i < sharks.size(); ++i)
    {
	Shark* shark = sharks.back();
	delete shark;
	sharks.pop_back();
    }
}

void World::update()
{
    for(int i=0; i < fishes.size(); ++i)
    {
	//各fishがどこの領域に属するかを更新
	Fish* fish = fishes.at(i);
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

	fishes.at(i)->update(this);
    }
    for(int i=0; i < sharks.size(); ++i)
    {
	int idx = sharks.at(i)->get_pos().x / (int)(width / PARTITION_X) 
	    + (int)(sharks.at(i)->get_pos().y / (int)(height / PARTITION_Y))*PARTITION_X;
	sharks.at(i)->set_pidx(idx);

	sharks.at(i)->update(this);
    }

}

void World::render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    for(int i=0; i < fishes.size(); ++i)
	fishes.at(i)->render(this);
    for(int i=0; i < sharks.size(); ++i)
	sharks.at(i)->render(this);
    glutSwapBuffers();
}


std::vector<Fish*> World::get_neighborfishes(int idx)
{
    //idxの領域の周辺８つの領域を含めて、属するfishを返す
    std::vector<Fish*> nfishes;
    int x = idx%PARTITION_X;
    int y = idx/PARTITION_X;
    for(int i = x-1; i <= x+1; ++i)
    {
	for(int j = y-1; j <= y+1; ++j)
	{
	    if(i<0 || i>=PARTITION_X || j<0 || j>=PARTITION_Y)
		continue;
	    int nidx = i + j*PARTITION_X; 
	    for(std::map<int, Fish*>::iterator it = partitons.at(nidx).begin();
		it != partitons.at(nidx).end(); ++it)
	    {
		nfishes.push_back(it->second);
	    }
	}
    }
    return nfishes;
}
