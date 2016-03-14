

#pragma once


class MyShader
{
public:
	MyShader(const char* vtxShdName,
		const char* frgShdName,
		const char* tessConShdName = nullptr,
		const char* tessEvaShdName = nullptr,
		const char* geoShdName = nullptr);

	int load_shader(const char* vtxShdName,
		const char* frgShdName,
		const char* tessConShdName = nullptr,
		const char* tessEvaShdName = nullptr,
		const char* geoShdName = nullptr);

	GLuint get_prog() const { return lpProg; }

private:

	GLuint lpProg;

	int load_and_compile(GLuint shader, const char *name);
	void printProgramInfoLog(GLuint program);
	void printShaderInfoLog(GLuint shader);
};

