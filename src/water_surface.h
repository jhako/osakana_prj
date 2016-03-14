


#pragma once


#include <vector>
#include <GL/glut.h>
#include <memory>

#include "glm/gtc/matrix_transform.hpp"


class MyShader;


class WaterSurface
{
protected:
	double	width;
	double	height;

	GLuint	frame_buf;

public:

	WaterSurface(double w, double h)
		:width(w), height(h)
	{}
	virtual ~WaterSurface(){}

	virtual void update() = 0;

	virtual void render(GLuint underwater_tex) = 0;

	virtual void mouse_action(double mouse_x, double mouse_y) = 0;
};


//CPU上で計算する
class WaterSurfaceCPU :public WaterSurface
{
	//水面を表現する頂点データ（z値）
	std::vector<std::vector<double>>		surf_vertex_z[3];

	//シェーダ
	std::unique_ptr<MyShader>	water_shader;
	std::unique_ptr<MyShader>	caustics_shader;

	//キューブマッピング用テクスチャ
	GLuint		cube_tex;
	//コースティクスマップ
	GLuint		caustics_map;

	//水面描画
	void draw_water_surface();

public:
	WaterSurfaceCPU(double w, double h);
	~WaterSurfaceCPU();

	void update();

	void render(GLuint underwater_tex);

	void mouse_action(double mouse_x, double mouse_y);

};



//GPU上で計算する（浅水方程式を使用）
class WaterSurfaceGPU_SWE:public WaterSurface
{
	std::unique_ptr<MyShader> copy_shader;
	std::unique_ptr<MyShader> st1_shader;
	std::unique_ptr<MyShader> st2_shader;
	std::unique_ptr<MyShader> ren_shader;
	std::unique_ptr<MyShader> caustics_shader;
	std::unique_ptr<MyShader> cau_pass1_shader;

	GLuint vao;
	GLuint vbo;

	GLuint frame_buf;
	GLuint us_map[2];
	GLuint x_us_map;
	GLuint y_us_map;
	GLuint normal_map;
	GLuint obstacle_map;
	GLuint caustics_map; //コースティクスマップ
	GLuint cau_p1, cau_p2;
	GLuint cube_tex; //キューブマッピング用テクスチャ

	//行列群
	glm::mat4x4 proj,   //射影行列
		        modelv; //モデルビュー行列
	glm::mat3x3 normalm; 


public:
	WaterSurfaceGPU_SWE(double w, double h);
	~WaterSurfaceGPU_SWE();

	void update();

	void render(GLuint underwater_tex);

	void mouse_action(double mouse_x, double mouse_y);

};

