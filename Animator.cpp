#include "Animator.hpp"

Animator::Animator(const SceneObject& obj)
:	objTarget(obj)
,	currentTime(0.0f)
,	currentAnim(0)
{
}

void Animator::Update(const float deltaT) {
	if (currentAnim) {

		currentTime += deltaT*24.0f;
		if (currentTime > currentAnim->duration) {
			currentTime = fmod(currentTime, currentAnim->duration);
		}

		objTarget.model.GetModel()->ComputeAnimPose(*currentAnim,currentTime+currentAnim->timeBegin,objTarget.pose);
	}
}

void Animator::SetAnimation(const Model::Animation& anim) {
	currentAnim = &anim;
	if (currentAnim) {
		currentTime = 0.0f;
	}
}
