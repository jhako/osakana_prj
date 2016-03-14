
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "shader.h"
#include "vec3d.h"
#include "water_surface.h"
#include "teximage.h"


//水面の解像度
constexpr int SURF_RESOLUTION_X = 100; //頂点数（X)
constexpr int SURF_RESOLUTION_Y = 100; //頂点数（Y)

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



WaterSurfaceCPU::WaterSurfaceCPU(double w, double h)
	:WaterSurface(w, h)
{
	// 水面表面の頂点データの初期化
	//	* surf_vertex_z[i][j][k]
	//		i : 時間ステップ (0, 1, 2)
	//		j : X座標インデックス (0, …, SURF_RESOLUTION_X - 1)
	//		k : Y座標インデックス (0, …, SURF_RESOLUTION_Y - 1)
	//  水面は四角ポリゴンの集合体として表現，各頂点のz位置をsurf_vertex_zに入れる
	//　i = 0, SURF_RESOLUTION_X - 1 および j = 0, SURF_RESOLUTION_Y - 1の端部頂点は，
	//  境界条件としてのダミーなので，実際のx, y座標とインデックスは1つずつずれて対応する
	//   (x, y) <-> (dx * [i + 1], dy * [j + 1])
	for(int i : {0, 1, 2})
	{
		surf_vertex_z[i].resize(SURF_RESOLUTION_X + 2); //ダミー2個分
		for(int j = 0; j < SURF_RESOLUTION_X + 2; ++j)
		{
			surf_vertex_z[i][j].resize(SURF_RESOLUTION_Y + 2, 20.0);
		}
	}

	//初期条件（テスト）
	int px = SURF_RESOLUTION_X / 4;
	int py = SURF_RESOLUTION_Y / 4;
	constexpr int N = 5;
	for(int i = px - N; i <= px + N; ++i)
	{
		for(int j = py - N; j <= py + N; ++j)
		{
			for(int s = 0; s < 2; ++s)
			{
				surf_vertex_z[s][i][j] += 10.0 * sin(3.141592 / (2 * N));
			}
		}
	}

	//2ステップ分計算
	constexpr double dt = 0.016;
	constexpr double c = 10;
	const double dx = (double)width / (SURF_RESOLUTION_X - 1);
	const double dy = (double)height / (SURF_RESOLUTION_Y - 1);
	for(int step = 1; step == 2; ++step)
	{
		for(int nx = 1; nx < SURF_RESOLUTION_X; ++nx)
		{
			for(int ny = 1; ny < SURF_RESOLUTION_Y; ++ny)
			{
				double x = nx * dx;
				double y = ny * dy;
				surf_vertex_z[2][nx][ny] = 2 * surf_vertex_z[1][nx][ny] - surf_vertex_z[0][nx][ny]
					+ (c * c * dt * dt / dx / dx)
					* (surf_vertex_z[1][nx - 1][ny] - 2 * surf_vertex_z[1][nx][ny] + surf_vertex_z[1][nx + 1][ny])
					+ (c * c * dt * dt / dy / dy)
					* (surf_vertex_z[1][nx][ny - 1] - 2 * surf_vertex_z[1][nx][ny] + surf_vertex_z[1][nx][ny + 1]);
			}
		}
	}

	//シェーダの作成
	water_shader = std::unique_ptr<MyShader>(new MyShader("shader/wscpu/water.vs", "shader/wscpu/water.fs"));
	caustics_shader = std::unique_ptr<MyShader>(new MyShader("shader/wscpu/caustics.vs", "shader/wscpu/caustics.fs"));

	//キューブマッピング
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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
	glActiveTexture(GL_TEXTURE0);


	//コースティクスマップの割り当て
	glGenTextures(1, &caustics_map);
	glBindTexture(GL_TEXTURE_2D, caustics_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	//フレームバッファの割り当て
	glGenFramebuffersEXT(1, &frame_buf);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buf);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, caustics_map, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

WaterSurfaceCPU::~WaterSurfaceCPU()
{
}

void WaterSurfaceCPU::update()
{
	//波動方程式による計算
	/*  差分法
	//波動方程式（c＝伝播速度）
	∂^2u/∂t^2 = c^2∇^2 u
	-> (u[n+1] - 2u[n] + u[n-1])/(dt)^2 = c^2((ux-1[n] - 2u[n] + ux+1[n])/(dx)^2 + (uy-1[n] - 2u[n] + uy+1[n])/(dy)^2)
	//減衰モデル（c＝減衰係数，ρ＝密度）
	　 ∂^2u/∂t^2 += -c/ρ∂u/∂t
	  -> (u[n+1] - 2u[n] + u[n-1])/(dt)^2 += -h(u[n]-u[n-1])/(dt)
	  */
	constexpr double dt = 0.016;
	constexpr double c = 200;
	constexpr double h = 0.1; //減衰率
	const double dx = (double)width / (SURF_RESOLUTION_X - 1);
	const double dy = (double)height / (SURF_RESOLUTION_Y - 1);
#ifdef DEBUG
	//CFL条件 : dx > cdt（発散しないための条件）
	assert(dx > c*dt && dy > c*dt, "CFL condition is not fulfilled!");
#endif // DEBUG

	//ステップ更新
	for(int nx = 1; nx <= SURF_RESOLUTION_X; ++nx)
	{
		for(int ny = 1; ny <= SURF_RESOLUTION_Y; ++ny)
		{
			surf_vertex_z[0][nx][ny] = surf_vertex_z[1][nx][ny];
			surf_vertex_z[1][nx][ny] = surf_vertex_z[2][nx][ny];
		}
	}
	for(int nx = 1; nx <= SURF_RESOLUTION_X; ++nx)
	{
		for(int ny = 1; ny <= SURF_RESOLUTION_Y; ++ny)
		{
			//波動方程式（差分法）
			surf_vertex_z[2][nx][ny] = 2 * surf_vertex_z[1][nx][ny] - surf_vertex_z[0][nx][ny]
				+ (c * c * dt * dt / dx / dx)
				* (surf_vertex_z[1][nx - 1][ny] - 2 * surf_vertex_z[1][nx][ny] + surf_vertex_z[1][nx + 1][ny])
				+ (c * c * dt * dt / dy / dy)
				* (surf_vertex_z[1][nx][ny - 1] - 2 * surf_vertex_z[1][nx][ny] + surf_vertex_z[1][nx][ny + 1]);
			//減衰
			surf_vertex_z[2][nx][ny] += -h*(surf_vertex_z[1][nx][ny] - surf_vertex_z[0][nx][ny])*dt;
		}
	}
}

void WaterSurfaceCPU::render(GLuint underwater_tex)
{
	//ライティングの設定
	GLfloat light_position0[] = {10, 10, 100, 0};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glEnable(GL_LIGHT0); //光源0を利用
	glEnable(GL_LIGHTING);

	//コースティクスマップの描画
	glUseProgram(caustics_shader->get_prog());
	glUniform1f(glGetUniformLocation(caustics_shader->get_prog(), "width"), (float)width);
	glUniform1f(glGetUniformLocation(caustics_shader->get_prog(), "height"), (float)height);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buf);
	const auto fbufs = std::vector<GLenum>{
		GL_COLOR_ATTACHMENT0_EXT
	};
	glDrawBuffers(fbufs.size(), &fbufs[0]);
	draw_water_surface();
	glFlush();
	glDrawBuffer(GL_FRONT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glUseProgram(0);

//	glDisable(GL_ALPHA_TEST);
//	glEnable(GL_DEPTH_TEST);

	//#define ON_2D //2D視点にするか否か
#ifdef ON_2D
	static GLfloat eye[3] = {320.0, 320.0, 1000.0};
	//2D
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, (double)width / height, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	gluLookAt(width / 2.0, height / 2.0, height / 2.0, width / 2.0, height / 2.0, 0.0, 0.0, 1.0, 0.0);
#else
	static GLfloat eye[3] = {320.0, 0.0, 500.0};
	static double time = 0.0;
	//		eye[0] = 320 + 320 * cos(time);
	//		eye[1] = 320 + 320 * sin(time);
	//		eye[2] = 250 + 200 * sin(time);
	time += 0.01;
	//--視点を3Dに---
	glViewport(0, 0, width, height);
	//射影行列を設定
	glMatrixMode(GL_PROJECTION);
	//単位行列に初期化
	glLoadIdentity();
	//視界の設定
	gluPerspective(60.0, (double)width / height, 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		eye[0], eye[1], eye[2], // 視点の位置x,y,z;
		320.0, 320.0, 0.0,   // 視界の中心位置の参照点座標x,y,z
		0.0, 0.0, 1.0);  //視界の上方向のベクトルx,y,z
#endif
	//---------------

	//シェーダの設定
	glUseProgram(water_shader->get_prog());
	glUniform1i(glGetUniformLocation(water_shader->get_prog(), "envmap"), 1);
	GLfloat x_scale = 1.0 / (float)width, y_scale = 1.0 / (float)height;
	glUniform1f(glGetUniformLocation(water_shader->get_prog(), "x_scale"), x_scale);
	glUniform1f(glGetUniformLocation(water_shader->get_prog(), "y_scale"), y_scale);

	//水中テクスチャデータの送信
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, underwater_tex);
	glUniform1i(glGetUniformLocation(water_shader->get_prog(), "buf_tex"), 2);

	//こースティックマップデータの送信
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, caustics_map);
	glUniform1i(glGetUniformLocation(water_shader->get_prog(), "cau_map"), 3);

	//質感の設定
	GLfloat tcolor[4] = {1.0, 1.0, 1.0, 0.1}, scolor[4] = {1.0, 1.0, 1.0, 1.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tcolor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, scolor);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

	glActiveTexture(GL_TEXTURE0);

	draw_water_surface();

	glUseProgram(0);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}


