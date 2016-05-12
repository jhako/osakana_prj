
#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <chrono>
#include <future>
#include <sstream>
#include "world.h"
#include "vision.h"
#include "util.h"
#include "Labeling.h"

#define POINTNUM 10


//ワールドサイズ
//int w = 640; int h = 640;
int w = 1024; int h = 768;

//Worldの実体（updateとdisplayで使うためグローバル）
World* p_world;

//Perspectiveの実体
Pers *p_pers;

/*
//FPS（フレーム更新数）
const int FPS_micro = 1000000.0 / 50; //50 FPS
std::chrono::time_point<std::chrono::system_clock> last_time;
*/


//OpenCV用
//#define USE_CAPTURE
#ifdef USE_CAPTURE
	cv::VideoCapture cap(0); //キャプチャ
	cv::Mat cv_tex; //CVWindowに表示するテクスチャ
#endif

//フルスクリーン
//#define FULL_SCREEN

//ミューテックス
std::mutex mtx;

//フラグ
bool flag_exit = false;

GLFWwindow* window;

static void keyboard(GLFWwindow* win, int key, int scancode, int action, int mods)
{
	switch(key)
	{
		case GLFW_KEY_ESCAPE: //ESC
			exit(EXIT_SUCCESS);
		default:
			break;
	}
}

static void mouse(GLFWwindow* win, int button, int action, int mods)
{
	double x, y;
	glfwGetCursorPos(win, &x, &y);
	p_world->set_mouse_pos(vec2d(x, y));
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		p_world->set_mouse_state(true);
	}
	else
	{
		p_world->set_mouse_state(false);
	}
	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		p_world->set_click_state_right(true);
	}
	else
	{
		p_world->set_click_state_right(false);
	}
}

static void motion(GLFWwindow* window, double x, double y)
{
	p_world->set_mouse_pos(vec2d(x, y));
}

static void update()
{

	//for debug
	static int debug_step = 0; const int DCount = 10;
	static std::chrono::time_point<std::chrono::system_clock> start_t, end_t;
	if(debug_step == 0)
		start_t = std::chrono::system_clock::now();


	//Worldのアップデート
	p_world->update();

	//更新のたびに再描画を行う
	p_world->render();
	glfwSwapBuffers(window);

	//GL関連のアップデート
	p_world->opengl_update();

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
	
	auto current_time = std::chrono::system_clock::now();

	/*
	//指定したFPSにフレームレートを維持
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_time).count();
	if(duration < FPS_micro)
	{
		//std::this_thread::sleep_for(std::chrono::microseconds(FPS_micro - duration));
	}
	last_time = current_time;
	*/
}


