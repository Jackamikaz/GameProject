#include "Pipeline.hpp"

Pipeline::Pipeline() {

	GLuint fCommon = CompileShader("shaders/common.fs", GL_FRAGMENT_SHADER);
	GLuint fMinimal = CompileShader("shaders/minimal.fs", GL_FRAGMENT_SHADER);
	GLuint vMinimal = CompileShader("shaders/minimal.vs", GL_VERTEX_SHADER);
	GLuint vMinimalskinned = CompileShader("shaders/minimalskinned.vs", GL_VERTEX_SHADER);
	//GLuint gShadow = CompileShader("shaders/shadow_volume.gs", GL_GEOMETRY_SHADER);
	GLuint gShadow = CompileShader("shaders/shadow_gpugems.gs", GL_GEOMETRY_SHADER);
	GLuint vSkinned = CompileShader("shaders/skinned.vs", GL_VERTEX_SHADER);
	GLuint vStatic = CompileShader("shaders/static.vs", GL_VERTEX_SHADER);

	techniques[depth].type[anim].shader.Load(vMinimalskinned,0,fMinimal);
	techniques[depth].type[stat].shader.Load(vMinimal,0,fMinimal);

	techniques[shadow].type[anim].shader.Load(vMinimalskinned,gShadow,fMinimal);
	techniques[shadow].type[stat].shader.Load(vMinimal,gShadow,fMinimal);

	techniques[color].type[anim].shader.Load(vSkinned,0,fCommon);
	techniques[color].type[stat].shader.Load(vStatic,0,fCommon);

	for(unsigned int i=0; i<nbPasses; ++i) {
		for(unsigned int j=0; j<nbTechTypes; ++j) {
			techniques[i].type[j].uniforms.INIT_UNIFORM_HANDLER(techniques[i].type[j].shader,DEFAULT_UNIFORMS);
		}
	}


	fbTex.FromFrameBuffer(frameBuffer,256,256,fbColor);
}

Pipeline::~Pipeline() {
	for(unsigned int i=0; i<nbPasses; ++i) {
		for(unsigned int j=0; j<nbTechTypes; ++j) {
			techniques[i].type[j].shader.Destroy();
		}
	}
}

void ApplyUniforms(const GlUniformHolder& uniforms, const SceneObject& obj, const Scene& scene);
void ConfigVertexAttribStat();
void ConfigVertexAttribAnim();
void EnableVertexAttribArrayTo(const unsigned int nb);
void DisableVertexAttribArrayTo(const unsigned int nb);
void NextShadowCaster(Scene::ObjectList::const_iterator& it, const Scene::ObjectList::const_iterator& end);

void Pipeline::Draw(const Scene& scene) {

	Eigen::Matrix4f finalMat;
	finalMat = scene.perspective * scene.view;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
/*	glEnable(GL_DEPTH_CLAMP);

	// Depth pass
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glDepthMask(GL_TRUE);
	DoPass(scene,depth,true);
	DoPass(scene,depth,false);

	// Shadow pass
	glEnable(GL_STENCIL_TEST);

	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

//	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
//	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
    glStencilMask(0xffffffff);
    glClearStencil(0);
    glClear( GL_STENCIL_BUFFER_BIT );

	DoPass(scene,shadow,true);
	DoPass(scene,shadow,false);
	glEnable(GL_CULL_FACE);

	// Color pass
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 0x0, 0xFF);

//	glStencilFunc( GL_EQUAL, 1, 0xffffffff );
//	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	glStencilMask(0);*/


	DoPass(scene,color,true);
	DoPass(scene,color,false);


/*	glDisable(GL_STENCIL_TEST);*/
}

