#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "vision.h"

#define OPENCV_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define OPENCV_VERSION_CODE OPENCV_VERSION(CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION)

#if OPENCV_VERSION_CODE < OPENCV_VERSION(2,3,1)
namespace cv
{
  enum {
    EVENT_MOUSEMOVE      =CV_EVENT_MOUSEMOVE,
    EVENT_LBUTTONDOWN    =CV_EVENT_LBUTTONDOWN,
    EVENT_RBUTTONDOWN    =CV_EVENT_RBUTTONDOWN,
    EVENT_MBUTTONDOWN    =CV_EVENT_MBUTTONDOWN,
    EVENT_LBUTTONUP      =CV_EVENT_LBUTTONUP,
    EVENT_RBUTTONUP      =CV_EVENT_RBUTTONUP,
    EVENT_MBUTTONUP      =CV_EVENT_MBUTTONUP,
    EVENT_LBUTTONDBLCLK  =CV_EVENT_LBUTTONDBLCLK,
    EVENT_RBUTTONDBLCLK  =CV_EVENT_RBUTTONDBLCLK,
    EVENT_MBUTTONDBLCLK  =CV_EVENT_MBUTTONDBLCLK
  };
  enum {
    EVENT_FLAG_LBUTTON   =CV_EVENT_FLAG_LBUTTON,
    EVENT_FLAG_RBUTTON   =CV_EVENT_FLAG_RBUTTON,
    EVENT_FLAG_MBUTTON   =CV_EVENT_FLAG_MBUTTON,
    EVENT_FLAG_CTRLKEY   =CV_EVENT_FLAG_CTRLKEY,
    EVENT_FLAG_SHIFTKEY  =CV_EVENT_FLAG_SHIFTKEY,
    EVENT_FLAG_ALTKEY    =CV_EVENT_FLAG_ALTKEY
  };
}
#endif

void colorExtraction(cv::Mat* src, cv::Mat* dst,
		     int code,
		     int ch1Lower, int ch1Upper,
		     int ch2Lower, int ch2Upper,
		     int ch3Lower, int ch3Upper
		     )
{
  
  cv::Mat colorImage;
  int lower[3];
  int upper[3];

  cv::Mat lut = cv::Mat(256, 1, CV_8UC3);   

  cv::cvtColor(*src, colorImage, code);

  lower[0] = ch1Lower;
  lower[1] = ch2Lower;
  lower[2] = ch3Lower;

  upper[0] = ch1Upper;
  upper[1] = ch2Upper;
  upper[2] = ch3Upper;

  for (int i = 0; i < 256; i++){
    for (int k = 0; k < 3; k++){
      if (lower[k] <= upper[k]){
	if ((lower[k] <= i) && (i <= upper[k])){
	  lut.data[i*lut.step+k] = 255;
	}else{
	  lut.data[i*lut.step+k] = 0;
	}
      }else{
	if ((i <= upper[k]) || (lower[k] <= i)){
	  lut.data[i*lut.step+k] = 255;
	}else{
	  lut.data[i*lut.step+k] = 0;
	}
      }
    }
  }

  //LUTを使用して二値化
  cv::LUT(colorImage, lut, colorImage);

  //Channel毎に分解
  std::vector<cv::Mat> planes;
  cv::split(colorImage, planes);

  //マスクを作成
  cv::Mat maskImage;
  cv::bitwise_and(planes[0], planes[1], maskImage);
  cv::bitwise_and(maskImage, planes[2], maskImage);

  //出力
  cv::Mat maskedImage;
  src->copyTo(maskedImage, maskImage);
  *dst = maskedImage;
}


Pers::Pers()
{
  //nothing to do;
}

int Pers::get_vector_size(){
  return pos_x.size();
}


void Pers::perspective(cv::Mat* src, cv::Mat* dst)
{
  std::cout << src->cols << " "  << src->rows << std::endl;
  if(get_vector_size() == 4){
    cv::Point2f pts1[] = {cv::Point2f(pos_x[0],pos_y[0]),cv::Point2f(pos_x[1],pos_y[1]),cv::Point2f(pos_x[2],pos_x[2]),cv::Point2f(pos_x[3],pos_y[3])};
    cv::Point2f pts2[] = {cv::Point2f(0,0),cv::Point2f(640,0),cv::Point2f(640,2400),cv::Point2f(0,480)};
    
    // 透視変換行列を計算
    cv::Mat perspective_matrix = cv::getPerspectiveTransform(pts1, pts2);
    // 変換
    cv::warpPerspective(*src, *dst, perspective_matrix, (*dst).size(), cv::INTER_LINEAR);
  }  
  return;
}


  void Pers::onMouse(int event, int x, int y, int flag, void*)
{
  // マウスイベントを取得
  if(event ==  cv::EVENT_LBUTTONDOWN && pos_x.size() < 4){
  pos_x.push_back(x);
  pos_y.push_back(y);
 }
  printpos();

  return;
}

void Pers::printpos()
{
  for(auto x: pos_x){
    std::cout << x;
  }
  std::cout << std::endl;
  for(auto y: pos_y){
    std::cout << y;
  }
  std::cout << std::endl;
  return;
}

std::vector<int> Pers::calc_center(cv::Mat* img)
{
  std::vector<int> center_pos(2, 0);
  int x, y, sum;
  cv::Mat ones_col = cv::Mat::ones(img->cols, 1, CV_8U);
  cv::Mat ones_row = cv::Mat::ones(img->rows, 1, CV_8U);
  cv::Mat cols = *img * ones_col;
  cv::Mat rows = img->t() * ones_row;
  sum = ((cv::Mat)(cols * ones_row)).at<unsigned char>(0, 0);
  center_pos[0] = sum;
  center_pos[1] = sum;
  return center_pos;
}

