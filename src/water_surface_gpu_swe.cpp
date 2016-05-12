

#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>
#include <boost/multi_array.hpp>
#include "glm/mat3x3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
#include "vec3d.h"
#include "water_surface.h"
#include "teximage.h"

//頂点データ
static const GLfloat position[][2] =
{
	{-1.0f, -1.0f},
	{1.0f, -1.0f},
	{1.0f, 1.0f},
	{-1.0f, 1.0f}
};
static const int vertices = sizeof position / sizeof position[0];

//テクスチャサイズ
//const int TSIZE = 100;

//デフォルトの水面高
constexpr float H = 10.0f;

//キューブマッピング用
static const GLenum target[] = {
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
};
static const char *tex[] = {
	"nenvnx.png",
	"nenvny.png",
	"nenvnz.png",
	"nenvpx.png",
	"nenvpy.png",
	"nenvpz.png",
};
static TexImage envimg[6];


//基本メッシュ数
const int NumMesh = 100;

WaterSurfaceGPU_SWE::WaterSurfaceGPU_SWE(double w, double h)
	:WaterSurface(w, h),
	 NMeshX(NumMesh * w / h),
	 NMeshY(NumMesh)
{
	ren_shader = std::unique_ptr<MyShader>(new MyShader(
		"shader/wsgpuswe/ren.vs",
		"shader/wsgpuswe/ren.fs",
		"shader/wsgpuswe/ren.tcs",
		"shader/wsgpuswe/ren.tes",
		"shader/wsgpuswe/ren.gs"));
	st1_shader = std::unique_ptr<MyShader>(new MyShader("shader/wsgpuswe/swe1.vs", "shader/wsgpuswe/swe1.fs"));
	st2_shader = std::unique_ptr<MyShader>(new MyShader("shader/wsgpuswe/swe2.vs", "shader/wsgpuswe/swe2.fs"));
	copy_shader = std::unique_ptr<MyShader>(new MyShader("shader/wsgpuswe/copy.vs", "shader/wsgpuswe/copy.fs"));
	caustics_shader = std::unique_ptr<MyShader>(new MyShader("shader/wsgpuswe/caustics2.vs", "shader/wsgpuswe/caustics2.fs"));
	cau_pass1_shader = std::unique_ptr<MyShader>(new MyShader("shader/wsgpuswe/cau_pass1.vs", "shader/wsgpuswe/cau_pass1.fs"));


	//キューブマッピング
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	for(int i = 0; i < 6; ++i) {
		envimg[i].load(tex[i]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexImage2D(target[i], 0, GL_RGBA, 128, 128, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, envimg[i].get_data());
	}
	glBindTexture(GL_TEXTURE_2D, 0);


	for(int i = 0; i < 2; ++i)
	{
		glGenTextures(1, &us_map[i]);
		glBindTexture(GL_TEXTURE_2D, us_map[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, NMeshX, NMeshY, 0, GL_RGB, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}


	glGenTextures(1, &x_us_map);
	glBindTexture(GL_TEXTURE_2D, x_us_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, NMeshX + 1, NMeshY + 1, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);


	glGenTextures(1, &y_us_map);
	glBindTexture(GL_TEXTURE_2D, y_us_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, NMeshX + 1, NMeshY + 1, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);


	glGenTextures(1, &normal_map);
	glBindTexture(GL_TEXTURE_2D, normal_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, NMeshX, NMeshY, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	//GLubyte obstacle_buf[NMeshY][NMeshX][3] = {0};
	boost::multi_array<GLubyte, 3> obstacle_buf(boost::extents[NMeshY][NMeshX][3]);
	std::fill(obstacle_buf.data(), obstacle_buf.data() + obstacle_buf.num_elements(), 0);
	for(int i = 0; i < NMeshY; ++i)
	{
		obstacle_buf[i][0][0] = 255;
		obstacle_buf[i][NMeshX - 1][0] = 255;
	}
	for(int i = 0; i < NMeshX; ++i)
	{
		obstacle_buf[0][i][0] = 255;
		obstacle_buf[NMeshY - 1][i][0] = 255;
	}
	glGenTextures(1, &obstacle_map);
	glBindTexture(GL_TEXTURE_2D, obstacle_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, NMeshX, NMeshY, 0, GL_RGB, GL_UNSIGNED_BYTE, &obstacle_buf[0][0][0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	//コースティクスマップの割り当て
	glGenTextures(1, &caustics_map);
	glBindTexture(GL_TEXTURE_2D, caustics_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	//コースティクスマップの割り当て（２）
	glGenTextures(1, &cau_p1);
	glBindTexture(GL_TEXTURE_2D, cau_p1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glGenTextures(1, &cau_p2);
	glBindTexture(GL_TEXTURE_2D, cau_p2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	//フレームバッファの割り当て
	//USN
	glGenFramebuffersEXT(1, &fbo_usn);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_usn);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, us_map[0], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, us_map[1], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, normal_map, 0);

	int a = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(a != GL_FRAMEBUFFER_COMPLETE)
		printf("@Wusn  Failed to make complete framebuffer object %x \n", a);
	//USXY
	glGenFramebuffersEXT(1, &fbo_usxy);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_usxy);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, x_us_map, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, y_us_map, 0);

	a = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(a != GL_FRAMEBUFFER_COMPLETE)
		printf("@Wusxy Failed to make complete framebuffer object %x \n", a);
	//CAU
	glGenFramebuffersEXT(1, &fbo_cau);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_cau);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, caustics_map, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, cau_p1, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, cau_p2, 0);

	a = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(a != GL_FRAMEBUFFER_COMPLETE)
		printf("@Wcau  Failed to make complete framebuffer object %x \n", a);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	// 頂点配列オブジェクト
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// 頂点バッファオブジェクト
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * vertices, position, GL_STATIC_DRAW);

	// 結合されている頂点バッファオブジェクトを attribute 変数から参照できるようにする
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// 頂点バッファオブジェクトと頂点配列オブジェクトの結合を解除する
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//initial condition
	boost::multi_array<GLfloat, 3> initial(boost::extents[NMeshY][NMeshX][3]);
	std::fill(initial.data(), initial.data() + initial.num_elements(), 0.0);

	for(int i = 0; i < NMeshY; ++i)
	{
		for(int j = 0; j < NMeshX; ++j)
		{
			initial[i][j][0] = H;
			initial[i][j][1] = 0.0;
			initial[i][j][2] = 0.0;
		}
	}

	/*
	float hsize = 10;

	for(float r = 0; r < hsize; r += 1.0f / TSIZE)
	{
		constexpr int N = 200;
		for(int i = 0; i < N; i++)
		{
			int x = r * cos(2.0 * 3.14 * ((double)i / N)) + TSIZE / 2.0;
			int y = r * sin(2.0 * 3.14 * ((double)i / N)) + TSIZE / 2.0;
			initial[x][y][0] = 0.05 * cos(r * 3.141592 / 2.0 / hsize) + H;
		}
	}
	*/

	//for(int i = 0; i < 2; ++i)
	for(int i = 0; i < 2; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, us_map[i]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NMeshX, NMeshY, GL_RGB, GL_FLOAT, &initial[0][0][0]);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	//行列の設定
///*
	//--2D
	const double eyeH = 5.0;
	const double rad = atan(1.0 / eyeH) * 2.0;
	//proj = glm::perspective<float>(glm::radians(90.0), (float)width / height, 0.1, 10);
	proj = glm::perspective<float>(rad, 1.0, 0.1, 10);
	//gluLookAt(width / 2.0, height / 2.0, height / 2.0, width / 2.0, height / 2.0, 0.0, 0.0, 1.0, 0.0);
	//glOrtho(0.0, width, 0, height, -1000, 1000);
	modelv = glm::lookAt(
		glm::vec3(0.0, 0.0, H + eyeH),
		glm::vec3(0.0, 0.0, H),
		glm::vec3(0.0, 1.0, 0.0));
//*/
	//--3D
/*
	proj = glm::perspective<float>(glm::radians(60.0), (float)width / height, 0.1, 10);
	static double t = 0.0;
	const double r = 3.0;
	//double x = r * cos(t);
	//double y = r * sin(t);
	double x = 0.0;
	double y = -1.01;
	double z = 1.4 + H;
	modelv = glm::lookAt(
		glm::vec3(x, y, z),
		glm::vec3(0.0, 0.0, H),
		glm::vec3(0.0, 0.0, 1.0));
	t += 0.01;
*/
	normalm = glm::transpose(glm::inverse(glm::mat3(modelv)));
}

WaterSurfaceGPU_SWE::~WaterSurfaceGPU_SWE()
{
}

void WaterSurfaceGPU_SWE::update()
{
	//render側に実装
}

void WaterSurfaceGPU_SWE::render(GLuint underwater_tex)
{
	//	glClearColor(240.0 / 255, 248.0 / 255, 1.0, 1.0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glDisable(GL_DEPTH_TEST);

	/*
	const auto fbufs = std::vector<GLenum>{
	GL_COLOR_ATTACHMENT0_EXT,
	GL_COLOR_ATTACHMENT1_EXT,
	GL_COLOR_ATTACHMENT2_EXT,
	GL_COLOR_ATTACHMENT3_EXT,
	GL_COLOR_ATTACHMENT4_EXT,
	GL_COLOR_ATTACHMENT5_EXT
	};
	*/

	//glBindTexture(GL_TEXTURE_2D, 0);

	//*** simulation ***
	constexpr int SimulationStep = 2;
	for(int step = 0; step < SimulationStep; ++step)
	{
		//copy 1->0
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_usn);

		glViewport(0, 0, NMeshX, NMeshY);
		glUseProgram(copy_shader->get_prog());

		constexpr GLenum buf_cpy[1] = {GL_COLOR_ATTACHMENT0_EXT};
		glDrawBuffers(1, buf_cpy);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, us_map[1]);
		glUniform1i(glGetUniformLocation(copy_shader->get_prog(), "src"), 0); //gl_FragData[0]

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
		glBindVertexArray(0);

		//glFlush();
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);

		//us0 -> x_us, y_us
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_usxy);

		glViewport(0, 0, NMeshX + 1, NMeshY + 1);
		glUseProgram(st1_shader->get_prog());

		constexpr GLenum buf_st1[3] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
		glDrawBuffers(3, buf_st1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, us_map[0]);
		glUniform1i(glGetUniformLocation(st1_shader->get_prog(), "us0"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, x_us_map);
		glUniform1i(glGetUniformLocation(st1_shader->get_prog(), "x_us"), 1); //gl_FragData[0]
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, y_us_map);
		glUniform1i(glGetUniformLocation(st1_shader->get_prog(), "y_us"), 2); //gl_FragData[1]
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, obstacle_map);
		glUniform1i(glGetUniformLocation(st1_shader->get_prog(), "obstacle_tex"), 3);
		glActiveTexture(GL_TEXTURE0);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
		glBindVertexArray(0);

		//glFlush();

		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);

		//x_us, y_us -> us1
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_usn);

		glViewport(0, 0, NMeshX, NMeshY);
		glUseProgram(st2_shader->get_prog());

		constexpr GLenum buf_st2[2] = {GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT};
		glDrawBuffers(2, buf_st2);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, us_map[0]);
		glUniform1i(glGetUniformLocation(st2_shader->get_prog(), "us0"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, x_us_map);
		glUniform1i(glGetUniformLocation(st2_shader->get_prog(), "x_us"), 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, y_us_map);
		glUniform1i(glGetUniformLocation(st2_shader->get_prog(), "y_us"), 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, normal_map);
		glUniform1i(glGetUniformLocation(st2_shader->get_prog(), "nrm_tex"), 3); //gl_FragData[1]
		glActiveTexture(GL_TEXTURE0);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
		glBindVertexArray(0);

		//glFlush();

		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}
	//****

	//コークティクスシェーダ
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_cau);

	glViewport(0, 0, width, height);
	glUseProgram(cau_pass1_shader->get_prog());

	constexpr GLenum buf_cau_p1[2] = {GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT};
	glDrawBuffers(2, buf_cau_p1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, us_map[0]);
	glUniform1i(glGetUniformLocation(cau_pass1_shader->get_prog(), "us_tex"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_map);
	glUniform1i(glGetUniformLocation(cau_pass1_shader->get_prog(), "nrm_tex"), 1);
	glActiveTexture(GL_TEXTURE0);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
	glBindVertexArray(0);

	//glFlush();

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	glUseProgram(caustics_shader->get_prog());

	constexpr GLenum buf_cau[1] = {GL_COLOR_ATTACHMENT0_EXT};
	glDrawBuffers(1, buf_cau);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cau_p1);
	glUniform1i(glGetUniformLocation(caustics_shader->get_prog(), "cau_p1"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cau_p2);
	glUniform1i(glGetUniformLocation(caustics_shader->get_prog(), "cau_p2"), 1);
	glActiveTexture(GL_TEXTURE0);

	//頂点の描画
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
	glBindVertexArray(0);

	//glFlush();

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);


	//FBO -> 標準バッファ
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	//glEnable(GL_DEPTH_TEST);


	//水面シェーダ
	glUseProgram(ren_shader->get_prog());

	glViewport(0, 0, width, height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, us_map[0]);
	glUniform1i(glGetUniformLocation(ren_shader->get_prog(), "us_tex"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_map);
	glUniform1i(glGetUniformLocation(ren_shader->get_prog(), "nrm_tex"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, caustics_map);
	glUniform1i(glGetUniformLocation(ren_shader->get_prog(), "cau_tex"), 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
	glUniform1i(glGetUniformLocation(ren_shader->get_prog(), "envmap"), 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, underwater_tex);
	glUniform1i(glGetUniformLocation(ren_shader->get_prog(), "under_tex"), 4);
	glActiveTexture(GL_TEXTURE0);


	glUniformMatrix4fv(glGetUniformLocation(ren_shader->get_prog(), "modelViewMatrix"), 1, GL_FALSE, &modelv[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ren_shader->get_prog(), "projectionMatrix"), 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(ren_shader->get_prog(), "normalMatrix"), 1, GL_FALSE, &normalm[0][0]);

	//頂点の描画
	glBindVertexArray(vao);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawArrays(GL_PATCHES, 0, vertices);
	glBindVertexArray(0);
	//glBindVertexArray(vao);
	//glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
	//glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	//glDisable(GL_DEPTH_TEST);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

/*
	//--for debug--
	glActiveTexture(GL_TEXTURE0);
	//
	(GL_TEXTURE_2D, debug_map);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TSIZE + 1, TSIZE + 1, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	//glBindTexture(GL_TEXTURE_2D, us_map[0]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TSIZE, TSIZE, 0, GL_RGB32F, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, caustics_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB32F, GL_FLOAT, 0);
	//glBindTexture(GL_TEXTURE_2D, cau_p1);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3d(1.0, 1.0, 1.0);
	glTexCoord2d(0.0, 0.0);
	glVertex2d(-1.0, 0.5);
	glTexCoord2d(1.0, 0.0);
	glVertex2d(-0.5, 0.5);
	glTexCoord2d(1.0, 1.0);
	glVertex2d(-0.5, 1.0);
	glTexCoord2d(0.0, 1.0);
	glVertex2d(-1.0, 1.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
*/
}

void WaterSurfaceGPU_SWE::mouse_action(double mouse_x, double mouse_y)
{
	const float dx = (float)width / (NMeshX - 1);
	const float dy = (float)height / (NMeshY - 1);
	//---マウス操作による波の作成---
	float mx = mouse_x;
	float my = mouse_y;
	//near面上の座標
	glm::vec3 p0(mx, height - my, 0.0);
	glm::vec3 p1(mx, height - my, 1.0);
	glm::vec4 vp(0.0, 0.0, width, height);
	glm::vec3 np = glm::unProject(p0, modelv, proj, vp);
	glm::vec3 fp = glm::unProject(p1, modelv, proj, vp);
	float st_depth = H;
	float point_x = np.x + (fp.x - np.x) * (st_depth - np.z) / (fp.z - np.z);
	float point_y = np.y + (fp.y - np.y) * (st_depth - np.z) / (fp.z - np.z);
	const float h = H - 0.06;
	int px = (point_x * 0.5 + 0.5) * NMeshX;
	int py = (point_y * 0.5 + 0.5) * NMeshY;
	constexpr int N = 3;
	constexpr int HN = N / 2;
	const std::vector<std::array<GLfloat, 3>> suf(N * N, {h, 0, 0});
	if(px >= HN && px < NMeshX - N / HN && py >= HN && py < NMeshY - HN)
	{
		glBindTexture(GL_TEXTURE_2D, us_map[1]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, px - HN, py - HN, N, N, GL_RGB, GL_FLOAT, &suf[0][0]);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

