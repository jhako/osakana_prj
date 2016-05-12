

#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <iostream>
#include "compressor.h"

constexpr int fzz_x[64]
= {0, 1, 0, 0, 1, 2, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 5, 6, 7, 7, 6, 7};
constexpr int fzz_y[64]
= {0, 0, 1, 2, 1, 0, 0, 1, 2, 3, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 4, 5, 6, 7, 7, 6, 5, 6, 7, 7};

constexpr int izz_x[64]
= {0, 1, 5, 6, 6, 7, 3, 4, 2, 4, 7, 5, 0, 2, 5, 2, 3, 0, 4, 1, 1, 6, 1, 3, 1, 3, 2, 0, 7, 0, 4, 5, 2, 3, 7, 0, 7, 5, 4, 6, 4, 6, 1, 6, 6, 3, 7, 4, 5, 2, 5, 7, 2, 0, 3, 5, 3, 4, 0, 1, 1, 2, 6, 7};
constexpr int izz_y[64]
= {0, 0, 0, 0, 1, 1, 3, 3, 0, 0, 0, 1, 2, 3, 3, 5, 0, 1, 1, 2, 3, 3, 5, 5, 1, 1, 2, 3, 3, 5, 5, 6, 1, 2, 2, 4, 4, 5, 6, 6, 2, 2, 4, 4, 5, 6, 6, 7, 2, 4, 4, 5, 6, 7, 7, 7, 4, 4, 6, 6, 7, 7, 7, 7};

const uint8_t i_qtable[8][8] =
{
	{8, 16, 19, 22, 26, 27, 29, 34},
	{16, 16, 22, 24, 27, 29, 34, 37},
	{19, 22, 26, 27, 29, 34, 34, 38},
	{22, 22, 26, 27, 29, 34, 37, 40},
	{22, 26, 27, 29, 32, 35, 40, 48},
	{26, 27, 29, 32, 35, 40, 48, 58},
	{26, 27, 29, 34, 38, 46, 56, 69},
	{27, 29, 35, 38, 46, 56, 69, 83}
};


//頂点データ
static const GLfloat position[][2] =
{
	{-1.0f, -1.0f},
	{1.0f, -1.0f},
	{1.0f, 1.0f},
	{-1.0f, 1.0f}
};
static const int vertices = sizeof position / sizeof position[0];



//ランレングス圧縮
int run_length_encode(int8_t* dst, int8_t* src, int DN, int SN)
{
	int8_t temp = src[0];
	int c = 1;
	int index = 0;
	for(int i = 1; i < SN; ++i)
	{
		if(temp != src[i] || c == 255)
		{
			dst[index++] = temp;
			dst[index++] = c - 128;
			if(index + 1 >= DN)
			{
				//std::cout << "ERROR : OUT OF RANGE" << std::endl;
				return index;
			}
			temp = src[i];
			c = 1;
		}
		else
		{
			++c;
		}
	}
	dst[index++] = temp;
	dst[index++] = c - 128;
	return index;
}

void run_length_decode(int8_t* dst, int8_t* src, int DN, int SN)
{
	int s_idx = 0;
	int d_idx = 0;
	while(s_idx + 1 < SN)
	{
		int x = src[s_idx++];
		int c = src[s_idx++] + 128;
		for(int j = 0; j < c; ++j)
		{
			dst[d_idx++] = x;
			if(d_idx == DN)
			{
				//std::cout << "ERROR : OUT OF RANGE &" << SN << std::endl;
				return;
			}
		}
	}
}


