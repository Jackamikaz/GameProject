#include "Texture.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <string.h>
#include "syseng.hpp"

Texture::Texture()
:	m_texID(0)
{}

Texture::Texture(const char* i_filename)
:	m_texID(0)
{
	Load(i_filename);
}

Texture::Texture(const GlFrameBuffer& frameBuffer, unsigned int width, unsigned int height, const FrameBufferComponent component)
:	m_texID(0)
{
	FromFrameBuffer(frameBuffer,width,height,component);
}

Texture::~Texture()
{
	if (m_texID != 0)
	{
		Destroy();
	}
}

void Texture::FromFrameBuffer(const GlFrameBuffer& frameBuffer, unsigned int width, unsigned int height, const FrameBufferComponent fbcomp) {
	SEASSERT(m_texID == 0);

	GLuint component, attachment;
	switch (fbcomp) {
	case fbColor:
		component = GL_RGBA;
		attachment = GL_COLOR_ATTACHMENT0;
		break;
	case fbDepth:
		component = GL_DEPTH_COMPONENT;
		attachment = GL_DEPTH_ATTACHMENT;
		break;
	}

    glGenTextures(1, &m_texID);
    glBindTexture(GL_TEXTURE_2D, m_texID);
    glTexImage2D(GL_TEXTURE_2D, 0, component, width, height, 0, component, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_texID, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//SDL_Surface * FlipSurface(SDL_Surface * surface);

bool Texture::Load(const char* i_filename, bool useMipMap)
{
	SEASSERT(m_texID == 0);
	if (m_texID != 0)
		return false;

    SDL_Surface* picture_surface = NULL;
    SDL_Surface* gl_surface = NULL;
    Uint32 rmask, gmask, bmask, amask;

    picture_surface = IMG_Load(i_filename);
    if (picture_surface == NULL) {
    	SysEng::Log(SysEng::error,"Error loading the file \"%s\" for texture loading.", i_filename);
        return false;
    }

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    SDL_PixelFormat format = *(picture_surface->format);
    format.BitsPerPixel = 32;
    format.BytesPerPixel = 4;
    format.Rmask = rmask;
    format.Gmask = gmask;
    format.Bmask = bmask;
    format.Amask = amask;

    gl_surface = SDL_ConvertSurface(picture_surface,&format,SDL_SWSURFACE);

//	SDL_Surface* gl_fliped_surface = NULL;
//	gl_fliped_surface = FlipSurface(gl_surface);
//	SDL_FreeSurface(gl_surface);
//	gl_surface = gl_fliped_surface;

    glGenTextures(1, &m_texID);

    glBindTexture(GL_TEXTURE_2D, m_texID);


	if (useMipMap)
	{

		gluBuild2DMipmaps(	GL_TEXTURE_2D, 4, gl_surface->w,
							gl_surface->h, GL_RGBA,GL_UNSIGNED_BYTE,
							gl_surface->pixels);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);

	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, gl_surface->w,
		gl_surface->h, 0, GL_RGBA,GL_UNSIGNED_BYTE,
		gl_surface->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	SDL_FreeSurface(gl_surface);
	SDL_FreeSurface(picture_surface);

	return true;
}

void Texture::Destroy()
{
	SEASSERT(m_texID =! 0);

	glDeleteTextures(1,&m_texID);

	m_texID = 0;
}

/*SDL_Surface * FlipSurface(SDL_Surface * surface)
{
    int current_line,pitch;
    SDL_Surface * fliped_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                   surface->w,surface->h,
                                   surface->format->BitsPerPixel,
                                   surface->format->Rmask,
                                   surface->format->Gmask,
                                   surface->format->Bmask,
                                   surface->format->Amask);



    SDL_LockSurface(surface);
    SDL_LockSurface(fliped_surface);

    pitch = surface->pitch;
    for (current_line = 0; current_line < surface->h; current_line ++)
    {
        memcpy(&((unsigned char* )fliped_surface->pixels)[current_line*pitch],
               &((unsigned char* )surface->pixels)[(surface->h - 1  -
                                                    current_line)*pitch],
               pitch);
    }

    SDL_UnlockSurface(fliped_surface);
    SDL_UnlockSurface(surface);
    return fliped_surface;
}*/
