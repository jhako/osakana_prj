#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <chrono>
#include <thread>
#include "world.h"
#include "vision.h"
#include "Labeling.h"

#define POINTNUM 10

//Worldの実体（updateとdisplayで使うためグローバル）
World* p_world;

//Perspectiveの実体
Pers *p_pers;

//FPS（フレーム更新数）
const int FPS_micro = 1000000.0 / 60; //60 FPS
std::chrono::time_point<std::chrono::system_clock> last_time;

//OpenCV用
cv::VideoCapture cap(1);

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

void mouse(int button, int state, int x, int y)
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

void motion(int x, int y)
{
	p_world->set_mouse_pos(vec2d(x, y));
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

	//カメラデータの取得
	cv::Mat frame, mid;
	cap >> frame;
	std::vector<cv::Vec3f> circles;

	//トラックバーの値の取得
	
	//int dp = cv::getTrackbarPos("dp", "Capture");
	//int minDist = cv::getTrackbarPos("minDist", "Capture");
	//int param1 = cv::getTrackbarPos("param1", "Capture");
	//int param2 = cv::getTrackbarPos("param2", "Capture");
	//int minRadius = cv::getTrackbarPos("minRadius", "Capture");
	//int maxRadius = cv::getTrackbarPos("maxRadius", "Capture");
	int thresh = cv::getTrackbarPos("thresh", "Capture");

	if(p_pers->get_vector_size() == 4){
	  p_pers->perspective(&frame, &mid); //透視変換
	  cv::Mat dst(mid.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	  //detect_shadow(mid, dst, thresh);
	  //myHoughCircles(dst, circles, (double)dp/50.0, (double)minDist/10.0, param1, param2, minRadius, maxRadius); //円を検出し、表示する
	  
	  cv::cvtColor(mid, dst, CV_RGB2GRAY);
	  cv::threshold(dst, dst, thresh, 255, cv::THRESH_BINARY);
	  dst = ~dst;
	  cv::Mat label(dst.size(), CV_16SC1);
	  LabelingBS labeling;
	  labeling.Exec(dst.data, (short *)label.data, dst.cols, dst.rows, false, 0);
	  // ラベリング結果を出力する、真っ白な状態で初期化
	  cv::Mat outimg(dst.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	  cv::Mat labelarea;
	  cv::compare(label, 1, labelarea, CV_CMP_EQ);
	  cv::Mat color(dst.size(), CV_8UC3, cv::Scalar(0, 0, 0));
	  color.copyTo(outimg, labelarea);
	  //std::cout << outimg.rows << " " << outimg.cols << " " << outimg.channels() << std::endl;
	  //PrintDepth(outimg.depth());
	  std::vector<cv::Point> shadow_points;
	  int count = 0;
	  for(int j = 0; j < 1000; j++){
	    int x = rand() % 640;
	    int y = rand() % 640;
	    //std::cout << (int) outimg.at<cv::Vec3b>(x, y)[0] << std::endl; 
	    if((int) outimg.at<cv::Vec3b>(x, y)[0] == 0){
	      std::cout << '(' << x << ',' << y << ')';
	      shadow_points.push_back(cv::Point(x, y));
	      count++;
	    }
	    if(count > POINTNUM) break;
	  }
	  std::cout << std::endl;
	  cv::imshow("Destination", outimg);
	  cv::imshow("Capture", frame);
	}else{
	  cv::imshow("Capture", frame);
	}
	cv::waitKey(10);

}

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
	glEnable(GL_DEPTH_TEST);
/*
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
	p_pers = new Pers();

	//--OpenCVの設定--
	//カメラの設定
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	if(!cap.isOpened()) return -1;
	//ウィンドウの作成
	cv::namedWindow("Capture", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	//トラックバーの作成

	int slider_value_low = 100;
	//int slider_value_high = 100;
	//cv::createTrackbar("dp", "Capture", &slider_value_low, 100);
	//cv::createTrackbar("minDist", "Capture", &slider_value_high, 100);
	//cv::createTrackbar("param1", "Capture", &slider_value_low, 100);
	//cv::createTrackbar("param2", "Capture", &slider_value_high, 100);
	//cv::createTrackbar("minRadius", "Capture", &slider_value_low, 100);
	//cv::createTrackbar("maxRadius", "Capture", &slider_value_high, 100);
	cv::createTrackbar("thresh", "Capture", &slider_value_low, 255);

	//マウスコールバックの設定
	//cv::setMouseCallback("Capture", p_pers->onMouse, 0);
	cv::setMouseCallback("Capture", cv_onMouse, p_pers);
	//メインループの実行
	glutMainLoop();

	return 0;
}
