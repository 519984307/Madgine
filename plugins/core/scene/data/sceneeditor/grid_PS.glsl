#version 420 core

in vec4 worldPos;

vec3 _smoothstep(float edge0, float edge1, vec3 x) {
	vec3 t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

void main()
{
	vec4 col = vec4(0.0,0.0,0.0,0.0);

	vec3 coord = worldPos.xyz / 1.0; //square size in world space
	coord /= worldPos.w;
	
	vec3 frac = fract(coord); //fractional parts of squares
	//interpolation, grad is smoothness of line gradients
	float grad = 0.01;
	vec3 mult = _smoothstep(0.0, grad, frac) - _smoothstep(1.0-grad, 1.0, frac);
	//col = mix(vec4(1.0,1.0,1.0,1.0), vec4(0.0,0.0,0.0,0.0), mult.x * mult.y * mult.z);
	col = vec4(0.7, 0.7, 0.7, clamp(1.0 - mult.x * mult.z, 0.0, 1.0));

    gl_FragColor = col;
}