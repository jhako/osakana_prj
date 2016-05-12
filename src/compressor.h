

#pragma once


#include <stdint.h>
#include <vector>
#include <mutex>
#include "shader.h"

class Compress3
{
	int data_w;
	int data_h;

	uint32_t frame_buf;
	uint32_t pixel_buf[2];
	int pbo_index = 0; //0 or 1

	uint32_t src_tex;
	uint32_t wrk_tex;
	uint32_t wrk2_tex;
	//uint32_t dst_tex;

	uint32_t fzigzag_tex;
	uint32_t izigzag_tex;
	uint32_t quant_tex;

	MyShader fdct1_shader;
	MyShader fdct2_shader;
	MyShader fqt_shader;

	GLuint vao;
	GLuint vbo;

	std::vector<int8_t> pixels;
	std::vector<int8_t> codes;

	int codes_size = 0;

	void render();
	
	std::mutex com_mtx;

public:
	Compress3(int dw, int dh, uint32_t fbo, uint32_t s_tex);

	void compress();

	std::vector<int8_t>& get_codes() { return codes; }
	int get_codes_size(){ return codes_size; }
	std::mutex& get_mtx(){ return com_mtx; }
};