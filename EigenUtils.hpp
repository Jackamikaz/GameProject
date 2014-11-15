// My thanks to Sylvain Pointeau http://spointeau.blogspot.fr/2013/12/hello-i-am-looking-at-opengl-3.html

#include "Eigen/Core"
#include <cmath>

template<class T>
Eigen::Matrix<T,4,4> perspective
(
    double fovy,
    double aspect,
    double zNear,
    double zFar
)
{
    typedef Eigen::Matrix<T,4,4> Matrix4;

    assert(aspect > 0);
    assert(zFar > zNear);

    //double radf = Math::degToRad(fovy);
    double radf = fovy * M_PI / 180.0;

    double tanHalfFovy = tan(radf / 2.0);
    Matrix4 res = Matrix4::Zero();
    res(0,0) = 1.0 / (aspect * tanHalfFovy);
    res(1,1) = 1.0 / (tanHalfFovy);
    res(2,2) = - (zFar + zNear) / (zFar - zNear);
    res(3,2) = - 1.0;
    res(2,3) = - (2.0 * zFar * zNear) / (zFar - zNear);
    return res;
}

template<class T>
Eigen::Matrix<T,4,4> lookAt
(
    Eigen::Matrix<T,3,1> const & eye,
    Eigen::Matrix<T,3,1> const & center,
    Eigen::Matrix<T,3,1> const & up
)
{
    typedef Eigen::Matrix<T,4,4> Matrix4;
    typedef Eigen::Matrix<T,3,1> Vector3;

    Vector3 f = (center - eye).normalized();
    Vector3 u = up.normalized();
    Vector3 s = f.cross(u).normalized();
    u = s.cross(f);

    Matrix4 res;
    res <<  s.x(),s.y(),s.z(),-s.dot(eye),
            u.x(),u.y(),u.z(),-u.dot(eye),
            -f.x(),-f.y(),-f.z(),f.dot(eye),
            0,0,0,1;

    return res;
}

template<class T>
Eigen::Matrix<T,4,4> lookAt(T eyex,T eyey,T eyez,T centerx,T centery,T centerz,T upx,T upy,T upz) {
	Eigen::Matrix<T,3,1> eye,center,up;
	eye << eyex, eyey, eyez;
	center << centerx, centery,centerz;
	up << upx, upy, upz;

	return lookAt(eye,center,up);
}

template<class T>
Eigen::Matrix<T,4,4> translate(T x, T y, T z) {
	Eigen::Matrix<T,4,4> mat;
	mat <<
			1, 0, 0, x,
			0, 1, 0, y,
			0, 0, 1, z,
			0, 0, 0, 1;
	return mat;
}

template<class T>
Eigen::Matrix<T,4,4> scale(T x, T y, T z) {
	Eigen::Matrix<T,4,4> mat;
	mat <<
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, 1;
	return mat;
}

template<class T>
Eigen::Matrix<T,4,4> rotate(T angle, T x, T y, T z) {

	T c = cos(angle);
	T s = sin(angle);

	T invSize = 1.0 / sqrt(x*x+y*y+z*z);
	x *= invSize;
	y *= invSize;
	z *= invSize;

	Eigen::Matrix<T,4,4> mat;

	mat <<	x*x*(1-c)+c,	x*y*(1-c)-z*s,	x*z*(1-c)+y*s,	0,
			y*x*(1-c)+z*s,	y*y*(1-c)+c,	y*z*(1-c)-x*s,	0,
			x*z*(1-c)-y*s,	y*z*(1-c)+x*s,	z*z*(1-c)+c,	0,
			0,				0,				0,				1;

	return mat;
}

template<class T>
inline Eigen::Matrix<T,4,4> rotate(T angle, Eigen::Matrix<T,3,1> const & axis) {
	return rotate<T>(angle, axis.x(), axis.y(), axis.z());
}
