
#include <mutex>
#include <iostream>
#include <algorithm>
#include "fish_loader.h"
#include "world.h"
#include "fish.h"
#include "shark.h"
#include "teximage.h"

#ifdef _WIN32
#include "win/dirent.h"
#else
#include <direct.h>
#endif // WIN32



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
	auto p_dir = opendir(dir.c_str());
	if(p_dir != NULL)
	{
		auto dent = readdir(p_dir);
		while(dent)
		{
			file_list.push_back(dent->d_name);
			dent = readdir(p_dir);
		}
		closedir(p_dir);
	}

	//for(auto& file : file_list)
	//{
	//	std::cout << file.c_str() << std::endl;
	//}

	auto begin = file_list.begin();
	auto end = std::remove_if(file_list.begin(), file_list.end(),
		[&](const std::string& str){ return str.find(".png") == std::string::npos; }
	);
	//puts("---");
	for(auto it = begin; it != end; ++it)
	{
		//std::cout << it->c_str() << std::endl;
		//まだ追加されていない
		if(file_history.find(*it) == file_history.end())
		{
			//ファイル名を作成
			std::string fn = dir + "/" + (*it);
			//std::cout << "add : " << fn.c_str() << std::endl;
			//テクスチャの作成
			texture_list.push_back(new TexImage(fn.c_str()));
			//魚追加
			additions.push_back(new Fish(vec2d(320, 320), vec2d(1, 0), texture_list.back(), 2.3));
			//履歴に追加
			file_history.insert(*it);
		}
	}

	return additions;
}
