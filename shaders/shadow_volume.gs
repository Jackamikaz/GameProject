#version 330

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 18) out;

layout (location = 0) in vec3 iPosition[];

uniform vec3 uDirectionalLight;
uniform mat4 uProjMatrix;

float EPSILON = 0.01;

void EmitQuad(int StartIndex, vec3 StartVertex, int EndIndex, vec3 EndVertex)
{
    vec3 l = uDirectionalLight * EPSILON;
    gl_Position = uProjMatrix * vec4((StartVertex + l), 1.0);
    EmitVertex();
    
    gl_Position = uProjMatrix * vec4(uDirectionalLight, 0.0);
    EmitVertex();

    l = uDirectionalLight * EPSILON;
    gl_Position = uProjMatrix * vec4((EndVertex + l), 1.0);
    EmitVertex();
    
    gl_Position = uProjMatrix * vec4(uDirectionalLight, 0.0);
    EmitVertex();

    EndPrimitive();            
}


void main()
{
    vec3 e1 = iPosition[2] - iPosition[0];
    vec3 e2 = iPosition[4] - iPosition[0];
    vec3 e3 = iPosition[1] - iPosition[0];
    vec3 e4 = iPosition[3] - iPosition[2];
    vec3 e5 = iPosition[4] - iPosition[2];
    vec3 e6 = iPosition[5] - iPosition[0];

    vec3 Normal = cross(e1,e2);

    if (dot(Normal, uDirectionalLight) > 0.000001) {

        Normal = cross(e3,e1);

        if (dot(Normal, uDirectionalLight) <= 0) {
            vec3 StartVertex = iPosition[0];
            vec3 EndVertex = iPosition[2];
            EmitQuad(0, StartVertex, 2, EndVertex);
        }

        Normal = cross(e4,e5);

        if (dot(Normal, uDirectionalLight) <= 0) {
            vec3 StartVertex = iPosition[2];
            vec3 EndVertex = iPosition[4];
            EmitQuad(2, StartVertex, 4, EndVertex);
        }

        Normal = cross(e2,e6);

        if (dot(Normal, uDirectionalLight) <= 0) {
            vec3 StartVertex = iPosition[4];
            vec3 EndVertex = iPosition[0];
            EmitQuad(4, StartVertex, 0, EndVertex);
        }

        gl_Position = uProjMatrix * vec4((iPosition[0] + uDirectionalLight), 1.0);
        EmitVertex();

        gl_Position = uProjMatrix * vec4((iPosition[2] + uDirectionalLight), 1.0);
        EmitVertex();

        gl_Position = uProjMatrix * vec4((iPosition[4] + uDirectionalLight), 1.0);
        EmitVertex();
        EndPrimitive();
    }
}