void WaterSurfaceCPU::draw_water_surface()
{
	const double dx = (double)width / (SURF_RESOLUTION_X - 1);
	const double dy = (double)height / (SURF_RESOLUTION_Y - 1);
	glBegin(GL_QUADS);
	for(int nx = 0; nx < SURF_RESOLUTION_X - 1; ++nx)
	{
		for(int ny = 0; ny < SURF_RESOLUTION_Y - 1; ++ny)
		{
			double x = nx * dx;
			double y = ny * dy;
			//法線ベクトルの設定
			vec3d norm;
			auto set_norm = [&](int ix, int iy) {
				norm = cross_vec3d(
					vec3d(2 * dx, 2 * dy, surf_vertex_z[2][ix + 1][iy + 1] - surf_vertex_z[2][ix - 1][iy - 1]),
					vec3d(2 * dx, -2 * dy, surf_vertex_z[2][ix + 1][iy - 1] - surf_vertex_z[2][ix - 1][iy + 1])).norm();
				glNormal3d(norm.x, norm.y, norm.z);
			};

			// 法線＋テクスチャ＋頂点座標の設定
			// （x,yインデックスは1つずつずれて頂点位置に対応）
			set_norm(nx + 1, ny + 1);
			glTexCoord2d(x / width, y / height);
			glVertex3d(x, y, surf_vertex_z[2][nx + 1][ny + 1]);

			set_norm(nx + 2, ny + 1);
			glTexCoord2d((x + dx) / width, y / height);
			glVertex3d(x + dx, y, surf_vertex_z[2][nx + 2][ny + 1]);

			set_norm(nx + 2, ny + 2);
			glTexCoord2d((x + dx) / width, (y + dy) / height);
			glVertex3d(x + dx, y + dy, surf_vertex_z[2][nx + 2][ny + 2]);

			set_norm(nx + 1, ny + 2);
			glTexCoord2d(x / width, (y + dy) / height);
			glVertex3d(x, y + dy, surf_vertex_z[2][nx + 1][ny + 2]);
		}
	}
	glEnd();
}


