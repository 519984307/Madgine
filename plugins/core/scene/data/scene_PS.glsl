#version 420 core

layout (std140, binding = 0) uniform PerApplication
{
	mat4 p;	
};

layout (std140, binding = 1) uniform PerFrame
{
	mat4 v;

	vec3 lightColor;
	vec3 lightDir;
};

layout (std140, binding = 2) uniform PerObject
{
	mat4 m;
	mat4 anti_m;

	bool hasLight;
	bool hasTexture;
	bool hasDistanceField;
	bool hasSkeleton;
};

layout(binding = 0) uniform sampler2D tex;


in vec4 color;
in vec4 worldPos;
in vec3 normal;
in vec2 uv;



float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{

	vec4 colorAcc = color;

	if (hasTexture){
		if (hasDistanceField){
			vec2 msdfUnit = 4.0/vec2(512.0, 512.0);
			vec4 sampled = texture2D(tex, uv);
			float sigDist = median(sampled.r, sampled.g, sampled.b) - 0.5;
			//sigDist *= dot(msdfUnit, vec2(0.5));
			sigDist *= 4.0;
			float opacity = clamp(sigDist + 0.5, 0.0, 1.0);
			colorAcc = mix(vec4(0), colorAcc, opacity);
		}
		else
		{
			colorAcc = colorAcc * texture2D(tex, uv);
		}
	}

	if (hasLight){
		float ambientStrength = 0.4;
		vec3 ambient = ambientStrength * lightColor;

		float diffuseStrength = 0.7;
		vec3 norm = normalize(normal);
		float diff = max(dot(norm, -lightDir), 0.0);
		vec3 diffuse = diffuseStrength * diff * lightColor;

		colorAcc = vec4(ambient + diffuse,1.0) * colorAcc;
	}

    gl_FragColor = colorAcc;
}