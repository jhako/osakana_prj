

#pragma once





class MyShader
{
public:
	MyShader(char* vtxShdName, char* frgShdName);

	int load_shader(char* vtxShdName, char* frgShdName);

	GLuint get_prog() const { return lpProg; }

private:

	GLuint lpProg;

	int load_and_compile(GLuint shader, char *name);
	void printProgramInfoLog(GLuint program);
	void printShaderInfoLog(GLuint shader);
};