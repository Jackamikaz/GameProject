#include "GlBuffer.hpp"
#include "Model.hpp"
#include "syseng.hpp"

GlBuffer::GlBuffer()
:	bufferID(0)
{}

GlBuffer::GlBuffer(const Model& model, ModelBufferType mbt)
:	bufferID(0)
{
	Load(model, mbt);
}

GlBuffer::~GlBuffer()
{
	if (bufferID != 0)
	{
		Destroy();
	}
}

void GlBuffer::Destroy()
{
	SEASSERT(bufferID =! 0);

	glDeleteBuffers(1,&bufferID);

	bufferID = 0;
}

bool GlBuffer::Load(const Model& model, ModelBufferType mbt) {

	SEASSERT(model.mesh && model.mesh->vertices);

	glGenBuffers(1,&bufferID);


	switch(mbt) {
	case vertices:
	{
		glBindBuffer(GL_ARRAY_BUFFER,bufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Model::Mesh::Vertex)*model.mesh->nbVertices,model.mesh->vertices,GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}
		break;
	case verticesWithBones:
	{
		Model::Mesh::VertexWithBones* verts = model.CreateAndFillVerticesWithBones();
		glBindBuffer(GL_ARRAY_BUFFER,bufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Model::Mesh::VertexWithBones)*model.mesh->nbVertices,verts,GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		delete [] verts;
	}
		break;
	case indices:
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*model.mesh->nbTriangles*3,model.mesh->triangles,GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	}
		break;
	case adjacencyIndices:
	{
		GLuint* ids = model.CreateAdjacencyIndexList();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*model.mesh->nbTriangles*6,ids,GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
		delete [] ids;
	}
		break;
	}

	return false;
}
