#version 400

in vec2 uv;
in vec3 fPos;
in vec3 fNormal;
in vec3 vPos;

uniform samplerCube cubemap;
uniform sampler2D image;
uniform vec4 materialColor; 
uniform mat4 ViewMatrix;
uniform bool skybox;
uniform vec3 eyePos;
uniform bool poolBall;

out vec4 FragColor;

void main()
{
	vec3 I = normalize(fPos - eyePos);
   	vec3 ReflectDir = reflect(I, normalize(fNormal));
	vec4 cubeMapColor = texture(cubemap, ReflectDir);
	// vec4 cubeMapColor = mix(materialColor * texture(image, uv) * 0.9 + cubeMapColor * 0.1);
	float relectionFactor = poolBall? 0.5 : 0.2;
	if (skybox) FragColor = texture(cubemap, fPos);
	else FragColor = mix(materialColor * texture(image, uv), cubeMapColor, relectionFactor);
}