#ifdef USE_CAPTURE
//認識部分を別スレッドで行う
static void cv_update()
{
	//イベントループ
	while(true)
	{
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

		if(p_pers->get_vector_size() == 4)
		{
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
			//std::vector<cv::Point> shadow_points;
			int count = 0;
			for(int j = 0; j < 1000; j++)
			{
				int x = rand() % 640;
				int y = rand() % 640;
				//std::cout << (int) outimg.at<cv::Vec3b>(x, y)[0] << std::endl; 
				if((int)outimg.at<cv::Vec3b>(x, y)[0] == 0)
				{
					std::cout << '(' << x << ',' << y << ')';
					//shadow_points.push_back(cv::Point(x, y));
					p_world->add_to_noise_list(vec2d(x, y));
					count++;
				}
				if(count > POINTNUM) break;
			}
			std::cout << std::endl;
			cv::imshow("Destination", outimg);
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

static void error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;
	exit(EXIT_FAILURE);
}

static void __stdcall OutputGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);


int main(int argc, char *argv[])
{
	//乱数のseedを初期化
	srand((unsigned)time(NULL));

	//エラーコールバックの設定
	glfwSetErrorCallback(error_callback);

	//glfwの初期化およびwindowの定義
	glfwInit();


	//ALPHA値有効，ダブルバッファリング，Zバッファ有効
	//glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	//ウィンドウサイズ，タイトルの設定
	//glutInitWindowSize(w, h);
	//glutCreateWindow("osakana_prj");

	//デバッグの有効化
	/*
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	*/
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	//モニタの設定
	GLFWmonitor* monitor = nullptr;
#ifdef FULL_SCREEN
	int msize;
	GLFWmonitor** monitors;
	if((monitors = glfwGetMonitors(&msize)) == nullptr)
	{
		printf("Error: No monitor");
		std::exit(1);
	}
	//一番サブのモニターを指定
	monitor = monitors[msize - 1];
#endif
	//ウィンドウの作成
	window = glfwCreateWindow(w, h, "osakana_prj", monitor, nullptr);
	//glfwSetWindowPos(window, 100, 100);
	glfwMakeContextCurrent(window);

	// GLEW初期化
	GLenum glewStatus = glewInit();
	if(glewStatus != GLEW_OK)
	{
		printf("Error: %s", glewGetErrorString(glewStatus));
		std::exit(1);
	}

	if(GLEW_ARB_debug_output)
	{
		puts("enable : debug_output");
	}

	//デバッグコールバック
	glDebugMessageCallback(OutputGLDebugMessage, NULL);

	//垂直同期
	glfwSwapInterval(1);

	//クリアカラーの設定（水色）
	glClearColor(240.0 / 255, 248.0 / 255, 1.0, 0);

	//各コールバック関数の設定
	glfwSetKeyCallback(window, keyboard);
	glfwSetMouseButtonCallback(window, mouse);
	glfwSetCursorPosCallback(window, motion); //マウス移動

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
	//glShadeModel(GL_SMOOTH);
	//カリング
	glEnable(GL_CULL_FACE);
	/*
	//時間記録
	last_time = std::chrono::system_clock::now();
	*/
	//バージョン表示（デバッグ）
	printf("version : %s\n", glGetString(GL_VERSION));


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

	//OpenCVは非同期処理
	auto fut_cv = std::async(std::launch::async, []{ cv_update(); });
#endif

	//メインループの実行
	while(!glfwWindowShouldClose(window))
	{
		update();
		glfwPollEvents();
	}

	//終了処理（ただしGLUTだと無意味）
	{
		std::lock_guard<std::mutex> lock(mtx);
		flag_exit = true; //終了
	}

	return 0;
}



static void __stdcall OutputGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam)
{
	static const char* kSourceStrings[] = {
		"OpenGL API",
		"Window System",
		"Shader Compiler",
		"Third Party",
		"Application",
		"Other",
	};
	int sourceNo = (GL_DEBUG_SOURCE_API_ARB <= source && source <= GL_DEBUG_SOURCE_OTHER_ARB)
		? (source - GL_DEBUG_SOURCE_API_ARB)
		: (GL_DEBUG_SOURCE_OTHER_ARB - GL_DEBUG_SOURCE_API_ARB);

	static const char* kTypeStrings[] = {
		"Error",
		"Deprecated behavior",
		"Undefined behavior",
		"Portability",
		"Performance",
		"Other",
	};
	int typeNo = (GL_DEBUG_TYPE_ERROR_ARB <= type && type <= GL_DEBUG_TYPE_OTHER_ARB)
		? (type - GL_DEBUG_TYPE_ERROR_ARB)
		: (GL_DEBUG_TYPE_OTHER_ARB - GL_DEBUG_TYPE_ERROR_ARB);

	static const char* kSeverityStrings[] = {
		"High",
		"Medium",
		"Low",
	};
	int severityNo = (GL_DEBUG_SEVERITY_HIGH_ARB <= type && type <= GL_DEBUG_SEVERITY_LOW_ARB)
		? (type - GL_DEBUG_SEVERITY_HIGH_ARB)
		: (GL_DEBUG_SEVERITY_LOW_ARB - GL_DEBUG_SEVERITY_HIGH_ARB);

	printf("Source : %s    Type : %s    ID : %d    Severity : %s    \nMessage : %s\n"
		, kSourceStrings[sourceNo], kTypeStrings[typeNo], id, kSeverityStrings[severityNo], message);
}