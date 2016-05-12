
#ifndef FISH_H
#define FISH_H


#include "vec2d.h"

//--プロトタイプ宣言--
class World;
class TexImageWithShader;
class Behavior;

using GLuint = unsigned int;

//--お魚クラス--
class Fish
{
protected:
	vec2d pos;           //位置
	vec2d velo;          //速度
	vec2d dire;          //向き
	TexImageWithShader* tex;       //画像データ
	Behavior* behavior;  //振る舞い
	double max_speed;    //最高速度
	int id;              //ID（識別番号）
	int partidx;         //位置インデックス（領域分割で利用）

public:
	//初期化関数（複数のコンストラクタでの同様の記述をまとめた関数）
	void init(vec2d pos_, vec2d velo_);
	Fish(vec2d pos_, vec2d velo_, GLuint shader);
	Fish(vec2d pos_, vec2d velo_, TexImageWithShader* tex_, double maxspd);
	virtual ~Fish();

	//情報を更新
	virtual void update(World* p_world);
	//描画
	void render(World* p_world);

	//アクセサ
	vec2d get_pos(){ return pos; }
	vec2d get_dire(){ return dire; }
	vec2d get_velocity(){ return velo; }
	double get_maxspeed(){ return max_speed; }
	int get_id(){ return id; }
	int get_pidx(){ return partidx; }
	void set_pidx(int idx){ partidx = idx; }
};

#endif
