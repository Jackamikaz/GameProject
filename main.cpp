// Example program:
// Using SDL2 to create an application window

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#include "glheader.h"

#include <stdio.h>
#include <math.h>
#include <GL/glu.h>
#include "syseng.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include "bmfont.h"
#include "ShaderProgram.hpp"
#include "GlBuffer.hpp"
#include "GlUniformHolder.hpp"
#include <stack>
#include "EigenUtils.hpp"
#include "FreeViewCamera.hpp"

#include "Pipeline.hpp"
#include "Scene.hpp"
#include "SceneObject.hpp"
#include "Animator.hpp"

using namespace Eigen;

void DisplayPoseDebug(Model::Pose pose, unsigned int size);

struct ModelInstance {
	ModelInstance()
	:	model(0)
	,	texture(0)
//	,	skinning(0)
//	,	vertices(0)
	,	pose(0)
	,	VBO(0)
	,	IBO(0)
	,	IBO2(0)
	{}
	Model* model;
	Texture* texture;
//	Vector3f* skinning;
//	Model::Mesh::VertexWithBones* vertices;
	Model::Pose pose;

	GlBuffer* VBO;
	GlBuffer* IBO;
	GlBuffer* IBO2;

	bool Load(const char* modelfile, const char* texturefile) {
		model = new Model();
		if (model->Load(modelfile)) {
			texture = new Texture(texturefile);
			pose = new Matrix4f[model->skeleton->nbBones];
		//	skinning = new Vector3f[model->mesh->nbVertices*2];
		//	vertices = model->CreateAndFillVerticesWithBones();

			VBO = new GlBuffer(*model,verticesWithBones);
			IBO = new GlBuffer(*model,indices);
			IBO2 =  new GlBuffer(*model,adjacencyIndices);
			return true;
		}
		delete model;
		model = 0;
		return false;
	}

	void Destroy() {
		delete model;
		delete texture;
		delete [] pose;
	//	delete [] skinning;
	//	delete [] vertices;
		delete VBO;
		delete IBO;
		delete IBO2;
	}
};

//std::stack<Matrix4f> matrixStack;
std::stack<Matrix4f,std::deque<Matrix4f,Eigen::aligned_allocator<Eigen::Matrix4f> > > matrixStack;

