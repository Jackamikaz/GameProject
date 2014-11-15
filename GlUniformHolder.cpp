#include "GlUniformHolder.hpp"
#include <cstdio>
#include "syseng.hpp"

GlUniformHolder::GlUniformHolder()
:	shader(0)
{
}

GlUniformHolder::~GlUniformHolder() {
	for(unsigned int i=0; i<locations.size(); ++i) {
		if (locations[i].locs) {
			delete [] locations[i].locs;
		}
	}
	locations.clear();
}

GlUniformHolder& GlUniformHolder::Start(const ShaderProgram& sh) {
	shader = &sh;
	glUseProgram(*shader);
	return *this;
}

GlUniformHolder& GlUniformHolder::AddUniform(const char* uniform) {

	Location loc;
	loc.loc = glGetUniformLocation(*shader,uniform);
	locations.push_back(loc);

	return *this;
}

GlUniformHolder& GlUniformHolder::AddUniform(const char* uniform, unsigned int arraySize) {

	Location loc;
	loc.nbLocs = arraySize;
	loc.locs = new GLint[loc.nbLocs];

	char buffer[128];

	for(unsigned int i=0; i<loc.nbLocs; ++i) {
		snprintf(buffer,128,"%s[%d]",uniform,i);
		loc.locs[i] = glGetUniformLocation(*shader,buffer);
	}

	locations.push_back(loc);

	return *this;
}

void GlUniformHolder::End() {
	glUseProgram(0);
}

#define ASSERT1 SEASSERT(locations[uniform].loc != -1)
#define ASSERT2 SEASSERT(locations[uniform].locs \
	&& index >= 0 && index < locations[uniform].nbLocs \
	&& locations[uniform].locs[index] != -1)

void GlUniformHolder::Set(unsigned int uniform, GLuint value) const
		{ASSERT1; glUniform1ui(locations[uniform].loc,			value);}
void GlUniformHolder::Set(unsigned int uniform, unsigned int index, GLuint value) const
		{ASSERT2; glUniform1ui(locations[uniform].locs[index],	value);}

void GlUniformHolder::Set(unsigned int uniform, GLfloat value) const
		{ASSERT1; glUniform1f(locations[uniform].loc,			value);}
void GlUniformHolder::Set(unsigned int uniform, unsigned int index, GLfloat value) const
		{ASSERT2; glUniform1f(locations[uniform].locs[index],	value);}

void GlUniformHolder::Set(unsigned int uniform, const Eigen::Vector3f& value) const
		{ASSERT1; glUniform3f(locations[uniform].loc,			value(0), value(1), value(2));}
void GlUniformHolder::Set(unsigned int uniform, unsigned int index, const Eigen::Vector3f& value) const
		{ASSERT2; glUniform3f(locations[uniform].locs[index],	value(0), value(1), value(2));}

void GlUniformHolder::Set(unsigned int uniform, const Eigen::Matrix4f& mat) const
		{ASSERT1; glUniformMatrix4fv(locations[uniform].loc,			1,false,mat.data());}
void GlUniformHolder::Set(unsigned int uniform, unsigned int index, const Eigen::Matrix4f& mat) const
		{ASSERT2; glUniformMatrix4fv(locations[uniform].locs[index],	1,false,mat.data());}
