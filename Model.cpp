#include "Model.hpp"

#include <fstream>
#include <limits>

#include <cstdio>

using namespace std;
using namespace Eigen;

Model::Model()
:	mesh(0)
,	skeleton(0) {

}

Model::~Model() {
	if (mesh) {
		if (mesh->vertices) {
			delete [] mesh->vertices;
		}
		if (mesh->triangles) {
			delete [] mesh->triangles;
		}
		if (mesh->skin) {
			if (mesh->skin->influences) {
				delete [] mesh->skin->influences;
			}
			delete mesh->skin;
		}

		delete mesh;
		mesh = 0;
	}

	if (skeleton) {
		if (skeleton->bones) {
			delete [] skeleton->bones;
		}
		delete skeleton;
		skeleton = 0;
	}

	if (!animations.empty()) {
		for(unsigned int i=0; i<animations.size(); ++i) {
			for (unsigned int j=0; j<animations[i].nbChannels; ++j) {
				delete [] animations[i].channels[j].keyframes;
			}
			delete [] animations[i].channels;
		}
		animations.clear();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//				LOADING AND READING FILE METHODS AND FUNCTIONS							//
//////////////////////////////////////////////////////////////////////////////////////////

string ReadMesh(ifstream& file, Model::Mesh* mesh);
void ReadSkeleton(ifstream& file, Model::Skeleton* skeleton);
void ReadAnimations(ifstream& file, vector<Model::Animation>& anim);

bool Model::Load(const char* filename) {

	ifstream file(filename);
	if (file.is_open()) {

		// read the first line, useless at the moment
		string model;
		int nbModels;

		file >> model >> nbModels;

		// read the first element (mesh or skeleton or animations)
		string element;
		file >> element;
		bool cont = true;
		while (cont && !file.eof()) {
			transform(element.begin(), element.end(), element.begin(), ::tolower);
			// mesh case
			if (element.compare("mesh") == 0) {
				SEASSERT(mesh == 0);
				mesh = new Mesh;
				mesh->vertices = 0;
				mesh->triangles = 0;
				mesh->skin = 0;
				element = ReadMesh(file, mesh);
			}
			// skeleton case
			else if (element.compare("skeleton") == 0) {
				SEASSERT(skeleton == 0);
				skeleton = new Skeleton;
				ReadSkeleton(file, skeleton);
				file >> element;
			}
			// animations case
			else if (element.compare("animations") == 0) {
				SEASSERT(animations.empty());
				ReadAnimations(file, animations);
				file >> element;
			}
			else {
				cont = false;
			}
		}

		file.close();
		return true;
	}

	SysEng::Log(SysEng::log, "Error opening the file \"%s\" for loading a model.",filename);
	return false;
}

string ReadMesh(ifstream& file, Model::Mesh* mesh) {
	string element;
	file >> mesh->name >> element;
	bool cont = true;
	while (cont && !file.eof()) {
		transform(element.begin(), element.end(), element.begin(), ::tolower);
		// vertices case
		if (element.compare("vertices") == 0) {
			file >> mesh->nbVertices;
			mesh->vertices = new Model::Mesh::Vertex[mesh->nbVertices];
			for (unsigned int i=0; i<mesh->nbVertices; ++i) {
				file >> mesh->vertices[i].pos(0) >> mesh->vertices[i].pos(1) >> mesh->vertices[i].pos(2);
				file >> mesh->vertices[i].nor(0) >> mesh->vertices[i].nor(1) >> mesh->vertices[i].nor(2);
				file >> mesh->vertices[i].uv(0) >> mesh->vertices[i].uv(1);
				mesh->vertices[i].nor.normalize();
			}
			file >> element;
		}
		//triangles case
		else if (element.compare("triangles") == 0) {
			file >> mesh->nbTriangles;
			mesh->triangles = new Vector3i[mesh->nbTriangles];
			for (unsigned int i=0; i<mesh->nbTriangles; ++i) {
				file >> mesh->triangles[i](0) >> mesh->triangles[i](1) >> mesh->triangles[i](2);
			}
			file >> element;
		}
		//skin case
		else if (element.compare("skin") == 0) {
			int nbVertices;
			file >> nbVertices;
			vector<Model::Mesh::Skin::VertexInfluence> influences;
			Model::Mesh::Skin::VertexInfluence influence;
			string word;
			for (int i=0; i<nbVertices; ++i) {
				int nbInfluences;
				file >> word >> nbInfluences;
				influence.influence.weight = numeric_limits<float>::quiet_NaN();
				influence.nbInfluences = nbInfluences;
				influences.push_back(influence);

				float totalWeight = 0.0f;
				unsigned int firstInfluence = influences.size();
				for (int j=0; j<nbInfluences; ++j) {
					file >> influence.influence.boneIndex >> influence.influence.weight;
					influences.push_back(influence);
					totalWeight += influence.influence.weight;
				}

				// make sure the influences all sum to 1.0f
				if (totalWeight != 1.0f) {
					for (int j=0; j<nbInfluences; ++j) {
						influences[firstInfluence+j].influence.weight /= totalWeight;
					}
				}
			}
			//put the skin as a vector into the array of the class
			mesh->skin = new Model::Mesh::Skin;
			mesh->skin->influences = new Model::Mesh::Skin::VertexInfluence[influences.size()];
			memcpy(mesh->skin->influences, influences.data(), sizeof(Model::Mesh::Skin::VertexInfluence) * influences.size());
			file >> element;
		}
		else {
			cont = false;
		}
	}
	return element;
}

void ReadSkeleton(ifstream& file, Model::Skeleton* skeleton) {
	file >> skeleton->nbBones;
	skeleton->bones = new Model::Skeleton::Bone[skeleton->nbBones];
	for (unsigned int i=0; i<skeleton->nbBones; ++i) {
		skeleton->bones[i].index = i;
		file >> skeleton->bones[i].name;
		int parentIndex;
		file >> parentIndex;
		if (parentIndex >= 0) {
			skeleton->bones[i].parent = &skeleton->bones[parentIndex];
		}
		else {
			skeleton->bones[i].parent = 0;
		}
		file >> skeleton->bones[i].transform.loc(0);
		file >> skeleton->bones[i].transform.loc(1);
		file >> skeleton->bones[i].transform.loc(2);
		file >> skeleton->bones[i].transform.rot.w();
		file >> skeleton->bones[i].transform.rot.x();
		file >> skeleton->bones[i].transform.rot.y();
		file >> skeleton->bones[i].transform.rot.z();
		file >> skeleton->bones[i].transform.scl(0);
		file >> skeleton->bones[i].transform.scl(1);
		file >> skeleton->bones[i].transform.scl(2);
	}
	skeleton->ConstructChildrenBones();
	skeleton->ConstructMatrices();
}

void ReadAnimations(ifstream& file, vector<Model::Animation>& animList) {
	int nbAnimations;
	file >> nbAnimations;

	Model::Animation anim;
	string word;

	for (int i=0; i<nbAnimations; ++i) {
		float minTimestamp =  numeric_limits<float>::infinity();
		float maxTimestamp = -numeric_limits<float>::infinity();

		file >> word >> anim.name >> anim.nbChannels;
		anim.channels = new Model::Animation::Channel[anim.nbChannels];
		for (unsigned int j=0; j<anim.nbChannels; ++j) {
			file >> word >> word >> anim.channels[j].nbKeyframes;
			anim.channels[j].keyframes = new Model::Animation::Channel::Keyframe[anim.channels[j].nbKeyframes];
			for (unsigned int k=0; k<anim.channels[j].nbKeyframes; ++k) {
				file >> anim.channels[j].keyframes[k].timestamp;
				file >> anim.channels[j].keyframes[k].transform.loc(0);
				file >> anim.channels[j].keyframes[k].transform.loc(1);
				file >> anim.channels[j].keyframes[k].transform.loc(2);
				file >> anim.channels[j].keyframes[k].transform.rot.w();
				file >> anim.channels[j].keyframes[k].transform.rot.x();
				file >> anim.channels[j].keyframes[k].transform.rot.y();
				file >> anim.channels[j].keyframes[k].transform.rot.z();
				file >> anim.channels[j].keyframes[k].transform.scl(0);
				file >> anim.channels[j].keyframes[k].transform.scl(1);
				file >> anim.channels[j].keyframes[k].transform.scl(2);

				float timestamp = anim.channels[j].keyframes[k].timestamp;

				if (timestamp < minTimestamp) {
					minTimestamp = timestamp;
				}
				if (timestamp > maxTimestamp) {
					maxTimestamp = timestamp;
				}
			}
		}
		anim.duration = maxTimestamp - minTimestamp;
		anim.timeBegin = minTimestamp;
		animList.push_back(anim);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//								ANIMATION SETUP METHODS									//
//////////////////////////////////////////////////////////////////////////////////////////

void Model::Animation::ComputeTransformPose(float t, Pose pose) const {
	for (unsigned int i=0; i<nbChannels; ++i) {
		unsigned int keyFrame = 0;
		while (t > channels[i].keyframes[keyFrame].timestamp && keyFrame < channels[i].nbKeyframes) {
			++keyFrame;
		}
		//loop cases, clamp for now
		if (keyFrame == 0) {
			pose[i] = channels[i].keyframes[keyFrame].transform.toMatrix();
		}
		else if (keyFrame == channels[i].nbKeyframes) {
			pose[i] = channels[i].keyframes[keyFrame-1].transform.toMatrix();
		}
		//lerp between keyFrame-1 and keyFrame
		else {
			int prevKeyFrame = keyFrame-1;
			float prevT = channels[i].keyframes[prevKeyFrame].timestamp;
			float nextT = channels[i].keyframes[keyFrame].timestamp;
			float lerpT = (nextT - t) / (nextT - prevT);
			pose[i] = channels[i].keyframes[prevKeyFrame].transform.lerp(1.0f-lerpT,channels[i].keyframes[keyFrame].transform).toMatrix();
		}
	}
}

void Model::ComputeAnimPose(const Animation& anim, float t, Pose pose) const {
	anim.ComputeTransformPose(t,pose);
	ComputeAnimPoseInternal(skeleton->bones[0], pose);
	// pre-process for skinning
	for(unsigned int i=0; i<skeleton->nbBones; ++i) {
		pose[i] = pose[i] * skeleton->bones[i].globalInverseMatrix;
	}
}

//useful for debug display, returns a pose of each world matrix of the joint
void Model::ComputeAnimPoseDebug(const Animation& anim, float t, Pose pose) const {
	anim.ComputeTransformPose(t,pose);
	ComputeAnimPoseInternal(skeleton->bones[0], pose);
}

void Model::ComputeAnimPoseInternal(const Skeleton::Bone& bone, Pose pose) const {
	if (bone.parent != 0) {
		pose[bone.index] = pose[bone.parent->index] * bone.localMatrix * pose[bone.index];
	}
	else {
		pose[bone.index] = bone.localMatrix * pose[bone.index];
	}

	for(unsigned int i=0; i<bone.nbChildren; ++i) {
		ComputeAnimPoseInternal(*bone.children[i], pose);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//				SKELETON INITIALIZATION WITH RECURSIVE METHODS							//
//////////////////////////////////////////////////////////////////////////////////////////

void Model::Skeleton::ConstructChildrenBones() {
	bones[0].ConstructChildren(*this);
}

void Model::Skeleton::ConstructMatrices() {
	bones[0].ConstructMatrices();
}

void Model::Skeleton::Bone::ConstructChildren(Model::Skeleton& skeleton) {
	vector<Bone*> tempChildren;
	for(unsigned int i=0; i<skeleton.nbBones; ++i) {
		if (skeleton.bones[i].parent == this) {
			tempChildren.push_back(&skeleton.bones[i]);
		}
	}
	nbChildren = tempChildren.size();
	children = new Bone*[nbChildren];
	memcpy(children, tempChildren.data(), sizeof(Bone*)*nbChildren);
	for(unsigned int i=0; i<nbChildren; ++i) {
		children[i]->ConstructChildren(skeleton);
	}
}

void Model::Skeleton::Bone::ConstructMatrices() {
	localMatrix = transform.toMatrix();
	globalInverseMatrix = localMatrix.inverse();
	if (parent) {
		localMatrix = parent->globalInverseMatrix * localMatrix;
	}

	for (unsigned int i=0; i<nbChildren; ++i) {
		children[i]->ConstructMatrices();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//							HELPER CLASSES AND METHODS									//
//////////////////////////////////////////////////////////////////////////////////////////

Matrix4f LocRotScale::toMatrix() const {
	Affine3f a;
	a.setIdentity();
	a.translation() = loc;
	a.rotate(rot);
	a.scale(scl);
	return a.matrix();
}

LocRotScale LocRotScale::lerp(float t, const LocRotScale& to) const {
	LocRotScale res;

	res.loc = loc + (to.loc - loc) * t;
	res.rot = rot.slerp(t,to.rot);
	res.scl = scl + (to.scl - scl) * t;

	return res;
}

Model::Mesh::VertexWithBones* Model::CreateAndFillVerticesWithBones() const {

	SEASSERT(mesh && mesh->skin);
	Mesh::VertexWithBones* vertices = new Mesh::VertexWithBones[mesh->nbVertices];

	Mesh::Skin::VertexInfluence* curInfl = mesh->skin->influences;
	for (unsigned int i=0; i<mesh->nbVertices; ++i) {

		vertices[i].pos = mesh->vertices[i].pos;
		vertices[i].nor = mesh->vertices[i].nor;
		vertices[i].uv = mesh->vertices[i].uv;

		float weightSum = 0.0f;

		for (unsigned int j=0; j<4; ++j) {
			if (j<curInfl->nbInfluences) {
				Model::Mesh::Skin::VertexInfluence::Influence& influence = (*curInfl)[j];
				vertices[i].bids[j] = influence.boneIndex;
				vertices[i].bws[j] = influence.weight;
				weightSum += influence.weight;
			}
			else {
				vertices[i].bids[j] = 0;
				vertices[i].bws[j] = 0.0f;
			}
		}

		// adjust weights if there is too many bones
		if(curInfl->nbInfluences > 4) {
			for (int j=0; j<4; ++j) {
				vertices[i].bws[j] /= weightSum;
			}
		}

		curInfl = curInfl->NextInfluence();
	}

	return vertices;
}

void Model::SkinToVectorArray(const Model::Pose pose, Vector3f* skinout) const {
	SEASSERT(mesh && mesh->skin);
	Model::Mesh::Skin::VertexInfluence* curInfl = mesh->skin->influences;
	for (unsigned int i=0; i<mesh->nbVertices; ++i) {

		Vector4f posSum(0.0f,0.0f,0.0f,0.0f);
		Vector4f norSum(0.0f,0.0f,0.0f,0.0f);

		Vector4f tmpPos;
		tmpPos << mesh->vertices[i].pos, 1.0f;

		Vector4f tmpNor;
		tmpNor << mesh->vertices[i].nor, 0.0f;

		for (unsigned int j=0; j<curInfl->nbInfluences; ++j) {
			Model::Mesh::Skin::VertexInfluence::Influence& influence = (*curInfl)[j];

			const Matrix4f& mat = pose[influence.boneIndex];

			posSum += (mat * tmpPos) * influence.weight;
			norSum += (mat * tmpNor) * influence.weight;
		}

		skinout[i*2](0) = posSum(0);
		skinout[i*2](1) = posSum(1);
		skinout[i*2](2) = posSum(2);

		skinout[i*2+1](0) = norSum(0);
		skinout[i*2+1](1) = norSum(1);
		skinout[i*2+1](2) = norSum(2);

		curInfl = curInfl->NextInfluence();
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
//			What follows is largely inspired from this tutorial by Etay Meiri :			//
//				http://ogldev.atspace.co.uk/www/tutorial39/tutorial39.html				//
//////////////////////////////////////////////////////////////////////////////////////////

struct Edge
{
    Edge(unsigned int _a, unsigned int _b)
    {
        SEASSERT(_a != _b);

        if (_a < _b)
        {
            a = _a;
            b = _b;
        }
        else
        {
            a = _b;
            b = _a;
        }
    }

    unsigned int a;
    unsigned int b;
};

struct Neighbors
{
	unsigned int n1;
	unsigned int n2;

    Neighbors()
    {
        n1 = n2 = (unsigned int)-1;
    }

    void AddNeigbor(unsigned int n, const char* meshName)
    {
		if (n1 == (unsigned int)-1) {
			n1 = n;
		}
		else if (n2 == (unsigned int)-1) {
			n2 = n;
		}
	//	else {
	//		SEASSERT(0);
	//		SEDEBUGLOG("Warning, more than one neighbour for face %d of mesh %s",n,meshName);
	//	}
    }

    unsigned int GetOther(unsigned int me) const
    {
        if (n1 == me) {
            return n2;
        }
        else if (n2 == me) {
            return n1;
        }
        else {
        	//SEASSERT(0);
        	return (unsigned int)-1;
        }

        return 0;
    }
};

struct CompareEdges
{
    bool operator()(const Edge& Edge1, const Edge& Edge2)
    {
        if (Edge1.a < Edge2.a) {
            return true;
        }
        else if (Edge1.a == Edge2.a) {
            return (Edge1.b < Edge2.b);
        }
        else {
            return false;
        }
    }
};


struct CompareVectors
{
    bool operator()(const Vector3f& a, const Vector3f& b)
    {
        if (a(0) < b(0)) {
            return true;
        }
        else if (a(0) == b(0)) {
            if (a(1) < b(1)) {
                return true;
            }
            else if (a(1) == b(1)) {
                if (a(2) < b(2)) {
                    return true;
                }
            }
        }

        return false;
    }
};


struct Face
{
    unsigned int indices[3];

    unsigned int GetOppositeIndex(const Edge& e) const
    {
        for (unsigned int i = 0 ; i < 3 ; i++) {
        	unsigned int id = indices[i];

            if (id != e.a && id != e.b) {
                return id;
            }
        }

        SEASSERT(0);

        return 0;
    }
};

GLuint* Model::CreateAdjacencyIndexList(GLuint invalidIndexValue) const {
	SEASSERT(mesh);

    std::map<Edge, Neighbors, CompareEdges> indexMap;
    std::map<Vector3f, unsigned int, CompareVectors> posMap;
    std::vector<Face> uniqueFaces;

    // Step 1 - find the two triangles that share every edge
    for (unsigned int i = 0 ; i < mesh->nbTriangles ; i++) {
        const Vector3i& face = mesh->triangles[i];

        Face unique;

        // If a position vector is duplicated in the VB we fetch the
        // index of the first occurrence.
        for (unsigned int j = 0 ; j < 3 ; j++) {
        	unsigned int id = face[j];
        	Vector3f& v = mesh->vertices[id].pos;

            if (posMap.find(v) == posMap.end()) {
                posMap[v] = id;
            }
            else {
                id = posMap[v];
            }

            unique.indices[j] = id;
        }

        uniqueFaces.push_back(unique);

        Edge e1(unique.indices[0], unique.indices[1]);
        Edge e2(unique.indices[1], unique.indices[2]);
        Edge e3(unique.indices[2], unique.indices[0]);

        indexMap[e1].AddNeigbor(i,mesh->name.c_str());
        indexMap[e2].AddNeigbor(i,mesh->name.c_str());
        indexMap[e3].AddNeigbor(i,mesh->name.c_str());
    }

	// Step 2 - build the index buffer with the adjacency info
	GLuint* indices = new GLuint[mesh->nbTriangles*6];

	for (unsigned int i = 0 ; i < mesh->nbTriangles ; i++) {
		const Face& face = uniqueFaces[i];

		for (unsigned int j = 0 ; j < 3 ; j++) {
			Edge e(face.indices[j], face.indices[(j + 1) % 3]);
			SEASSERT(indexMap.find(e) != indexMap.end());
			Neighbors n = indexMap[e];
			unsigned int otherTri = n.GetOther(i);

			unsigned int oppositeIndex = (unsigned int)-1;
			if (otherTri != (unsigned int)-1) {
				const Face& otherFace = uniqueFaces[otherTri];
				oppositeIndex = otherFace.GetOppositeIndex(e);
			}
		//	else {
		//		SEASSERT(0);
		//		SEDEBUGLOG("No neighbour for face %d at %d in %s",i,j,mesh->name.c_str());
		//	}

			//Indices.push_back(face.indices[j]);
			indices[i*6 + j*2] = face.indices[j];
			//Indices.push_back(OppositeIndex == -1 ? invalidIndexValue : OppositeIndex);
			indices[i*6 + j*2 + 1] = oppositeIndex == (unsigned int)-1 ? invalidIndexValue : oppositeIndex;
		}
	}

	return indices;
}
