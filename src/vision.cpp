#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "vision.h"

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

void perspective(cv::Mat* src, cv::Mat* dst)
{
  cv::Point2f pts1[] = {cv::Point2f(150,150),cv::Point2f(150,300),cv::Point2f(350,300),cv::Point2f(350,150)};
  cv::Point2f pts2[] = {cv::Point2f(200,150),cv::Point2f(200,300),cv::Point2f(340,270),cv::Point2f(340,180)};

  //cv::Point2f pts1[] = {cv::Point2f(150,150.),cv::Point2f(150,300.),cv::Point2f(350,300.),cv::Point2f(350,150.)};
  //cv::Point2f pts2[] = {cv::Point2f(200,200.),cv::Point2f(150,300.),cv::Point2f(350,300.),cv::Point2f(300,200.)};

  // 透視変換行列を計算
  cv::Mat perspective_matrix = cv::getPerspectiveTransform(pts1, pts2);
  // 変換
  cv::warpPerspective(*src, *dst, perspective_matrix, (*src).size(), cv::INTER_LINEAR);  
  return;
}

/*
void onMouse( int event, int x, int y, int flag, void* )
{
  std::string desc;

  // マウスイベントを取得
  switch(event) {
  case cv::EVENT_MOUSEMOVE:
    desc += "MOUSE_MOVE";
    break;
  case cv::EVENT_LBUTTONDOWN:
    desc += "LBUTTON_DOWN";
    break;
  case cv::EVENT_RBUTTONDOWN:
    desc += "RBUTTON_DOWN";
    break;
  case cv::EVENT_MBUTTONDOWN:
    desc += "MBUTTON_DOWN";
    break;
  case cv::EVENT_LBUTTONUP:
    desc += "LBUTTON_UP";
    break;
  case cv::EVENT_RBUTTONUP:
    desc += "RBUTTON_UP";
    break;
  case cv::EVENT_MBUTTONUP:
    desc += "MBUTTON_UP";
    break;
  case cv::EVENT_LBUTTONDBLCLK:
    desc += "LBUTTON_DBLCLK";
    break;
  case cv::EVENT_RBUTTONDBLCLK:
    desc += "RBUTTON_DBLCLK";
    break;
  case cv::EVENT_MBUTTONDBLCLK:
    desc += "MBUTTON_DBLCLK";
    break;
  }

  // マウスボタン，及び修飾キーを取得
  if(flag & cv::EVENT_FLAG_LBUTTON)
    desc += " + LBUTTON";
  if(flag & cv::EVENT_FLAG_RBUTTON)
    desc += " + RBUTTON";
  if(flag & cv::EVENT_FLAG_MBUTTON)
    desc += " + MBUTTON";
  if(flag & cv::EVENT_FLAG_CTRLKEY)
    desc += " + CTRL";
  if(flag & cv::EVENT_FLAG_SHIFTKEY)
    desc += " + SHIFT";
  if(flag & cv::EVENT_FLAG_ALTKEY)
    desc += " + ALT";

  std::cout << desc << " (" << x << ", " << y << ")" << std::endl;
}
*/
