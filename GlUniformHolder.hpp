#ifndef __GL_UNIFORM_HOLDER_HPP__
#define __GL_UNIFORM_HOLDER_HPP__

#include "ShaderProgram.hpp"
#include "glheader.h"
#include <vector>
#include <Eigen/Dense>

class GlUniformHolder {
public:
						GlUniformHolder();
						~GlUniformHolder();

	GlUniformHolder&	Start(const ShaderProgram& sh);
	GlUniformHolder&	AddUniform(const char*);
	GlUniformHolder&	AddUniform(const char*, unsigned int arraySize);
	void				End();

	inline bool			UniformExists(unsigned int uniform) const {
		return locations[uniform].locs ? (locations[uniform].locs[0] != -1) : locations[uniform].loc != -1;
	}

	void				Set(unsigned int uniform, GLuint value) const;
	void				Set(unsigned int uniform, unsigned int index, GLuint value) const;

	void				Set(unsigned int uniform, GLfloat value) const;
	void				Set(unsigned int uniform, unsigned int index, GLfloat value) const;

	void				Set(unsigned int uniform, const Eigen::Vector3f& mat) const;
	void				Set(unsigned int uniform, unsigned int index, const Eigen::Vector3f& mat) const;

	void				Set(unsigned int uniform, const Eigen::Matrix4f& mat) const;
	void				Set(unsigned int uniform, unsigned int index, const Eigen::Matrix4f& mat) const;

private:
	const ShaderProgram* shader;

	struct Location {
		Location() : loc(0), locs(0) {}
		union {
			GLint loc;
			unsigned int nbLocs;
		};
		GLint* locs;
	};
	std::vector<Location> locations;
};

#define DEFAULT_UNIFORMS(m)	\
	m(uViewMatrix)			\
	m(uProjMatrix)			\
	m(uFinalMatrix)			\
	m(uSampler0)			\
	m(uDirectionalLight)	\
	m(uAmbientFactor)		\
	m(uBones,32)

#define GET_MACRO(_1,_2,NAME,...) NAME
#define INITMACRO(...) GET_MACRO(__VA_ARGS__, INITMACRO2, INITMACRO1)(__VA_ARGS__)
#define ENUMIFYMACRO(...) GET_MACRO(__VA_ARGS__, ENUMIFYMACRO2, ENUMIFYMACRO1)(__VA_ARGS__)

#define INITMACRO1(val) .AddUniform(#val)
#define INITMACRO2(val,id) .AddUniform(#val,id)
#define ENUMIFYMACRO1(val) val,
#define ENUMIFYMACRO2(val,id) val,

#define DEFINE_UNIFORM_ENUM(uniforms) uniforms(ENUMIFYMACRO)
#define INIT_UNIFORM_HANDLER(shader,uniforms) Start(shader) uniforms(INITMACRO).End()

enum DefaultUniforms { DEFINE_UNIFORM_ENUM(DEFAULT_UNIFORMS) };

#endif//__GL_UNIFORM_HOLDER_HPP__
