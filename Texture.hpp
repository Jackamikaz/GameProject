#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "glheader.h"
#include "GlFrameBuffer.hpp"

class Texture
{
public:
						Texture();
						Texture(const char* i_filename);
						Texture(const GlFrameBuffer& frameBuffer, unsigned int width, unsigned int height, const FrameBufferComponent component);
						~Texture();

	bool				Load(const char* i_filename, bool useMipMap = true);
	void				FromFrameBuffer(const GlFrameBuffer& frameBuffer, unsigned int width, unsigned int height, const FrameBufferComponent component);
	void				Destroy();

	inline bool			operator!(void) const {return (m_texID == 0);}
	inline				operator GLuint(void) const {return m_texID;}

private:
	GLuint				m_texID;
};

#endif
