#ifndef __ANIMATOR_HPP__
#define __ANIMATOR_HPP__

#include "Model.hpp"
#include "SceneObject.hpp"

class Animator {
public:

								Animator(const SceneObject& obj);

	void						Update(const float deltaT);

	//TODO: queue, blend, etc. With looping option or stop, reverse, etc.
	void						SetAnimation(const Model::Animation& animation);
private:
	const SceneObject&			objTarget;

	float						currentTime;
	const Model::Animation*		currentAnim;
};

#endif//__ANIMATOR_HPP__
