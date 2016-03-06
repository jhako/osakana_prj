#include <opencv/cv.h>

void colorExtraction(cv::Mat*, cv::Mat*, int, int, int, int, int, int, int); 


class Pers{
 private:
  std::vector<int> pos_x, pos_y;
 public:
  Pers();
  void perspective(cv::Mat*, cv::Mat*);
  void onMouse(int, int, int, int, void*);
  void printpos();
};

