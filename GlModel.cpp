#include "GlModel.hpp"

GlModel::GlModel()
:	model(0)
,	texture(0)
,	VBO(0)
,	IBO(0)
,	IBOadj(0)
{
}

GlModel::GlModel(const char* modelfile, const char* texturefile, bool castsShadow)
:	model(0)
,	texture(0)
,	VBO(0)
,	IBO(0)
,	IBOadj(0)
{
	Load(modelfile,texturefile,castsShadow);
}

GlModel::~GlModel() {
	if (model) {delete model;}
	if (texture) {delete texture;}
	if (VBO) {delete VBO;}
	if (IBO) {delete IBO;}
	if (IBOadj) {delete IBOadj;}
}

bool GlModel::Load(const char* modelfile, const char* texturefile, bool castsShadow) {
	model = new Model();
	if (model->Load(modelfile)) {
		texture = new Texture(texturefile);

		VBO = new GlBuffer(*model, model->skeleton ? verticesWithBones : vertices);
		IBO = new GlBuffer(*model,indices);
		if (castsShadow) {
			IBOadj =  new GlBuffer(*model,adjacencyIndices);
		}
		return true;
	}
	delete model;
	model = 0;
	return false;
}

