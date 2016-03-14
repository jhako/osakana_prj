
#include <stdlib.h>
#include <time.h>
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
	p_world = new World();
	int w = p_world->get_width();
	int h = p_world->get_height();

	//glutの初期化およびwindowの定義
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(w, h);
	glutCreateWindow("BOIDS");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, w, h, 0.0);
	glViewport(0, 0, w, h);
	glClearColor(240.0 / 255, 248.0 / 255, 1.0, 0);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(update);
	last_time = std::chrono::system_clock::now();

	//メインループの実行
	glutMainLoop();

	return 0;
}
