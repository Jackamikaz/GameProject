#ifndef __GL_MODEL_HPP__
#define __GL_MODEL_HPP__

#include "Model.hpp"
#include "GlBuffer.hpp"
#include "syseng.hpp"

class GlModel {
public:
							GlModel();
							GlModel(const char* modelfile, const char* texturefile, bool castsShadow = true);
							~GlModel();

	bool					Load(const char* modelfile, const char* texturefile, bool castsShadow = true);

	inline bool				HasSkeleton() const		{SEASSERT(model); return model->skeleton != 0;}
	inline bool				CastsShadow() const		{return IBOadj != 0;}

	inline Model*			GetModel() const		{return model;}
	inline Texture*			GetTexture() const		{return texture;}
	inline GlBuffer*		GetVBO() const			{return VBO;}
	inline GlBuffer*		GetIBO() const			{return IBO;}
	inline GlBuffer*		GetIBOadj() const		{return IBOadj;}

private:
	Model*					model;
	Texture*				texture;

	GlBuffer*				VBO;
	GlBuffer*				IBO;
	GlBuffer*				IBOadj;
};

#endif//__GL_MODEL_HPP__
