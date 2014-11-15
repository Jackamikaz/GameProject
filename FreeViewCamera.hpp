#ifndef __FREE_VIEW_CAMERA_HPP__
#define __FREE_VIEW_CAMERA_HPP__

#include <Eigen/Dense>

class FreeViewCamera {
public:
									FreeViewCamera();

	void							Walk(const float dist);
	void							SideStep(const float dist);
	void							Hover(const float dist);

	void							TurnH(const float angle);
	void							TurnV(const float angle);
	void							TurnSelf(const float angle);
	void							TurnRestr(const float angle, const Eigen::Vector3f& up);

	void							LookAt(const Eigen::Vector3f& target, const Eigen::Vector3f& up);

	Eigen::Matrix4f					ToMatrix() const;

	inline const Eigen::Vector3f&	GetForward() const {return forward;}
	inline const Eigen::Vector3f&	GetSide() const {return side;}
	Eigen::Vector3f					position;
private:
	Eigen::Vector3f					forward;
	Eigen::Vector3f					side;
};

#endif//__FREE_VIEW_CAMERA_HPP__
