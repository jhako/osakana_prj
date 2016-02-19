
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
	//α値があるか
	bool alpha;
public:
	TexImage(const char* fn);
	TexImage(){}
	~TexImage();

	void load(const char* fn);

	//描画
	void render(int x, int y);
	void render(int x, int y, double rad); //回転角指定あり
	void render(int x1, int y1, int x2, int y2); //任意の座標への描画

	void resize(double a);

	unsigned char*	get_data();
};
