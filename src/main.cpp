
#include <stdlib.h>
#include <time.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <chrono>
#include <thread>
#include "world.h"


//Worldの実体（updateとdisplayで使うためグローバル）
World* p_world;

const int FPS_micro = 1000000.0 / 60; //60 FPS
std::chrono::time_point<std::chrono::system_clock> last_time;


static void display()
{
	p_world->render();
}

static void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '\033': //ESC
		exit(1);
	default:
		break;
	}
}

static void update()
{
	p_world->update();

	//更新のたびに再描画を行う
	glutPostRedisplay();

	auto current_time = std::chrono::system_clock::now();

	//指定したFPSにフレームレートを維持
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_time).count();
	if (duration < FPS_micro)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(FPS_micro - duration));
	}

	last_time = current_time;

}

int main(int argc, char *argv[])
{
	//乱数のseedを初期化
	srand((unsigned)time(NULL));

	//ワールドサイズ
	int w = 640; int h = 640;

	//glutの初期化およびwindowの定義
	glutInit(&argc, argv);
	//ALPHA値有効，ダブルバッファリング，Zバッファ有効
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	//ウィンドウサイズ，タイトルの設定
	glutInitWindowSize(w, h);
	glutCreateWindow("osakana_prj");
	//
	glEnable(GL_DEPTH_TEST);

// /* -- 3D --
	//射影行列を設定
	glMatrixMode(GL_PROJECTION);
	//単位行列に初期化
	glLoadIdentity();
	//視界の設定
	gluPerspective(60.0, (double)w / h, 0.1, 1000.0);
	//視点
	gluLookAt(
		320.0, 0.0, 500.0, // 視点の位置x,y,z;
		320.0, 320.0, 0.0,   // 視界の中心位置の参照点座標x,y,z
		0.0, 0.0, 1.0);  //視界の上方向のベクトルx,y,z
// */

 /* -- 2D --
	//射影行列を設定
	glMatrixMode(GL_PROJECTION);
	//単位行列に初期化
	glLoadIdentity();
	//画面の座標をワールド座標に対応
	glOrtho(0.0, w, h, 0.0, -1.0, 1.0);
	//ビューポート変換
	glViewport(0, 0, w, h);
 */
	//クリアカラーの設定（水色）
	glClearColor(240.0 / 255, 248.0 / 255, 1.0, 0);
	//各コールバック関数の設定
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(update);
	//αテストの判定値
	glAlphaFunc(GL_GEQUAL, 0.5);
	//光源の作成
	GLfloat light_position0[] = {320.0, 320.0, 30.0, 1.0}; //光源0の座標
	const GLfloat lightcol[] = {1.0f, 1.0f, 1.0f, 1.0f }; // 直接光強度
	const GLfloat lightamb[] = {0.3f, 0.3f, 0.3f, 1.0f}; // 環境光強度
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightcol);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightcol);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	//スムースシェーディング（テスト）
	glShadeModel(GL_SMOOTH);
	//両面表示
	glEnable(GL_CULL_FACE);

	last_time = std::chrono::system_clock::now();

	printf("version : %s\n", glGetString(GL_VERSION));

	// GLEW初期化
	GLenum glewStatus = glewInit();
	if(glewStatus != GLEW_OK)
	{
		printf("Error: %s", glewGetErrorString(glewStatus));
		std::exit(1);
	}

	p_world = new World(w, h);

	//メインループの実行
	glutMainLoop();

	return 0;
}
