#ifndef __SCENE_OBJECT_HPP__
#define __SCENE_OBJECT_HPP__

#include <Eigen/Dense>
#include "GlModel.hpp"

class SceneObject {
public:
								SceneObject(const GlModel& glmodel);
								~SceneObject();
	Eigen::Matrix4f				localMat;

	inline bool					IsAnimated() const {return pose != 0;}
	inline const Texture&		GetTexture() const {return texOverride ? *texOverride : *model.GetTexture();}

	const GlModel&				model;
	Model::Pose					pose;
	const Texture*				texOverride;
};

#endif//__SCENE_OBJECT_HPP__
