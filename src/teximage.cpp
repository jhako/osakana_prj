

#include "teximage.h"
#include <opencv2/opencv.hpp>
//#include <opencv2/highgui.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "vec2d.h"

#ifndef GL_BGRA
#ifdef GL_BGRA_EXT
#define GL_BGRA GL_BGRA_EXT
#else
#define GL_BGRA 0x80E1
#endif
#endif


TexImage::TexImage(const char* fn)
{
	load(fn);
}

TexImage::~TexImage()
{
	delete img;
}

void TexImage::load(const char * fn)
{
	//ファイルを読み込み、cv::Matに変換
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	img = new cv::Mat();
	*img = cv::imread(fn, -1);
	if(img->empty())
	{
		printf("ERROR : 画像の読み込みに失敗 , %s\n", fn);
	}
	cv::flip(*img, *img, 0);
	if(img->elemSize() == 4)
	{
		alpha = true;
	}
	else
	{
		alpha = false;
	}
	printf("%s : %d, %d\n", fn, img->elemSize(), img->elemSize1());
}

//cv::Matのデータを用いて、テクスチャとしてOpenGLで描画する
void TexImage::render(int x, int y)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	if(alpha)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->size().width, img->size().height, 0, GL_BGRA, GL_UNSIGNED_BYTE, img->data);
	}
	else
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->size().width, img->size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glEnable(GL_TEXTURE_2D);
	glNormal3d(0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 1.0);
	glVertex3d(x, y, 0.0);
	glTexCoord2d(1.0, 1.0);
	glVertex3d(x + img->size().width, y, 0.0);
	glTexCoord2d(1.0, 0.0);
	glVertex3d(x + img->size().width, y + img->size().height, 0.0);
	glTexCoord2d(0.0, 0.0);
	glVertex3d(x, y + img->size().height, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

//回転あり
void TexImage::render(int x, int y, double rad)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	if(alpha)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->size().width, img->size().height, 0, GL_BGRA, GL_UNSIGNED_BYTE, img->data);
	}
	else
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->size().width, img->size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glEnable(GL_TEXTURE_2D);
	glNormal3d(0.0, 0.0, 1.0);
	glBegin(GL_QUADS);

	vec2d center = vec2d(x + img->size().width / 2.0, y + img->size().height / 2.0);
	vec2d v;
	glTexCoord2d(0.0, 1.0);
	v = center + vec2d(-img->size().width / 2.0, -img->size().height / 2.0).rotate(rad);
	glVertex3d(v.x, v.y, 0.0);

	glTexCoord2d(1.0, 1.0);
	v = center + vec2d(img->size().width / 2.0, -img->size().height / 2.0).rotate(rad);
	glVertex3d(v.x, v.y, 0.0);

	glTexCoord2d(1.0, 0.0);
	v = center + vec2d(img->size().width / 2.0, img->size().height / 2.0).rotate(rad);
	glVertex3d(v.x, v.y, 0.0);

	glTexCoord2d(0.0, 0.0);
	v = center + vec2d(-img->size().width / 2.0, img->size().height / 2.0).rotate(rad);
	glVertex3d(v.x, v.y, 0.0);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void TexImage::render(int x1, int y1, int x2, int y2)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	if(alpha)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->size().width, img->size().height, 0, GL_BGRA, GL_UNSIGNED_BYTE, img->data);
	}
	else
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->size().width, img->size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glEnable(GL_TEXTURE_2D);
	glNormal3d(0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 1.0);
	glVertex3d(x1, y1, 0.0);
	glTexCoord2d(1.0, 1.0);
	glVertex3d(x2, y1, 0.0);
	glTexCoord2d(1.0, 0.0);
	glVertex3d(x2, y2, 0.0);
	glTexCoord2d(0.0, 0.0);
	glVertex3d(x1, y2, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void TexImage::resize(double a)
{
	cv::Mat *tmp = new cv::Mat();
	cv::resize(*img, *tmp, cv::Size(), a, a);
	delete img;
	img = tmp;
}

unsigned char* TexImage::get_data()
{
	return img->data;
}
