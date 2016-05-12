

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"

MyShader::MyShader(
	const char* vtxShdName,
	const char* frgShdName,
	const char* tessConShdName,
	const char* tessEvaShdName,
	const char* geoShdName)
{
	load_shader(vtxShdName, frgShdName, tessConShdName, tessEvaShdName, geoShdName);
}

int MyShader::load_shader(
	const char* vtxShdName,
	const char* frgShdName,
	const char* tessConShdName,
	const char* tessEvaShdName,
	const char* geoShdName)
{
	GLuint vtxShader;
	GLuint frgShader;
	GLuint tessConShader;
	GLuint tessEvaShader;
	GLuint geoShader;
	GLuint prog;
	GLint linked;

	// プログラムオブジェクトの作成
	prog = glCreateProgram();

	//シェーダオブジェクトの作成＆コンパイル＆登録＆削除
	vtxShader = glCreateShader(GL_VERTEX_SHADER);
	if(load_and_compile(vtxShader, vtxShdName) < 0)
	{
		return -1;
	}
	glAttachShader(prog, vtxShader);
	glDeleteShader(vtxShader);

	frgShader = glCreateShader(GL_FRAGMENT_SHADER);
	if(load_and_compile(frgShader, frgShdName) < 0)
	{
		return -1;
	}
	glAttachShader(prog, frgShader);
	glDeleteShader(frgShader);

	if(tessConShdName != nullptr)
	{
		tessConShader = glCreateShader(GL_TESS_CONTROL_SHADER);
		if(load_and_compile(tessConShader, tessConShdName) < 0)
		{
			return -1;
		}
		glAttachShader(prog, tessConShader);
		glDeleteShader(tessConShader);
	}
	if(tessEvaShdName != nullptr)
	{
		tessEvaShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
		if(load_and_compile(tessEvaShader, tessEvaShdName) < 0)
		{
			return -1;
		}
		glAttachShader(prog, tessEvaShader);
		glDeleteShader(tessEvaShader);
	}
	if(geoShdName != nullptr)
	{
		geoShader = glCreateShader(GL_GEOMETRY_SHADER);
		if(load_and_compile(geoShader, geoShdName) < 0)
		{
			return -1;
		}
		glAttachShader(prog, geoShader);
		glDeleteShader(geoShader);
	}

	// シェーダプログラムのリンク 
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	printProgramInfoLog(prog);
	if(linked == GL_FALSE)
	{
		fprintf(stderr, "Link error of %s & %s or other shaders!!\n", vtxShdName, frgShdName);
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
	fprintf(stderr, "%s has been compiled. \n", name);
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
