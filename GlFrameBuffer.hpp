#ifndef __GL_FRAME_BUFFER_HPP__
#define __GL_FRAME_BUFFER_HPP__

#include "glheader.h"

enum FrameBufferComponent {
	fbColor, fbDepth //TODO: stentil and depth_stencil
};

class GlFrameBuffer {
public:
						GlFrameBuffer();
						~GlFrameBuffer();

	void				Generate();
	void				Destroy();

	inline bool			operator!(void) const {return (frameBufferID == 0);}
	inline				operator GLuint(void) const {return frameBufferID;}
private:
	GLuint				frameBufferID;
};

#endif//__GL_FRAME_BUFFER_HPP__
