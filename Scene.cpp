#include "Scene.hpp"

Scene::Scene()
:	ambientFactor(0.5f)
{
	perspective.setIdentity();
	view.setIdentity();
	lightDirection << 0.0f, 0.0f, -1.0f, 0.0f;
}

void Scene::Insert(const SceneObject& obj) {
	if (obj.IsAnimated()) {
		objectsWithAnim.push_back(&obj);
	}
	else {
		objectsNoAnim.push_back(&obj);
	}

}

const Scene::ObjectList& Scene::GetObjectsList(bool withAnim) const {
	return withAnim ? objectsWithAnim : objectsNoAnim;
}
