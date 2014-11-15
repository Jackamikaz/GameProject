#ifndef __SHADER_PROGRAM_HPP__
#define __SHADER_PROGRAM_HPP__

#include "glheader.h"

class ShaderProgram {
public:
						ShaderProgram();
						ShaderProgram(const char* vsFile, const char* gsFile, const char* fsFile);
						ShaderProgram(const GLuint vs, const GLuint gs, const GLuint fs);
						~ShaderProgram();

	bool				Load(const char* vsFile, const char* gsFile, const char* fsFile);
	bool				Load(const GLuint vs, const GLuint gs, const GLuint fs);
	void				Destroy();

	inline bool			operator!(void) const {return (shaderID == 0);}
	inline				operator GLuint(void) const {return shaderID;}

private:
	GLuint shaderID;
};

GLuint CompileShader(const char* file, GLenum shadertype);

#endif//__SHADER_PROGRAM_HPP__
