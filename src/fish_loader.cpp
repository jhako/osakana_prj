
#include <mutex>
#include <iostream>
#include <algorithm>
#include "fish_loader.h"
#include "world.h"
#include "fish.h"
#include "shark.h"
#include "teximage.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>



FishLoader::FishLoader(std::string dir_, World* p_world_)
	:dir(dir_),
	 p_world(p_world_)
{
}



FishLoader::~FishLoader()
{
	for(int i = 0; i < texture_list.size(); ++i)
	{
		auto tex = texture_list.back();
		delete tex;
		texture_list.pop_back();
	}
}


std::vector<Fish*> FishLoader::load_fish()
{
	//魚リスト
	std::vector<Fish*>	additions;

	//ファイルリスト
	std::vector<std::string> file_list;
	file_list.reserve(30); //領域確保

	//ディレクトリ内のファイルをすべてリストに追加
	const boost::filesystem::path path(dir);
	const auto end = boost::filesystem::directory_iterator();
	for(auto p = boost::filesystem::directory_iterator(path); p != end; ++p)
	{
		std::string fn = p->path().filename().string();
		if(fn.find_first_of('.') != std::string::npos)
		{
			//std::cout << "found : " << fn << std::endl;
			file_list.push_back(fn);
		}
	}

	for(auto& it : file_list)
	{
		//まだ追加されていない
		if(file_history.find(it) == file_history.end())
		{
			//ファイル名を作成
			std::string fn = dir + "/" + (it);
			//std::cout << "add : " << fn.c_str() << std::endl;
			//テクスチャの作成
			texture_list.push_back(new TexImageWithShader(fn.c_str(), 22, 50, p_world->get_shader()));
			//魚追加
			additions.push_back(new Fish(vec2d(320, 320), vec2d(1, 0), texture_list.back(), 2.3));
			//履歴に追加
			file_history.insert(it);
		}
	}

	return additions;
}
