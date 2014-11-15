#include "ShaderProgram.hpp"
#include "syseng.hpp"
#include <iostream>
#include <fstream>

ShaderProgram::ShaderProgram()
:	shaderID(0)
{
}

ShaderProgram::ShaderProgram(const char* vsFile, const char* gsFile, const char* fsFile)
:	shaderID(0)
{
	Load(vsFile,gsFile,fsFile);
}

ShaderProgram::ShaderProgram(const GLuint vs, const GLuint gs, const GLuint fs)
:	shaderID(0)
{
	Load(vs,gs,fs);
}

ShaderProgram::~ShaderProgram() {
	if (shaderID != 0) {
		Destroy();
	}
}

bool ShaderProgram::Load(const char* vsFile, const char* gsFile, const char* fsFile) {
	SEASSERT(vsFile || gsFile || fsFile); //at least one file should be here

	// one : compile the shaders
	GLuint vs = 0, gs = 0, fs = 0;
	if (vsFile) {vs = CompileShader(vsFile, GL_VERTEX_SHADER);}
	if (gsFile) {gs = CompileShader(gsFile, GL_GEOMETRY_SHADER);}
	if (fsFile) {fs = CompileShader(fsFile, GL_FRAGMENT_SHADER);}

	if ((vsFile && !vs) || (gsFile && !gs) || (fsFile && !fs)) {
		if (vs) {glDeleteShader(vs);}
		if (gs) {glDeleteShader(gs);}
		if (fs) {glDeleteShader(fs);}
		SysEng::Log(SysEng::error, "Shader creation failed. Aborting program creation.");
		return false;
	}

	Load(vs,gs,fs);

	// five : get rid of the shader objects
	if (vs) {
		glDetachShader(shaderID,vs);
		glDeleteShader(vs);
	}

	if (gs) {
		glDetachShader(shaderID,gs);
		glDeleteShader(gs);
	}

	if (fs) {
		glDetachShader(shaderID,fs);
		glDeleteShader(fs);
	}

	return true;
}

bool ShaderProgram::Load(const GLuint vs, const GLuint gs, const GLuint fs) {
	SEASSERT(vs || gs || fs);

	// two : attach the shaders to a newly created program
	shaderID = glCreateProgram();
	if (vs) {glAttachShader(shaderID, vs);}
	if (gs) {glAttachShader(shaderID, gs);}
	if (fs) {glAttachShader(shaderID, fs);}

	// three : link
	glLinkProgram(shaderID);

	// four : error checks
	GLint success = 0;
	GLchar errorLog[1024] = { 0 };

	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
	if (success == 0) {
		glGetProgramInfoLog(shaderID, sizeof(errorLog), 0, errorLog);
		SysEng::Error("Error linking shader program:\n%s", errorLog);
		Destroy();
		return false;
	}

	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderID, sizeof(errorLog), 0, errorLog);
		SysEng::Error("Invalid shader program:\n%s", errorLog);
		Destroy();
		return false;
	}

	return true;
}

void ShaderProgram::Destroy() {
	if (shaderID) {
		glDeleteProgram(shaderID);
		shaderID = 0;
	}
}


GLuint CompileShader(const char* file, GLenum shadertype) {
	// one: create shader
	GLuint shobj = glCreateShader(shadertype);

	if (shobj == 0) {
		SysEng::Log(SysEng::error ,"Error creating shader \"%s\"", file);
		return 0;
	}

	// two: read shader file
	std::ifstream f(file);
	std::string source;

	if (f.is_open()) {
			std::string line;
			while (getline(f, line)) {
				source.append(line);
				source.append("\n");
			}

		f.close();
	}
	else {
		SysEng::Log(SysEng::error ,"Could not read file \"%s\"", file);
		return 0;
	}

	// three: feed the source
	const GLchar* p[1];
	p[0] = source.c_str();
	GLint Lengths[1];
	Lengths[0]= source.length();
	glShaderSource(shobj, 1, p, Lengths);

	// five: compile!
	glCompileShader(shobj);

	// six: compile error checking
	GLint success;
	glGetShaderiv(shobj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[1024];
		glGetShaderInfoLog(shobj, 1024, NULL, infoLog);
		SysEng::Error("Error compiling shader \"%s\":\n%s", file, infoLog);
		glDeleteShader(shobj);
		return 0;
	}

	return shobj;
}
