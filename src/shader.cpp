

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "shader.h"

MyShader::MyShader(const char * vtxShdName, const char * frgShdName)
{
	load_shader(vtxShdName, frgShdName);
}

int MyShader::load_shader(const char * vtxShdName, const char * frgShdName)
{
	GLuint vtxShader;
	GLuint frgShader;
	GLuint prog;
	GLint linked;

	/* シェーダオブジェクトの作成 */
	vtxShader = glCreateShader(GL_VERTEX_SHADER);
	frgShader = glCreateShader(GL_FRAGMENT_SHADER);

	/* バーテックスシェーダのロードとコンパイル */
	if(load_and_compile(vtxShader, vtxShdName) < 0)
	{
		return -1;
	}

	/* フラグメントシェーダのロードとコンパイル */
	if(load_and_compile(frgShader, frgShdName) < 0)
	{
		return -1;
	}

	/* プログラムオブジェクトの作成 */
	prog = glCreateProgram();

	/* シェーダオブジェクトのシェーダプログラムへの登録 */
	glAttachShader(prog, vtxShader);
	glAttachShader(prog, frgShader);

	/* シェーダオブジェクトの削除 */
	glDeleteShader(vtxShader);
	glDeleteShader(frgShader);

	/* シェーダプログラムのリンク */
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	printProgramInfoLog(prog);
	if(linked == GL_FALSE)
	{
		fprintf(stderr, "Link error of %s & %s!!\n", vtxShdName, frgShdName);
		return -1;
	}

	lpProg = prog;

	return 0;
}

int MyShader::load_and_compile(GLuint shader, const char * name)
{
	FILE *fp;
	void *buf;
	int size;
	GLint compiled;

	if((fp = fopen(name, "rb")) == NULL)
	{
		fprintf(stderr, "%s is not found!!\n", name);
		return -1;
	}

	/* ファイルサイズの取得 */
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	/* シェーダプログラムの読み込み領域を確保 */
	if((buf = (void *)malloc(size)) == NULL)
	{
		fprintf(stderr, "Memory is not enough for %s\n", name);
		fclose(fp);
		return -1;
	}

	/* ファイルを先頭から読み込む */
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1, size, fp);

	/* シェーダオブジェクトにプログラムをセット */
	glShaderSource(shader, 1, (const GLchar **)&buf, &size);

	/* シェーダ読み込み領域の解放 */
	free(buf);
	fclose(fp);

	/* シェーダのコンパイル */
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	printShaderInfoLog(shader);		/* コンパイルログの出力 */
	if(compiled == GL_FALSE)
	{
		fprintf(stderr, "Compile error in %s!!\n", name);
		return -1;
	}

	return 0;
}

void MyShader::printProgramInfoLog(GLuint program)
{
	int logSize;
	int length;

	/* ログの長さは、最後のNULL文字も含む */
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);

	if(logSize > 1)
	{
		GLchar *infoLog = new GLchar(logSize);
		glGetProgramInfoLog(program, logSize, &length, infoLog);
		fprintf(stderr, "Program Info Log\n%s\n", infoLog);
		delete infoLog;
	}
}

void MyShader::printShaderInfoLog(GLuint shader)
{
	int logSize;
	int length;

	/* ログの長さは、最後のNULL文字も含む */
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

	if(logSize > 1)
	{
		GLchar *infoLog = new GLchar(logSize);
		glGetShaderInfoLog(shader, logSize, &length, infoLog);
		fprintf(stderr, "Shader Info Log\n%s\n", infoLog);
		delete infoLog;
	}
}
