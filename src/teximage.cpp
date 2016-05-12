

#include "teximage.h"
#include <opencv2/opencv.hpp>
//#include <opencv2/highgui.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
	printf("finish loading texture : %s (PB %d, CB %d)\n", fn, img->elemSize(), img->elemSize1());
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




TexImageWithShader::TexImageWithShader(const char * fn, int width_, int height_, GLuint shader_)
	:width(width_), height(height_), shader(shader_)
{
	load(fn);
}

TexImageWithShader::~TexImageWithShader()
{
	delete img;
}

void TexImageWithShader::load(const char * fn)
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
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	GLint i_format;
	GLenum format;
	if(img->elemSize() == 4)
	{
		alpha = true;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		i_format = GL_RGBA;
		format = GL_BGRA;
	}
	else
	{
		alpha = false;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
		i_format = GL_RGB;
		format = GL_BGR;
	}
	gluBuild2DMipmaps(GL_TEXTURE_2D, i_format, img->size().width, img->size().height, format, GL_UNSIGNED_BYTE, img->data);
	//glTexImage2D(GL_TEXTURE_2D, 0, i_format, img->size().width, img->size().height, 0, format, GL_UNSIGNED_BYTE, img->data);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	printf("finish loading texture : %s (PB %d, CB %d) -> %d\n", fn, img->elemSize(), img->elemSize1(), tex);
}

void TexImageWithShader::render(int x, int y, double rad)
{
	//glColor3f(1.0f, 1.0f, 1.0f);
	//glEnable(GL_TEXTURE_2D);
	//glNormal3d(0.0, 0.0, 1.0);

	glUseProgram(shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(glGetUniformLocation(shader, "tex"), 0);

	glBegin(GL_QUADS);
	//vec2d center = vec2d(x + width / 2.0, y + height / 2.0);
	vec2d center = vec2d(x, y);
	vec2d v;
	glTexCoord2d(0.0, 1.0);
	v = center + vec2d(-width / 2.0, -height / 2.0).rotate(rad);
	glVertex3d(v.x, v.y, 0.0);

	glTexCoord2d(1.0, 1.0);
	v = center + vec2d(width / 2.0, -height / 2.0).rotate(rad);
	glVertex3d(v.x, v.y, 0.0);

	glTexCoord2d(1.0, 0.0);
	v = center + vec2d(width / 2.0, height / 2.0).rotate(rad);
	glVertex3d(v.x, v.y, 0.0);

	glTexCoord2d(0.0, 0.0);
	v = center + vec2d(-width / 2.0, height / 2.0).rotate(rad);
	glVertex3d(v.x, v.y, 0.0);
	glEnd();

	//glDisable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}