void WaterSurfaceCPU::mouse_action(double mouse_x, double mouse_y)
{
	const double dx = (double)width / (SURF_RESOLUTION_X - 1);
	const double dy = (double)height / (SURF_RESOLUTION_Y - 1);
	//---マウス操作による波の作成---
	//行列を取得（モデルビュー／射影）
	GLdouble mvM[16], prM[16];
	GLint vpM[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, mvM);
	glGetDoublev(GL_PROJECTION_MATRIX, prM);
	glGetIntegerv(GL_VIEWPORT, vpM);
	double mx = mouse_x;
	double my = mouse_y;
	vec3d np, fp;
	//near面上の座標
	gluUnProject(mx, height - my, 0, mvM, prM, vpM, &np.x, &np.y, &np.z);
	gluUnProject(mx, height - my, 1, mvM, prM, vpM, &fp.x, &fp.y, &fp.z);
	double st_depth = 20.0;
	vec3d point = np + (fp - np) * (st_depth - np.z) / (fp.z - np.z);
	int px = point.x / dx + 1;
	int py = point.y / dy + 1;
	auto f_max = [](int a, int b){ return a > b ? a : b; };
	auto f_min = [](int a, int b){ return a < b ? a : b; };
	constexpr int N = 2;
	for(int i = f_max(1, px - N); i <= f_min(SURF_RESOLUTION_X, px + N); ++i)
	{
		for(int j = f_max(1, py - N); j <= f_min(SURF_RESOLUTION_Y, py + N); ++j)
		{
			//	surf_vertex_z[0][i][j] = 20 + 10.0 * sin(3.141592 / (2 * N));
			//	surf_vertex_z[1][i][j] = 20 + 10.0 * sin(3.141592 / (2 * N));
			surf_vertex_z[2][i][j] = 20 + 10.0 * sin(3.141592 / (2 * N));
		}
	}
}

