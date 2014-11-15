#include "SceneObject.hpp"

SceneObject::SceneObject(const GlModel& glmodel)
:	model(glmodel)
,	pose(0)
,	texOverride(0)
{
	localMat.setIdentity();

	if (model.HasSkeleton()) {
		pose = new Eigen::Matrix4f[model.GetModel()->skeleton->nbBones];
		for(unsigned int i=0; i<model.GetModel()->skeleton->nbBones; ++i) {
			pose[i].setIdentity();
		}
	}
}

SceneObject::~SceneObject() {
	if (pose) {
		delete [] pose;
	}
}