void mainProgram(SDL_Window* window) {
	Matrix4f workMat;
	workMat.setIdentity();

    // shader test
	ShaderProgram shader;
	shader.Load("shaders/skinned.vs",0,"shaders/common.fs");
	GlUniformHolder uniforms;
	uniforms.INIT_UNIFORM_HANDLER(shader,DEFAULT_UNIFORMS);

    glClearColor(0,1,1,1);
    glEnable(GL_LIGHT0);

    double angle = 0.0f;
    Uint32 lastTick = 0;

    std::vector<ModelInstance> models;

    ModelInstance instance;
	if (instance.Load("zaza.gmf","zaza1_uv.png")) {models.push_back(instance);}
	if (instance.Load("chevre.gmf","chevreuv.png")) {models.push_back(instance);}
	if (instance.Load("octoroc.gmf","octoroctex.png")) {models.push_back(instance);}

    unsigned int modelInstanceID = 0;
    ModelInstance model = models[modelInstanceID];

    unsigned int animId = 0;
    Model::Animation* anim = &model.model->animations[animId];
    float animT = 0.0f;

    // for terrain
    /*Model terrain;
    terrain.Load("terrain.gmf");
    Texture grass("grass.png");
    GlBuffer terrainVBO(terrain,vertices);
    GlBuffer terrainIBO(terrain,indices);*/
    GlModel mdlTerrain;
    mdlTerrain.Load("terrain.gmf","grass.png");
    ShaderProgram simpleshader("shaders/static.vs",0,"shaders/common.fs");
    GlUniformHolder uniforms2;
    uniforms2.INIT_UNIFORM_HANDLER(simpleshader,DEFAULT_UNIFORMS);

    // outline test
    ShaderProgram outlineShader("shaders/minimalskinned.vs","shaders/shadow_volume.gs","shaders/minimal.fs");
    GlUniformHolder ouniforms;
    ouniforms.INIT_UNIFORM_HANDLER(outlineShader,DEFAULT_UNIFORMS);

    // bmfont test
    BMFont font;
    font.LoadFont("handel.fnt");

    BMFont logFont;
    logFont.LoadFont("lucida_small.fnt");

    // camera
    Vector3f camPos;
    camPos << -5.0f, 0.0f, 1.0f;
    float camHa = 0.0f;
    float camVa = 0.0f;
    bool turning = false;
    bool forward = false;

    // light
    Vector4f dirLight;
    //dirLight << 1.0f, -1.0f, -0.5f, 0.0f;
    dirLight << 0.0f, 0.0f, -1.0f, 0.0f;
    dirLight.normalize();
    float ambientFactor = 0.5f;

    // Wait the quit event
    SDL_Event event;
    bool cont = true;
    while (cont) {
    	Uint32 newTick = SDL_GetTicks();
    	float deltaT = float(newTick - lastTick) / 1000.f;
    	lastTick = newTick;


    	while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				cont = false;
			}

			if (event.type ==SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_SPACE) {
					++animId;
					if (animId >= model.model->animations.size()) {
						animId = 0;
					}

					animT = 0.0f;
					anim = &model.model->animations[animId];

					SysEng::Log(SysEng::log,"Switching to animation %s",anim->name.c_str());
				}
				else if (event.key.keysym.sym == SDLK_m) {
					++modelInstanceID;
					if (modelInstanceID >= models.size()) {
						modelInstanceID = 0;
					}

					model = models[modelInstanceID];

					animId = 0;
					animT = 0.0f;
					anim = &model.model->animations[animId];

					SysEng::Log(SysEng::log,"Switching to model %s",model.model->mesh->name.c_str());
				}
				else if (event.key.keysym.sym == SDLK_e) {
					SEASSERT(false);
				}
				else if (event.key.keysym.sym == SDLK_t) {
					SysEng::Log(SysEng::debug,"animT value = %f", animT);
				}
			}
			else if (event.type == SDL_MOUSEMOTION) {
				if (turning) {
					camHa += event.motion.xrel * 0.003f;
					camVa += event.motion.yrel * 0.003f;
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == 1) {
					forward = true;
				}
				else if (event.button.button == 3) {
					turning = true;
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == 1) {
					forward = false;
				}
				else if (event.button.button == 3) {
					turning = false;
				}
			}
    	}

    	if (forward) {
	    	Vector3f camDir;
	    	float cosVa = cos(camVa);
	    	camDir << cos(camHa)*cosVa, sin(camHa)*cosVa, sin(camVa);

	    	camPos += camDir * deltaT * 5.0f;
		}

    	//Clear color buffer
    	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

    	Matrix4f perspMat = perspective<float>(70.0,640.0/480.0,0.1,50.0);

    	angle += deltaT;

    	workMat.setIdentity();
    	Vector3f camDir,up;
    	float cosVa = cos(camVa);
    	camDir << cos(camHa)*cosVa, sin(camHa)*cosVa, sin(camVa) ;
    	up << 0.0f, 0.0f, 1.0f;
    	workMat *= lookAt<float>(camPos,camPos+camDir,up);


    	glEnable(GL_DEPTH_TEST);
    	glEnable(GL_DEPTH_CLAMP);
    	//glEnable(GL_LIGHTING);

    	//Update anim
    	animT += deltaT*24.0f;
		if (animT > anim->duration) {
			animT = fmod(animT, anim->duration);
		}

		model.model->ComputeAnimPose(*anim,animT+anim->timeBegin,model.pose);

		//Skinning
		//SkinTheMesh(*model.model,model.pose,model.skinning);

    	//Display model
    	//glEnableClientState(GL_VERTEX_ARRAY);
    	//glEnableClientState(GL_NORMAL_ARRAY);
    	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    	//glVertexPointer(3, GL_FLOAT, sizeof(Vector3f)*2, model.skinning);
    	//glNormalPointer(GL_FLOAT, sizeof(Vector3f)*2, model.skinning+1);
    	//glTexCoordPointer(2, GL_FLOAT, sizeof(Model::Mesh::Vertex), &model.model->mesh->vertices->uv);

		matrixStack.push(workMat);
		workMat *= translate(0.0f,0.0f,-1.0f);
		workMat *= scale(3.0f,3.0f,3.0f);

    	glUseProgram(shader);

    	glEnable(GL_TEXTURE_2D);
    	glEnable(GL_CULL_FACE);
    	glBindTexture(GL_TEXTURE_2D, *model.texture);

    	Matrix4f mat = perspMat*workMat;
    	Vector4f dirLightFinal;
    	dirLightFinal = /*workMat **/ dirLight;
    	dirLightFinal.normalize();
    	uniforms.Set(uSampler0,(unsigned int)0);
    	uniforms.Set(uViewMatrix,workMat);
    	uniforms.Set(uFinalMatrix,mat);
    	uniforms.Set(uDirectionalLight,(Vector3f)dirLightFinal.head<3>());
    	uniforms.Set(uAmbientFactor,ambientFactor);
    	for(unsigned int i=0; i<model.model->skeleton->nbBones; ++i) {
    		uniforms.Set(uBones,i,model.pose[i]);
    	}

       	typedef Model::Mesh::VertexWithBones VerBo;

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		glBindBuffer(GL_ARRAY_BUFFER, *model.VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VerBo), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VerBo), (GLvoid *)(&((VerBo*)(0))->nor));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VerBo), (GLvoid *)(&((VerBo*)(0))->uv));
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(VerBo), (GLvoid *)(&((VerBo*)(0))->bids));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VerBo), (GLvoid *)(&((VerBo*)(0))->bws));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *model.IBO);
		glDrawElements(GL_TRIANGLES, model.model->mesh->nbTriangles*3, GL_UNSIGNED_INT, 0);

		// outline
		//glDisable(GL_DEPTH_TEST);
		//glDisable(GL_CULL_FACE);
		glUseProgram(outlineShader);
    	ouniforms.Set(uViewMatrix,workMat);
    	//ouniforms.Set(uFinalMatrix,mat);
    	ouniforms.Set(uProjMatrix,perspMat);
    	ouniforms.Set(uDirectionalLight,(Vector3f)dirLightFinal.head<3>());
    	for(unsigned int i=0; i<model.model->skeleton->nbBones; ++i) {
    		ouniforms.Set(uBones,i,model.pose[i]);
    	}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *model.IBO2);
	//	glEnable(GL_PRIMITIVE_RESTART);
	//	glPrimitiveRestartIndex((unsigned int)-1);
		//glLineWidth(10.0f);
		glDrawElements(GL_TRIANGLES_ADJACENCY, model.model->mesh->nbTriangles*6, GL_UNSIGNED_INT, 0);
		//glEnable(GL_DEPTH_TEST);
	//	glDisable(GL_PRIMITIVE_RESTART);
	//	glEnable(GL_CULL_FACE);

		// terrain
		typedef Model::Mesh::Vertex Vertex;
    	workMat = matrixStack.top();
    	matrixStack.pop();
    	matrixStack.push(workMat);
    	workMat *= translate<float>(0.0f,0.0f,-2.0f);
		glUseProgram(simpleshader);
		glBindTexture(GL_TEXTURE_2D, *mdlTerrain.GetTexture());//grass);
		mat = perspMat*workMat;
		dirLightFinal = /*workMat **/ dirLight;
    	dirLightFinal.normalize();
    	uniforms2.Set(uSampler0,(unsigned int)0);
    	uniforms2.Set(uViewMatrix,workMat);
		uniforms2.Set(uFinalMatrix,mat);
		uniforms2.Set(uDirectionalLight,(Vector3f)dirLightFinal.head<3>());
		uniforms2.Set(uAmbientFactor,ambientFactor);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, *mdlTerrain.GetVBO());//terrainVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(&((Vertex*)(0))->nor));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(&((Vertex*)(0))->uv));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *mdlTerrain.GetIBO());//terrainIBO);
		//glDrawElements(GL_TRIANGLES, terrain.mesh->nbTriangles*3, GL_UNSIGNED_INT, 0);
		glDrawElements(GL_TRIANGLES, mdlTerrain.GetModel()->mesh->nbTriangles*3, GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		glUseProgram(0);

		workMat = matrixStack.top();
		matrixStack.pop();

		// debug draws
    	glDisable(GL_TEXTURE_2D);
    	glDisable(GL_DEPTH_TEST);
    	glDisable(GL_LIGHTING);

    	glMatrixMode(GL_PROJECTION);
    	glLoadMatrixf(perspMat.data());
    	glMatrixMode(GL_MODELVIEW);
    	matrixStack.push(workMat);
		workMat *= translate(0.0f,0.0f,-1.0f);
		workMat *= scale(3.0f,3.0f,3.0f);
    	glLoadMatrixf(workMat.data());
    	workMat = matrixStack.top();
    	matrixStack.pop();

    	//skeleton debug
    	/*

    	glPointSize(5.0f);

    	model.model->ComputeAnimPoseDebug(*anim,animT+anim->timeBegin,model.pose);
    	DisplayPoseDebug(model.pose, model.model->skeleton->nbBones);*/

    	//axes
		glBegin(GL_LINES);
		glColor3ub(255,0,0);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		glColor3ub(0,255,0);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		glColor3ub(0,0,255);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,1.0f);
		glEnd();
		glColor3ub(255,255,255);

		//terrain normals
		/*glLoadMatrixf(workMat.data());
		glBegin(GL_LINES);
		for(unsigned int i=0; i<terrain.mesh->nbVertices; ++i) {
			glVertex3fv(terrain.mesh->vertices[i].pos.data());
			Vector3f tonor;
			tonor = terrain.mesh->vertices[i].pos + terrain.mesh->vertices[i].nor;
			glVertex3fv(tonor.data());
		}
		glEnd();*/

    	// font display test
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0,640,480,0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		font.Print(10.0f,470.0f - font.GetHeight(), "%s : %s",model.model->mesh->name.c_str(), anim->name.c_str());

		SysEng::DisplayLogs(logFont);

    	//Update screen
    	SDL_GL_SwapWindow(window);

    	SDL_Delay(30);
    }

    for(std::vector<ModelInstance>::iterator it = models.begin(); it != models.end(); ++it) {
    	(*it).Destroy();
    }

    models.clear();
}

