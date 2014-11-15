#ifndef __PIPELINE_HPP__
#define __PIPELINE_HPP__

#include "Scene.hpp"
#include "ShaderProgram.hpp"
#include "GlUniformHolder.hpp"

#include "GlFrameBuffer.hpp"
#include "Texture.hpp"

class Pipeline {
public:

					Pipeline();
					~Pipeline();
	void			Draw(const Scene& scene);

	GlFrameBuffer	frameBuffer;
	Texture			fbTex;

private:
	enum Pass {
		depth,
		shadow,
		color,
		nbPasses,
	};

	enum TechniqueType {
		anim,
		stat,
		nbTechTypes,
	};

	struct Technique {
		struct Tech {
			ShaderProgram	shader;
			GlUniformHolder	uniforms;
		};
		Tech type[nbTechTypes];
	};

	Technique techniques[nbPasses];

	void			DoPass(const Scene& scene, unsigned int pass, bool anim) const;
};

#endif//__PIPELINE_HPP__
