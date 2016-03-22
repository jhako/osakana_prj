
#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <chrono>
#include <future>
#include "world.h"
#include "vision.h"

//Worldの実体（updateとdisplayで使うためグローバル）
World* p_world;

//Perspectiveの実体
Pers *p_pers;

//FPS（フレーム更新数）
const int FPS_micro = 1000000.0 / 50; //50 FPS
std::chrono::time_point<std::chrono::system_clock> last_time;

//OpenCV用
#define USE_CAPTURE
#ifdef USE_CAPTURE
	cv::VideoCapture cap(0); //キャプチャ
	cv::Mat cv_tex; //CVWindowに表示するテクスチャ
#endif

//ミューテックス
std::mutex mtx;

//フラグ
bool flag_exit = false;

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

static void mouse(int button, int state, int x, int y)
{
	p_world->set_mouse_pos(vec2d(x, y));
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		p_world->set_mouse_state(true);
	}
	else
	{
		p_world->set_mouse_state(false);
	}
}

static void motion(int x, int y)
{
	p_world->set_mouse_pos(vec2d(x, y));
}

static void update()
{
/*
	//for debug
	static int debug_step = 0; const int DCount = 10;
	static std::chrono::time_point<std::chrono::system_clock> start_t, end_t;
	if(debug_step == 0)
		start_t = std::chrono::system_clock::now();
*/

	//Worldのアップデート
	p_world->update();

	//更新のたびに再描画を行う
	glutPostRedisplay();

	//GL関連のアップデート
	p_world->opengl_update();

/*
	//for debug
	if(debug_step == DCount)
	{
		end_t = std::chrono::system_clock::now();
		auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(end_t - start_t).count();
		std::cout << "time-span : " << ts << ",  FPS : " << 1000.0 * DCount / ts << std::endl;
		debug_step = 0;
	}
	else
		++debug_step;
*/

	auto current_time = std::chrono::system_clock::now();

	//指定したFPSにフレームレートを維持
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_time).count();
	if(duration < FPS_micro)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(FPS_micro - duration));
	}
	last_time = current_time;
}

#ifdef USE_CAPTURE
//認識部分を別スレッドで行う
static void cv_update()
{
	//イベントループ
	while(true)
	{
		//カメラデータの取得
		cv::Mat frame;
		cv::Mat tmp_frame;
		cv::Mat dst;
		cap >> frame;

		//トラックバーの値の取得
		int hue_min = cv::getTrackbarPos("Hue min", "Capture");
		int hue_max = cv::getTrackbarPos("Hue max", "Capture");
		int satulation_min = cv::getTrackbarPos("Satulation min", "Capture");
		int satulation_max = cv::getTrackbarPos("Satulation max", "Capture");
		int value_min = cv::getTrackbarPos("Value min", "Capture");
		int value_max = cv::getTrackbarPos("Value max", "Capture");

		if(p_pers->get_vector_size() == 4){
			p_pers->perspective(&frame, &tmp_frame); //透視変換
			colorExtraction(&tmp_frame, &dst, CV_BGR2HSV, hue_min, hue_max, satulation_min, satulation_max, value_min, value_max);//色抽出
			cv::imshow("Destination", dst);
			cv::imshow("Capture", frame);
		}
		else{
			cv::imshow("Capture", frame);
		}
		cv::waitKey(10);

		{
			std::lock_guard<std::mutex> lock(mtx);
			if(flag_exit)
			{
				break;
			}
		}
	}
}
#endif


static void cv_onMouse(int event, int x, int y, int flag, void* param)
{
  Pers* pers = static_cast<Pers*>(param);
  pers->onMouse(event, x, y, flag, nullptr);
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
//	glEnable(GL_DEPTH_TEST);
	//クリアカラーの設定（水色）
	glClearColor(240.0 / 255, 248.0 / 255, 1.0, 0);
	//各コールバック関数の設定
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion); //マウス移動
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
	//時間記録
	last_time = std::chrono::system_clock::now();
	//バージョン表示（デバッグ）
	printf("version : %s\n", glGetString(GL_VERSION));

	// GLEW初期化
	GLenum glewStatus = glewInit();
	if(glewStatus != GLEW_OK)
	{
		printf("Error: %s", glewGetErrorString(glewStatus));
		std::exit(1);
	}

	p_world = new World(w, h);

#ifdef USE_CAPTURE
	p_pers = new Pers();

	//--OpenCVの設定--
	//カメラの設定
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	if(!cap.isOpened()) return -1;
	//ウィンドウの作成
	cv::namedWindow("Capture", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("Destination", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	//トラックバーの作成
	int slider_value_low = 0;
	int slider_value_high = 180;
	cv::createTrackbar("Hue min", "Capture", &slider_value_low, 180);
	cv::createTrackbar("Hue max", "Capture", &slider_value_high, 180);
	cv::createTrackbar("Satulation min", "Capture", &slider_value_low, 180);
	cv::createTrackbar("Satulation max", "Capture", &slider_value_high, 180);
	cv::createTrackbar("Value min", "Capture", &slider_value_low, 180);
	cv::createTrackbar("Value max", "Capture", &slider_value_high, 180);

	//マウスコールバックの設定
	//cv::setMouseCallback("Capture", p_pers->onMouse, 0);
	cv::setMouseCallback("Capture", cv_onMouse, p_pers);

	//OpenCVは非同期処理
	auto fut_cv = std::async(std::launch::async, []{ cv_update(); });
#endif

	//メインループの実行
	glutMainLoop();

	//終了処理（ただしGLUTだと無意味）
	{
		std::lock_guard<std::mutex> lock(mtx);
		flag_exit = true; //終了
	}

	return 0;
}