void mainProgBis(SDL_Window* window) {

	//Païepeulaïne
	Scene scene;
	scene.perspective = perspective<float>(70.0,640.0/480.0,0.1,50.0);

	GlModel mdlTerrain("terrain.gmf","grass.png",false);
	SceneObject terrain(mdlTerrain);
	terrain.localMat = translate(0.0f,0.0f,-2.0f);

	GlModel mdlAnim("octoroc.gmf","octoroctex.png",true);
	SceneObject animObj(mdlAnim);
	animObj.localMat = translate(0.0f,0.0f,-1.0f) * scale(3.0f,3.0f,3.0f);

	GlModel mdlCrate("crate.gmf","crate.png",true);
	SceneObject crate(mdlCrate);
	crate.localMat = translate(4.0f,4.0f,-1.0f);
	SceneObject otherCrate(mdlCrate);
	otherCrate.localMat = translate(-4.0f,4.0f,-1.0f);
	otherCrate.texOverride = mdlTerrain.GetTexture();

	scene.Insert(terrain);
	scene.Insert(animObj);
	scene.Insert(crate);
	scene.Insert(otherCrate);

	Animator animator(animObj);
	unsigned int zazaAnimID = 0;
	animator.SetAnimation(animObj.model.GetModel()->animations[zazaAnimID]);

	Pipeline pipeline;
	glClearColor(0,1,1,1);

    // camera
    /*Vector3f camPos;
    camPos << -5.0f, 0.0f, 1.0f;
    float camHa = 0.0f;
    float camVa = 0.0f;*/
    bool turning = false;
    bool forward = false;
    Vector3f up;
    up << 0.0f, 0.0f, 1.0f;
	FreeViewCamera camera1;
	FreeViewCamera camera2;
	camera2.position << 3.0f, 3.0f, 0.0f;
	camera2.LookAt(camera1.position,up);
	FreeViewCamera* curcam = &camera2;

	std::map<unsigned int,bool> keyPressed;

    BMFont logFont;
    logFont.LoadFont("lucida_small.fnt");

    Uint32 lastTick = SDL_GetTicks();
    SDL_Event event;
    bool cont = true;
    while (cont) {
    	Uint32 newTick = SDL_GetTicks();
    	float deltaT = float(newTick - lastTick) / 1000.f;
    	lastTick = newTick;

    	while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				cont = false;
			}
			if (event.type ==SDL_KEYDOWN) {

				std::map<unsigned int,bool>::iterator it = keyPressed.find(event.key.keysym.sym);
				if (it != keyPressed.end()) {
					it->second = true;
				}

				if (event.key.keysym.sym == SDLK_SPACE) {
					++zazaAnimID;
					if (zazaAnimID >= animObj.model.GetModel()->animations.size()) {
						zazaAnimID = 0;
					}

					const Model::Animation& newAnim = animObj.model.GetModel()->animations[zazaAnimID];
					animator.SetAnimation(newAnim);

					SysEng::Log(SysEng::log,"Switching to animation %s",newAnim.name.c_str());
				}
				else if (event.key.keysym.sym == SDLK_c) {
					curcam = curcam == &camera1 ? &camera2 : &camera1;
				}
			}
			else if (event.type == SDL_KEYUP) {
				std::map<unsigned int,bool>::iterator it = keyPressed.find(event.key.keysym.sym);
				if (it != keyPressed.end()) {
					it->second = false;
				}
			}
			else if (event.type == SDL_MOUSEMOTION) {
				if (turning) {
					//camHa += event.motion.xrel * 0.003f;
					//camVa += event.motion.yrel * 0.003f;
					curcam->TurnRestr(event.motion.xrel * 0.003f, up);
					curcam->TurnV(event.motion.yrel * 0.003f);
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == 1) {
					forward = true;
				}
				else if (event.button.button == 3) {
					turning = true;
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == 1) {
					forward = false;
				}
				else if (event.button.button == 3) {
					turning = false;
				}
			}
    	}

    	if (forward) {
	    //	Vector3f camDir;
	    //	float cosVa = cos(camVa);
	    //	camDir << cos(camHa)*cosVa, sin(camHa)*cosVa, sin(camVa);

	    //	camPos += camDir * deltaT * 5.0f;
    		curcam->Walk(deltaT * 5.0f);
		}

    	if (keyPressed[SDLK_UP]) {
    		curcam->Hover(deltaT * 5.0f);
    	}
    	if (keyPressed[SDLK_DOWN]) {
    		curcam->Hover(-deltaT * 5.0f);
    	}
    	if (keyPressed[SDLK_LEFT]) {
    		curcam->SideStep(-deltaT * 5.0f);
    	}
    	if (keyPressed[SDLK_RIGHT]) {
    		curcam->SideStep(deltaT * 5.0f);
    	}
    	if (keyPressed[SDLK_KP_1]) {
    		curcam->TurnH(deltaT);
    	}
    	if (keyPressed[SDLK_KP_2]) {
    		curcam->TurnH(-deltaT);
		}
    	if (keyPressed[SDLK_KP_4]) {
    		curcam->TurnV(deltaT);
		}
		if (keyPressed[SDLK_KP_5]) {
			curcam->TurnV(-deltaT);
		}
		if (keyPressed[SDLK_KP_7]) {
			curcam->TurnSelf(deltaT);
		}
		if (keyPressed[SDLK_KP_8]) {
			curcam->TurnSelf(-deltaT);
		}

    	// camera
    //	Vector3f camDir;
    //	float cosVa = cos(camVa);
    //	camDir << cos(camHa)*cosVa, sin(camHa)*cosVa, sin(camVa) ;
    //	scene.view = lookAt<float>(camPos,camPos+camDir,up);
    	scene.view = curcam->ToMatrix();
    //	scene.lightDirection << camDir, 0.0f;
    	scene.lightDirection << curcam->GetForward(), 0.0f;

    	// anims
    	animator.Update(deltaT);

    	// main draw
    	pipeline.Draw(scene);

		// font display
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0,640,480,0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);

		SysEng::DisplayLogs(logFont);

		//Update screen
		SDL_GL_SwapWindow(window);

		SDL_Delay(30);
	}
}

