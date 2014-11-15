#ifndef __GL_BUFFER_HPP__
#define __GL_BUFFER_HPP__

#include "glheader.h"

class Model;

enum ModelBufferType {
	vertices, verticesWithBones, indices, adjacencyIndices,
};

class GlBuffer {
public:
						GlBuffer();
						GlBuffer(const Model& model, ModelBufferType mbt);
						~GlBuffer();

	bool				Load(const Model& model, ModelBufferType mbt);
	void				Destroy();

	inline bool			operator!(void) const {return (bufferID == 0);}
	inline				operator GLuint(void) const {return bufferID;}

private:
	GLuint bufferID;
};


#endif//__GL_BUFFER_HPP__
