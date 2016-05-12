

#pragma once

#include <unordered_set>
#include <vector>

class World;
class Fish;
class TexImageWithShader;

class FishLoader
{
public:
	FishLoader(std::string dir_, World* p_world_);
	~FishLoader();

	std::vector<Fish*>	load_fish();

private:
	std::string		dir;
	World*			p_world;

	std::unordered_set<std::string>		file_history;
	std::vector<TexImageWithShader*>	texture_list;

};
