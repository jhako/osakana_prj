
//プロトタイプ宣言
namespace cv
{
    class Mat;
}

//--画像データの管理クラス--
class TexImage
{
    //画像データ
    cv::Mat* img;
public:
    TexImage(const char* fn);
    ~TexImage();

    //描画
    void render(int x, int y);
    void render(int x, int y, double rad); //回転角指定あり

    void resize(double a);
};
