#version 430 core

layout (std140, binding = 0) uniform PerApplication
{
	mat4 p;		
};

layout (std140, binding = 1) uniform PerFrame
{
	mat4 v;
};

layout (std140, binding = 2) uniform PerObject
{
	mat4 m;
	
	bool hasTexture;
	bool hasDistanceField;
};


layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aPos2;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in vec2 aUV;

out vec4 color;
out vec4 worldPos;
out vec3 normal;
out vec2 uv;


void main()
{
	worldPos = m * vec4(aPos, 1.0);
	
    gl_Position = p * (v * worldPos + vec4(aPos2, 0.0, 0.0));	
    color = aColor;
	normal = aNormal;
	uv = vec2(aUV.x, 1.0 - aUV.y);	
}