Compress3::Compress3(int dw, int dh, uint32_t fbo, uint32_t s_tex)
  :	data_w(dw), data_h(dh),
	fdct1_shader("shader/compressor/fdct1_shd.vs", "shader/compressor/fdct1_shd.fs"),
	fdct2_shader("shader/compressor/fdct2_shd.vs", "shader/compressor/fdct2_shd.fs"),
	fqt_shader("shader/compressor/fqt_shd.vs", "shader/compressor/fqt_shd.fs"),
	frame_buf(fbo),
	src_tex(s_tex)
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buf);

	//A0 : data_tex (already set)
	//A1 : wrk
	//A2 : wrk2
	//int i = 1;
	//32bit - texture
	/*
	for(auto* tex : {&wrk_tex, &wrk2_tex})
	{
		GLuint btex;
		glGenTextures(1, &btex);
		(*tex) = btex;
		glBindTexture(GL_TEXTURE_2D, *tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, data_w, data_h, 0, GL_RGB, GL_FLOAT, 0);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + (i++), GL_TEXTURE_2D, *tex, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	*/
	glGenTextures(1, &wrk_tex);
	glBindTexture(GL_TEXTURE_2D, wrk_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, data_w, data_h, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, wrk_tex, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &wrk2_tex);
	glBindTexture(GL_TEXTURE_2D, wrk2_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, data_w, data_h, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, wrk2_tex, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	int a = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(a != GL_FRAMEBUFFER_COMPLETE)
		printf("@C Failed to make complete framebuffer object %x \n", a);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


	//ZIGZAG行列
	glGenTextures(1, &fzigzag_tex); //FOWARD
	glBindTexture(GL_TEXTURE_2D, fzigzag_tex);
	uint8_t fzigzag_buf[64][2];
	for(int i = 0; i < 64; ++i)
	{
		fzigzag_buf[i][0] = fzz_x[i];
		fzigzag_buf[i][1] = fzz_y[i];
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, 8, 8, 0, GL_RG, GL_UNSIGNED_BYTE, fzigzag_buf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &izigzag_tex); //INVERT
	glBindTexture(GL_TEXTURE_2D, izigzag_tex);
	uint8_t izigzag_buf[64][2];
	for(int i = 0; i < 64; ++i)
	{
		izigzag_buf[i][0] = izz_x[i];
		izigzag_buf[i][1] = izz_y[i];
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, 8, 8, 0, GL_RG, GL_UNSIGNED_BYTE, izigzag_buf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);


	//量子化行列
	glGenTextures(1, &quant_tex);
	glBindTexture(GL_TEXTURE_2D, quant_tex);
	uint8_t quant_buf[64][2];
	for(int i = 0; i < 64; ++i)
	{
		quant_buf[i][0] = (&i_qtable[0][0])[i];
		quant_buf[i][1] = 255;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, 8, 8, 0, GL_RG, GL_UNSIGNED_BYTE, quant_buf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 頂点配列オブジェクト
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * vertices, position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//PBOの作成
	for(int i = 0; i < 2; ++i)
	{
		glGenBuffers(1, &pixel_buf[i]);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixel_buf[i]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, data_w * data_h * 3, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	pixels.resize(data_w * data_h * 3);
	codes.resize(data_w * data_h * 3);
}





void Compress3::compress()
{
	//--FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buf);

	glViewport(0, 0, data_w, data_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//**F-DCT1**
	glUseProgram(fdct1_shader.get_prog());
	constexpr GLenum buf_fdct1[1] = {GL_COLOR_ATTACHMENT1_EXT};
	glDrawBuffers(1, buf_fdct1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, src_tex);
	glUniform1i(glGetUniformLocation(fdct1_shader.get_prog(), "TX"), data_w);
	glUniform1i(glGetUniformLocation(fdct1_shader.get_prog(), "TY"), data_h);
	glUniform1i(glGetUniformLocation(fdct1_shader.get_prog(), "src"), 0);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	//*******


	//**F-DCT2**
	glUseProgram(fdct2_shader.get_prog());
	constexpr GLenum buf_fdct2[1] = {GL_COLOR_ATTACHMENT2_EXT};
	glDrawBuffers(1, buf_fdct2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wrk_tex);
	glUniform1i(glGetUniformLocation(fdct2_shader.get_prog(), "TX"), data_w);
	glUniform1i(glGetUniformLocation(fdct2_shader.get_prog(), "TY"), data_h);
	glUniform1i(glGetUniformLocation(fdct2_shader.get_prog(), "src"), 0);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);


	//*******
	//**F-QUANTATTION**
	glUseProgram(fqt_shader.get_prog());
	constexpr GLenum buf_fqt[1] = {GL_COLOR_ATTACHMENT0_EXT};
	glDrawBuffers(1, buf_fqt);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wrk2_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fzigzag_tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, quant_tex);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(fqt_shader.get_prog(), "TX"), data_w);
	glUniform1i(glGetUniformLocation(fqt_shader.get_prog(), "TY"), data_h);
	glUniform1i(glGetUniformLocation(fqt_shader.get_prog(), "src"), 0);
	glUniform1i(glGetUniformLocation(fqt_shader.get_prog(), "fzigzag"), 1);
	glUniform1i(glGetUniformLocation(fqt_shader.get_prog(), "quant"), 2);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	//*******

	//**copy**
	int next_pi = (pbo_index + 1) % 2;
	//一つのPBOに転送
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buf[next_pi]);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glReadPixels(0, 0, data_w, data_h, GL_RGB, GL_UNSIGNED_BYTE, 0);
	//前フレームで取得したバッファ
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buf[pbo_index]);
	GLubyte *ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if(ptr)
	{
		for(int i = 0; i < data_w * data_h * 3; ++i)
		{
			pixels[i] = ptr[i];
			//codes[i] = 0;
		}
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
	pbo_index = next_pi;
	//*********
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	///

	//**ENCODE**
	{
		std::lock_guard<std::mutex> locks(com_mtx);
		codes_size = run_length_encode(&codes[0], &pixels[0], codes.size(), pixels.size());
	}
	//std::cout << codes.size() << " -> " << codes_size << " : " << 100.0 * codes_size / codes.size() << "%" << std::endl;


	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//glViewport(0, 0, 640, 640);
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, src_tex);
	//glBegin(GL_QUADS);
	//glColor3d(1.0, 1.0, 1.0);
	//glTexCoord2d(0.0, 0.0);
	//glVertex2d(-1.0, 0.5);
	//glTexCoord2d(1.0, 0.0);
	//glVertex2d(-0.5, 0.5);
	//glTexCoord2d(1.0, 1.0);
	//glVertex2d(-0.5, 1.0);
	//glTexCoord2d(0.0, 1.0);
	//glVertex2d(-1.0, 1.0);
	//glEnd();
}



void Compress3::render()
{
	glBegin(GL_POLYGON);
	glVertex2f(-1.0, -1.0);
	glVertex2f(-1.0, 1.0);
	glVertex2f(1.0, 1.0);
	glVertex2f(1.0, -1.0);
	glEnd();
}