int main(int argc, char* argv[]) {
	SDL_Window *window;
	SDL_GLContext context;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window = SDL_CreateWindow(
    		"yo les chocos",
    		SDL_WINDOWPOS_UNDEFINED,
    		SDL_WINDOWPOS_UNDEFINED,
    		640, 480,
    		SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    int stbits;
    glGetIntegerv(GL_STENCIL_BITS, &stbits);
    SEDEBUGLOG("There is %i stencil bits according to OpenGL.",stbits);
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE,&stbits);
    SEDEBUGLOG("There is %i stencil bits according to SDL.",stbits);

    // Check that the window was successfully made
    if (window == NULL) {
        // In the event that the window could not be made...
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 2;
	}

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    SysEng::GetSingleton().mainWindowHandle = wmInfo.info.win.window;

    //mainProgram(window);
    mainProgBis(window);

    // Close and destroy the window
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    SysEng::DestroySingleton();

    return 0;
}

void DisplayPoseDebug(Model::Pose pose, unsigned int size) {
	for(unsigned int i=0; i<size; ++i) {
		glPushMatrix();

		glMultMatrixf(pose[i].data());

		glBegin(GL_POINTS);
		glVertex3f(0.0f,0.0f,0.0f);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.1f,0.0f);
		glEnd();

		glPopMatrix();
	}
}
