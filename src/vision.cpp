#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

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
