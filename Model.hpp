#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include "syseng.hpp"
#include <Eigen/Dense>
#include <vector>

struct LocRotScale {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	Eigen::Vector3f loc;
	Eigen::Quaternionf rot;
	Eigen::Vector3f scl;

	Eigen::Matrix4f toMatrix() const;
	LocRotScale lerp(float t, const LocRotScale& to) const;
};

class Model {
public:

	Model();
	~Model();

	std::string name;

	struct Mesh {
		std::string name;
		unsigned int nbVertices;
		struct Vertex {
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
			Eigen::Vector3f pos;
			Eigen::Vector3f nor;
			Eigen::Vector2f uv;
		};
		struct VertexWithBones {
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
			Eigen::Vector3f pos;
			Eigen::Vector3f nor;
			Eigen::Vector2f uv;
			Eigen::Vector4i bids;
			Eigen::Vector4f bws;
		};
		Vertex* vertices;

		unsigned int nbTriangles;
		Eigen::Vector3i* triangles;

		struct Skin {
			struct VertexInfluence {
				struct Influence {
					int boneIndex;
					float weight;
				};

				union {
					Influence influence;
					unsigned int nbInfluences;
				};

				//hacky but handy
				Influence& operator [] (unsigned int n) {
					SEASSERT(n >= 0 && n < nbInfluences);
					return (this)[n+1].influence;
				}

				VertexInfluence* NextInfluence() {
					return (this + nbInfluences + 1);
				}
			};

			VertexInfluence* influences;
		};

		Skin* skin;
	};

	struct Skeleton {
		struct Bone{
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
			LocRotScale transform;
			Eigen::Matrix4f localMatrix;
			Eigen::Matrix4f globalInverseMatrix;
			Bone* parent;
			unsigned int nbChildren;
			Bone** children;
			std::string name;
			int index;
			void ConstructChildren(Skeleton& skeleton);
			void ConstructMatrices();
		};
		unsigned int nbBones;
		Bone* bones;
		void ConstructChildrenBones();
		void ConstructMatrices();
	};

	typedef Eigen::Matrix4f* Pose;

	struct Animation {
		std::string name;

		struct Channel {
			struct Keyframe {
				EIGEN_MAKE_ALIGNED_OPERATOR_NEW
				LocRotScale transform;
				float timestamp;
			};
			unsigned int nbKeyframes;
			Keyframe* keyframes;
		};
		unsigned int nbChannels;
		Channel* channels;
		float timeBegin;
		float duration;

		void ComputeTransformPose(float t, Pose pose) const;
	};

	Mesh* mesh;
	Skeleton* skeleton;
	std::vector<Animation> animations;

	void ComputeAnimPose(const Animation& anim, float t, Pose pose) const;
	void ComputeAnimPoseDebug(const Animation& anim, float t, Pose pose) const;
	bool Load(const char* filename);

	Mesh::VertexWithBones* CreateAndFillVerticesWithBones() const;
	void SkinToVectorArray(const Pose finalPose, Eigen::Vector3f* skinout) const;

	GLuint* CreateAdjacencyIndexList(GLuint invalidIndexValue = (unsigned int)-1) const;

private:
	void ComputeAnimPoseInternal(const Skeleton::Bone& bone, Pose pose) const;
};

#endif//__MODEL_HPP__
