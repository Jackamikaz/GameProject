#include "GlFrameBuffer.hpp"
#include "syseng.hpp"

GlFrameBuffer::GlFrameBuffer()
:	frameBufferID(0)
{
	Generate();
}

GlFrameBuffer::~GlFrameBuffer() {
	if (frameBufferID != 0)
	{
		Destroy();
	}
}

void GlFrameBuffer::Generate() {
	SEASSERT(frameBufferID == 0);

	glGenFramebuffers(1,&frameBufferID);
}

void GlFrameBuffer::Destroy() {
	SEASSERT(frameBufferID =! 0);

	glDeleteFramebuffers(1,&frameBufferID);

	frameBufferID = 0;
}
