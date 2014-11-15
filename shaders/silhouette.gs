#version 410

layout (triangles_adjacency) in;
layout (line_strip, max_vertices = 6) out;

layout (location = 0) in vec3 iPosition[];

void EmitLine(int StartIndex, int EndIndex)
{
    gl_Position = gl_in[StartIndex].gl_Position;
    EmitVertex();

    gl_Position = gl_in[EndIndex].gl_Position;
    EmitVertex();

    EndPrimitive();
}

uniform vec3 uDirectionalLight;

void main()
{
    vec3 e1 = iPosition[2] - iPosition[0];
    vec3 e2 = iPosition[4] - iPosition[0];
    vec3 e3 = iPosition[1] - iPosition[0];
    vec3 e4 = iPosition[3] - iPosition[2];
    vec3 e5 = iPosition[4] - iPosition[2];
    vec3 e6 = iPosition[5] - iPosition[0];

    vec3 Normal = cross(e1,e2);

    if (dot(Normal, uDirectionalLight) > 0.00001) {

        Normal = cross(e3,e1);

        if (dot(Normal, uDirectionalLight) <= 0) {
            EmitLine(0, 2);
        }

        Normal = cross(e4,e5);

        if (dot(Normal, uDirectionalLight) <=0) {
            EmitLine(2, 4);
        }

        Normal = cross(e2,e6);

        if (dot(Normal, uDirectionalLight) <= 0) {
            EmitLine(4, 0);
        }
    }
}