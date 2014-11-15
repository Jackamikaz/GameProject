#ifndef __SCENE_HPP__
#define __SCENE_HPP__

#include "Scene.hpp"
#include "SceneObject.hpp"
#include <list>
#include <Eigen/Dense>

class Scene {
public:
	typedef std::list<const SceneObject*> ObjectList;

								Scene();

	void						Insert(const SceneObject& obj);
	const ObjectList&			GetObjectsList(bool withAnim) const;

	Eigen::Matrix4f				perspective;
	Eigen::Matrix4f				view;
	Eigen::Matrix4f				view2;

	Eigen::Vector4f				lightDirection;
	float						ambientFactor;

private:
	ObjectList					objectsWithAnim;
	ObjectList					objectsNoAnim;
};

#endif