void Pipeline::DoPass(const Scene& scene, unsigned int pass, bool animated) const {
	const ShaderProgram& shader = techniques[pass].type[animated ? anim : stat].shader;
	const GlUniformHolder& uniforms = techniques[pass].type[animated ? anim : stat].uniforms;

	void (*configVertex)(void) = animated ? ConfigVertexAttribAnim : ConfigVertexAttribStat;

	glUseProgram(shader);

	EnableVertexAttribArrayTo(animated ? 5 : 3);

	const Scene::ObjectList& objs = scene.GetObjectsList(animated);

	Scene::ObjectList::const_iterator it = objs.begin();
	const Scene::ObjectList::const_iterator end = objs.end();
	if (pass == shadow) {
		NextShadowCaster(it,end);
	}

	while(it != end) {
		const SceneObject& obj = *(*it);

		glBindTexture(GL_TEXTURE_2D, obj.GetTexture());

		ApplyUniforms(uniforms,obj,scene);

		glBindBuffer(GL_ARRAY_BUFFER, *obj.model.GetVBO());
		configVertex();

		if (pass == shadow) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *obj.model.GetIBOadj());
			glDrawElements(GL_TRIANGLES_ADJACENCY, obj.model.GetModel()->mesh->nbTriangles*6, GL_UNSIGNED_INT, 0);
			NextShadowCaster(++it,end);
		}
		else {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *obj.model.GetIBO());
			glDrawElements(GL_TRIANGLES, obj.model.GetModel()->mesh->nbTriangles*3, GL_UNSIGNED_INT, 0);
			++it;
		}
	}

	glUseProgram(0);
	DisableVertexAttribArrayTo(5);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ApplyUniforms(const GlUniformHolder& uniforms, const SceneObject& obj, const Scene& scene) {
	Eigen::Matrix4f objMat = scene.view * obj.localMat;
	Eigen::Matrix4f finalMat = scene.perspective * objMat;

	if (uniforms.UniformExists(uSampler0)) {
		uniforms.Set(uSampler0,(unsigned int)0);
	}

	if (uniforms.UniformExists(uViewMatrix)) {
		uniforms.Set(uViewMatrix,objMat);
	}

	if (uniforms.UniformExists(uProjMatrix)) {
	uniforms.Set(uProjMatrix,scene.perspective);
	}

	if (uniforms.UniformExists(uFinalMatrix)) {
		uniforms.Set(uFinalMatrix,finalMat);
	}

	if (uniforms.UniformExists(uDirectionalLight)) {
		Eigen::Vector4f dirLightFinal;
		dirLightFinal = objMat * scene.lightDirection;
		dirLightFinal.normalize();
		uniforms.Set(uDirectionalLight,(Eigen::Vector3f)dirLightFinal.head<3>());
	}

	if (uniforms.UniformExists(uAmbientFactor)) {
		uniforms.Set(uAmbientFactor,scene.ambientFactor);
	}

	if (uniforms.UniformExists(uBones)) {
		for(unsigned int i=0; i<obj.model.GetModel()->skeleton->nbBones; ++i) {
			uniforms.Set(uBones,i,obj.pose[i]);
		}
	}
}

void ConfigVertexAttribStat() {
	typedef Model::Mesh::Vertex Ver;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Ver), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Ver), (GLvoid *)(&((Ver*)(0))->nor));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Ver), (GLvoid *)(&((Ver*)(0))->uv));
}

void ConfigVertexAttribAnim() {
	typedef Model::Mesh::VertexWithBones VerBo;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VerBo), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VerBo), (GLvoid *)(&((VerBo*)(0))->nor));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VerBo), (GLvoid *)(&((VerBo*)(0))->uv));
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(VerBo), (GLvoid *)(&((VerBo*)(0))->bids));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VerBo), (GLvoid *)(&((VerBo*)(0))->bws));
}

void EnableVertexAttribArrayTo(const unsigned int nb) {
	for(unsigned int i=0; i<nb; ++i) {
		glEnableVertexAttribArray(i);
	}
}

void DisableVertexAttribArrayTo(const unsigned int nb) {
	for(unsigned int i=0; i<nb; ++i) {
		glDisableVertexAttribArray(i);
	}
}

void NextShadowCaster(Scene::ObjectList::const_iterator& it, const Scene::ObjectList::const_iterator& end) {
	while(it != end && !(*it)->model.CastsShadow()) {
		++it;
	}
}
