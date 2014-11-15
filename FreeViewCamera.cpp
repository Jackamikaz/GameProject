#include "FreeViewCamera.hpp"
#include "EigenUtils.hpp"

FreeViewCamera::FreeViewCamera() {
	position << 0.0f, 0.0f, 0.0f;
	forward << 1.0f, 0.0f, 0.0f;
	side << 0.0f, -1.0f, 0.0f;
}

void FreeViewCamera::Walk(const float dist) {
	position += forward * dist;
}

void FreeViewCamera::SideStep(const float dist) {
	position += side * dist;
}

void FreeViewCamera::Hover(const float dist) {
	position += (side.cross(forward)).normalized() * dist;
}

void FreeViewCamera::TurnH(const float angle) {
	TurnRestr(angle,side.cross(forward));
}

void FreeViewCamera::TurnV(const float angle) {
	forward = rotate<float>(angle,side).topLeftCorner<3,3>() * forward;
}

void FreeViewCamera::TurnSelf(const float angle) {
	side = rotate<float>(-angle,forward).topLeftCorner<3,3>() * side;
}

void FreeViewCamera::TurnRestr(const float angle, const Eigen::Vector3f& up) {
	Eigen::Matrix3f mat = rotate<float>(angle,up).topLeftCorner<3,3>();

	forward = mat * forward;
	side = mat * side;
}

void FreeViewCamera::LookAt(const Eigen::Vector3f& target, const Eigen::Vector3f& up) {
	forward = target - position;
	forward.normalize();

	side = forward.cross(up);
	side.normalize();
}

Eigen::Matrix4f FreeViewCamera::ToMatrix() const {
	Eigen::Vector3f up = side.cross(forward);
	up.normalize();

	Eigen::Matrix4f mat;

	mat <<	side.x(),		side.y(),		side.z(),		-side.dot(position),
			up.x(),			up.y(),			up.z(),			-up.dot(position),
			-forward.x(),	-forward.y(),	-forward.z(),	forward.dot(position),
			0,				0,				0,				1;

	return mat;
}
